#include "vkpch.h"
#include "VulkanCommandBuffer.h"

#include "VoltVulkan/Common/VulkanCommon.h"
#include "VoltVulkan/Common/VulkanHelpers.h"

#include "VoltVulkan/Graphics/VulkanPhysicalGraphicsDevice.h"
#include "VoltVulkan/Graphics/VulkanSwapchain.h"

#include "VoltVulkan/Pipelines/VulkanRenderPipeline.h"
#include "VoltVulkan/Pipelines/VulkanComputePipeline.h"

#include "VoltVulkan/Descriptors/VulkanDescriptorTable.h"

#include "VoltVulkan/Images/VulkanImage2D.h"
#include "VoltVulkan/Buffers/VulkanStorageBuffer.h"

#include <VoltRHI/Graphics/GraphicsContext.h>
#include <VoltRHI/Graphics/GraphicsDevice.h>
#include <VoltRHI/Graphics/DeviceQueue.h>

#include <VoltRHI/Memory/Allocation.h>

#include <VoltRHI/Buffers/IndexBuffer.h>
#include <VoltRHI/Buffers/VertexBuffer.h>

#include <VoltRHI/Images/ImageView.h>

#include <VoltRHI/Shader/Shader.h>

#include <vulkan/vulkan.h>
#include <VoltRHI/Core/Profiling.h>

namespace Volt::RHI
{
	namespace Utility
	{
		std::pair<VkPipelineStageFlags2, VkAccessFlags2> GetSrcInfoFromResourceState(const ResourceState resourceState)
		{
			switch (resourceState)
			{
				case ResourceState::RenderTarget:
				{
					return { VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT, VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT };
					break;
				}

				case ResourceState::Present:
				{
					return { VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT, 0 };
					break;
				}

				case ResourceState::PixelShaderRead:
				{
					return { VK_PIPELINE_STAGE_2_FRAGMENT_SHADER_BIT, VK_ACCESS_2_SHADER_READ_BIT };
					break;
				}

				case ResourceState::NonPixelShaderRead:
				{
					return { VK_PIPELINE_STAGE_2_VERTEX_SHADER_BIT, VK_ACCESS_2_SHADER_READ_BIT };
					break;
				}

				case ResourceState::TransferSrc:
				{
					return { VK_PIPELINE_STAGE_2_TRANSFER_BIT, VK_ACCESS_2_TRANSFER_READ_BIT };
					break;
				}

				case ResourceState::TransferDst:
				{
					return { VK_PIPELINE_STAGE_2_TRANSFER_BIT, VK_ACCESS_2_TRANSFER_WRITE_BIT };
					break;
				}

				case ResourceState::Undefined:
				{
					return { VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT, 0 };
					break;
				}

				case ResourceState::IndirectArgument:
				{
					return { VK_PIPELINE_STAGE_2_DRAW_INDIRECT_BIT, VK_ACCESS_2_INDIRECT_COMMAND_READ_BIT };
					break;
				}

				case ResourceState::UnorderedAccess:
				{
					return { VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT, VK_ACCESS_2_SHADER_WRITE_BIT };
					break;
				}
			}

			return { VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT, 0 };
		}

		std::pair<VkPipelineStageFlags2, VkAccessFlags2> GetDstInfoFromResourceState(const ResourceState resourceState)
		{
			switch (resourceState)
			{
				case ResourceState::RenderTarget:
				{
					return { VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT, VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT };
					break;
				}

				case ResourceState::Present:
				{
					return { VK_PIPELINE_STAGE_2_NONE , 0 };
					break;
				}

				case ResourceState::PixelShaderRead:
				{
					return { VK_PIPELINE_STAGE_2_FRAGMENT_SHADER_BIT, VK_ACCESS_2_SHADER_READ_BIT };
					break;
				}

				case ResourceState::NonPixelShaderRead:
				{
					return { VK_PIPELINE_STAGE_2_VERTEX_SHADER_BIT, VK_ACCESS_2_SHADER_READ_BIT };
					break;
				}

				case ResourceState::TransferSrc:
				{
					return { VK_PIPELINE_STAGE_2_TRANSFER_BIT, VK_ACCESS_2_TRANSFER_READ_BIT };
					break;
				}

				case ResourceState::TransferDst:
				{
					return { VK_PIPELINE_STAGE_2_TRANSFER_BIT, VK_ACCESS_2_TRANSFER_WRITE_BIT };
					break;
				}

				case ResourceState::IndirectArgument:
				{
					return { VK_PIPELINE_STAGE_2_DRAW_INDIRECT_BIT, VK_ACCESS_2_INDIRECT_COMMAND_READ_BIT };
					break;
				}

				case ResourceState::UnorderedAccess:
				{
					return { VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT, VK_ACCESS_2_SHADER_WRITE_BIT };
					break;
				}

				case ResourceState::Undefined:
				{
					return { VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT, 0 };
					break;
				}
			}

			return { VK_PIPELINE_STAGE_2_NONE, 0 };
		}

		const VkImageLayout GetDstLayoutFromResourceState(const ResourceState resourceState)
		{
			switch (resourceState)
			{
				case ResourceState::RenderTarget: return VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
				case ResourceState::Present: return VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
				case ResourceState::PixelShaderRead: return VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
				case ResourceState::NonPixelShaderRead: return VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
				case ResourceState::TransferSrc: return VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
				case ResourceState::TransferDst: return VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
			}

			return VK_IMAGE_LAYOUT_UNDEFINED;
		}
	}

	VulkanCommandBuffer::VulkanCommandBuffer(const uint32_t count, QueueType queueType, bool swapchainTarget)
		: CommandBuffer(queueType), m_commandBufferCount(count), m_isSwapchainTarget(swapchainTarget)
	{
		m_currentCommandBufferIndex = m_commandBufferCount - 1; // This makes sure that we start at command buffer index 0 for clarity

		Invalidate();
	}

	VulkanCommandBuffer::~VulkanCommandBuffer()
	{
		Release();
	}

	void VulkanCommandBuffer::Begin()
	{
		VT_PROFILE_FUNCTION();

		const uint32_t lastIndex = m_currentCommandBufferIndex;
		m_lastCommandBufferIndex = lastIndex;
		m_currentCommandBufferIndex = (m_currentCommandBufferIndex + 1) % m_commandBufferCount;

		auto device = GraphicsContext::GetDevice();
		const uint32_t index = GetCurrentCommandBufferIndex();
		const auto& currentCommandBuffer = m_commandBuffers.at(index);

		if (!m_isSwapchainTarget)
		{
			VT_VK_CHECK(vkWaitForFences(device->GetHandle<VkDevice>(), 1, &currentCommandBuffer.fence, VK_TRUE, UINT64_MAX));
			VT_VK_CHECK(vkResetFences(device->GetHandle<VkDevice>(), 1, &currentCommandBuffer.fence));
			VT_VK_CHECK(vkResetCommandPool(device->GetHandle<VkDevice>(), currentCommandBuffer.commandPool, 0));
		}

		// Begin command buffer
		{
			VkCommandBufferBeginInfo beginInfo{};
			beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
			beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

			VT_VK_CHECK(vkBeginCommandBuffer(currentCommandBuffer.commandBuffer, &beginInfo));
		}

		if (m_hasTimestampSupport)
		{
			vkCmdResetQueryPool(currentCommandBuffer.commandBuffer, m_timestampQueryPools.at(index), 0, m_timestampQueryCount);
			vkCmdWriteTimestamp2(currentCommandBuffer.commandBuffer, VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT, m_timestampQueryPools.at(index), 0);

			m_timestampCounts[lastIndex] = m_nextAvailableTimestampQuery;
			m_nextAvailableTimestampQuery = 2;
		}
	}

	void VulkanCommandBuffer::End()
	{
		VT_PROFILE_FUNCTION();

		const uint32_t index = GetCurrentCommandBufferIndex();

		if (m_hasTimestampSupport)
		{
			vkCmdWriteTimestamp2(m_commandBuffers.at(index).commandBuffer, VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT, m_timestampQueryPools.at(index), 1);
		}

		VT_VK_CHECK(vkEndCommandBuffer(m_commandBuffers.at(index).commandBuffer));
	}

	void VulkanCommandBuffer::Execute()
	{
		VT_PROFILE_FUNCTION();

		if (!m_isSwapchainTarget)
		{
			auto device = GraphicsContext::GetDevice();
			device->GetDeviceQueue(m_queueType)->Execute({ As<VulkanCommandBuffer>() });
		}

		FetchTimestampResults();
	}

	void VulkanCommandBuffer::ExecuteAndWait()
	{
		VT_PROFILE_FUNCTION();

		if (!m_isSwapchainTarget)
		{
			auto device = GraphicsContext::GetDevice();
			device->GetDeviceQueue(m_queueType)->Execute({ As<VulkanCommandBuffer>() });

			const uint32_t index = GetCurrentCommandBufferIndex();
			VT_VK_CHECK(vkWaitForFences(device->GetHandle<VkDevice>(), 1, &m_commandBuffers.at(index).fence, VK_TRUE, UINT64_MAX));
		}

		FetchTimestampResults();
	}

	void VulkanCommandBuffer::WaitForLastFence()
	{
		auto device = GraphicsContext::GetDevice();
		VT_VK_CHECK(vkWaitForFences(device->GetHandle<VkDevice>(), 1, &m_commandBuffers.at(m_lastCommandBufferIndex).fence, VK_TRUE, UINT64_MAX));
	}

	void VulkanCommandBuffer::WaitForFences()
	{
		std::vector<VkFence> fences{};
		for (const auto& cmdBuffer : m_commandBuffers)
		{
			fences.emplace_back(cmdBuffer.fence);
		}

		auto device = GraphicsContext::GetDevice();
		VT_VK_CHECK(vkWaitForFences(device->GetHandle<VkDevice>(), static_cast<uint32_t>(fences.size()), fences.data(), VK_TRUE, UINT64_MAX));
	}

	void VulkanCommandBuffer::Draw(const uint32_t vertexCount, const uint32_t instanceCount, const uint32_t firstVertex, const uint32_t firstInstance)
	{
		const uint32_t index = GetCurrentCommandBufferIndex();
		VT_PROFILE_GPU_CONTEXT(m_commandBuffers.at(index).commandBuffer);
		VT_PROFILE_GPU_EVENT("Draw");

		vkCmdDraw(m_commandBuffers.at(index).commandBuffer, vertexCount, instanceCount, firstVertex, firstInstance);
	}

	void VulkanCommandBuffer::DrawIndexed(const uint32_t indexCount, const uint32_t instanceCount, const uint32_t firstIndex, const uint32_t vertexOffset, const uint32_t firstInstance)
	{
		const uint32_t index = GetCurrentCommandBufferIndex();
		VT_PROFILE_GPU_CONTEXT(m_commandBuffers.at(index).commandBuffer);
		VT_PROFILE_GPU_EVENT("DrawIndexed");

		vkCmdDrawIndexed(m_commandBuffers.at(index).commandBuffer, indexCount, instanceCount, firstIndex, vertexOffset, firstInstance);
	}

	void VulkanCommandBuffer::DrawIndexedIndirect(Ref<StorageBuffer> commandsBuffer, const size_t offset, const uint32_t drawCount, const uint32_t stride)
	{
		const uint32_t index = GetCurrentCommandBufferIndex();
		VT_PROFILE_GPU_CONTEXT(m_commandBuffers.at(index).commandBuffer);
		VT_PROFILE_GPU_EVENT("DrawIndexedIndirect");

		vkCmdDrawIndexedIndirect(m_commandBuffers.at(index).commandBuffer, commandsBuffer->GetHandle<VkBuffer>(), offset, drawCount, stride);
	}

	void VulkanCommandBuffer::DrawIndirect(Ref<StorageBuffer> commandsBuffer, const size_t offset, const uint32_t drawCount, const uint32_t stride)
	{
		const uint32_t index = GetCurrentCommandBufferIndex();
		VT_PROFILE_GPU_CONTEXT(m_commandBuffers.at(index).commandBuffer);
		VT_PROFILE_GPU_EVENT("DrawIndirect");

		vkCmdDrawIndirect(m_commandBuffers.at(index).commandBuffer, commandsBuffer->GetHandle<VkBuffer>(), offset, drawCount, stride);
	}

	void VulkanCommandBuffer::DrawIndexedIndirectCount(Ref<StorageBuffer> commandsBuffer, const size_t offset, Ref<StorageBuffer> countBuffer, const size_t countBufferOffset, const uint32_t maxDrawCount, const uint32_t stride)
	{
		const uint32_t index = GetCurrentCommandBufferIndex();
		VT_PROFILE_GPU_CONTEXT(m_commandBuffers.at(index).commandBuffer);
		VT_PROFILE_GPU_EVENT("DrawIndexedIndirectCount");

		vkCmdDrawIndexedIndirectCount(m_commandBuffers.at(index).commandBuffer, commandsBuffer->GetHandle<VkBuffer>(), offset, countBuffer->GetHandle<VkBuffer>(), countBufferOffset, maxDrawCount, stride);
	}

	void VulkanCommandBuffer::DrawIndirectCount(Ref<StorageBuffer> commandsBuffer, const size_t offset, Ref<StorageBuffer> countBuffer, const size_t countBufferOffset, const uint32_t maxDrawCount, const uint32_t stride)
	{
		const uint32_t index = GetCurrentCommandBufferIndex();
		VT_PROFILE_GPU_CONTEXT(m_commandBuffers.at(index).commandBuffer);
		VT_PROFILE_GPU_EVENT("DrawIndirectCount");

		vkCmdDrawIndirectCount(m_commandBuffers.at(index).commandBuffer, commandsBuffer->GetHandle<VkBuffer>(), offset, countBuffer->GetHandle<VkBuffer>(), countBufferOffset, maxDrawCount, stride);
	}

	void VulkanCommandBuffer::Dispatch(const uint32_t groupCountX, const uint32_t groupCountY, const uint32_t groupCountZ)
	{
		const uint32_t index = GetCurrentCommandBufferIndex();
		VT_PROFILE_GPU_CONTEXT(m_commandBuffers.at(index).commandBuffer);
		VT_PROFILE_GPU_EVENT("Dispatch");

		vkCmdDispatch(m_commandBuffers.at(index).commandBuffer, groupCountX, groupCountY, groupCountZ);
	}

	void VulkanCommandBuffer::SetViewports(const std::vector<Viewport>& viewports)
	{
		const uint32_t index = GetCurrentCommandBufferIndex();
		VT_PROFILE_GPU_CONTEXT(m_commandBuffers.at(index).commandBuffer)

			vkCmdSetViewport(m_commandBuffers.at(index).commandBuffer, 0, static_cast<uint32_t>(viewports.size()), reinterpret_cast<const VkViewport*>(viewports.data()));
	}

	void VulkanCommandBuffer::SetScissors(const std::vector<Rect2D>& scissors)
	{
		const uint32_t index = GetCurrentCommandBufferIndex();
		VT_PROFILE_GPU_CONTEXT(m_commandBuffers.at(index).commandBuffer);

		vkCmdSetScissor(m_commandBuffers.at(index).commandBuffer, 0, static_cast<uint32_t>(scissors.size()), reinterpret_cast<const VkRect2D*>(scissors.data()));
	}

	void VulkanCommandBuffer::BindPipeline(Ref<RenderPipeline> pipeline)
	{
		m_currentComputePipeline.Reset();

		if (pipeline == nullptr)
		{
			m_currentRenderPipeline.Reset();
			return;
		}

		m_currentRenderPipeline = pipeline;

		const uint32_t index = GetCurrentCommandBufferIndex();
		VT_PROFILE_GPU_CONTEXT(m_commandBuffers.at(index).commandBuffer);

		vkCmdBindPipeline(m_commandBuffers.at(index).commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline->GetHandle<VkPipeline>());
	}

	void VulkanCommandBuffer::BindPipeline(Ref<ComputePipeline> pipeline)
	{
		m_currentRenderPipeline.Reset();

		if (pipeline == nullptr)
		{
			m_currentComputePipeline.Reset();
			return;
		}

		m_currentComputePipeline = pipeline;

		const uint32_t index = GetCurrentCommandBufferIndex();
		VT_PROFILE_GPU_CONTEXT(m_commandBuffers.at(index).commandBuffer);

		vkCmdBindPipeline(m_commandBuffers.at(index).commandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, pipeline->GetHandle<VkPipeline>());
	}

	void VulkanCommandBuffer::BindVertexBuffers(const std::vector<Ref<VertexBuffer>>& vertexBuffers, const uint32_t firstBinding)
	{
		std::vector<VkBuffer> vkBuffers{ vertexBuffers.size() };
		std::vector<VkDeviceSize> offsets{};

		for (uint32_t i = 0; i < vertexBuffers.size(); i++)
		{
			vkBuffers[i] = vertexBuffers[i]->GetHandle<VkBuffer>();
			offsets.emplace_back(0);
		}

		const uint32_t index = GetCurrentCommandBufferIndex();
		VT_PROFILE_GPU_CONTEXT(m_commandBuffers.at(index).commandBuffer);

		vkCmdBindVertexBuffers(m_commandBuffers.at(index).commandBuffer, firstBinding, static_cast<uint32_t>(vkBuffers.size()), vkBuffers.data(), offsets.data());
	}

	void VulkanCommandBuffer::BindIndexBuffer(Ref<IndexBuffer> indexBuffer)
	{
		const uint32_t index = GetCurrentCommandBufferIndex();
		VT_PROFILE_GPU_CONTEXT(m_commandBuffers.at(index).commandBuffer);

		constexpr VkDeviceSize offset = 0;
		vkCmdBindIndexBuffer(m_commandBuffers.at(index).commandBuffer, indexBuffer->GetHandle<VkBuffer>(), offset, VK_INDEX_TYPE_UINT32);
	}

	void VulkanCommandBuffer::BindDescriptorTable(Ref<DescriptorTable> descriptorTable)
	{
		VT_PROFILE_FUNCTION();

		VulkanDescriptorTable& vulkanDescriptorTable = descriptorTable->AsRef<VulkanDescriptorTable>();
		const uint32_t index = GetCurrentCommandBufferIndex();
		VT_PROFILE_GPU_CONTEXT(m_commandBuffers.at(index).commandBuffer);

		vulkanDescriptorTable.Update(index);

		VkPipelineBindPoint bindPoint = m_currentRenderPipeline ? VK_PIPELINE_BIND_POINT_GRAPHICS : VK_PIPELINE_BIND_POINT_COMPUTE;

		// #TODO_Ivar: move to an implementation that binds all descriptor sets in one call
		for (const auto& [set, sets] : vulkanDescriptorTable.GetDescriptorSets())
		{
			vkCmdBindDescriptorSets(m_commandBuffers.at(index).commandBuffer, bindPoint, GetCurrentPipelineLayout(), set, 1, &sets.at(index), 0, nullptr);
		}
	}

	void VulkanCommandBuffer::BeginRendering(const RenderingInfo& renderingInfo)
	{
		VT_PROFILE_FUNCTION();

		std::vector<VkRenderingAttachmentInfo> colorAttachmentInfo{};
		VkRenderingAttachmentInfo depthAttachmentInfo{};

		for (const auto& colorAtt : renderingInfo.colorAttachments)
		{
			auto& newInfo = colorAttachmentInfo.emplace_back();
			newInfo.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO;
			newInfo.imageView = colorAtt.view->GetHandle<VkImageView>();
			newInfo.imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
			newInfo.loadOp = Utility::VoltToVulkanLoadOp(colorAtt.clearMode);
			newInfo.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
			newInfo.clearValue = { colorAtt.clearColor[0], colorAtt.clearColor[1], colorAtt.clearColor[2], colorAtt.clearColor[3] };
		}

		if (!renderingInfo.depthAttachmentInfo.view)
		{
			depthAttachmentInfo.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO;
			depthAttachmentInfo.imageView = renderingInfo.depthAttachmentInfo.view->GetHandle<VkImageView>();
			depthAttachmentInfo.imageLayout = VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL;
			depthAttachmentInfo.loadOp = Utility::VoltToVulkanLoadOp(renderingInfo.depthAttachmentInfo.clearMode);
			depthAttachmentInfo.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
			depthAttachmentInfo.clearValue.depthStencil = { renderingInfo.depthAttachmentInfo.clearColor[0], static_cast<uint32_t>(renderingInfo.depthAttachmentInfo.clearColor[1]) };
		}

		VkRenderingInfo vkRenderingInfo{};
		vkRenderingInfo.sType = VK_STRUCTURE_TYPE_RENDERING_INFO;
		vkRenderingInfo.renderArea = { renderingInfo.renderArea.offset.x, renderingInfo.renderArea.offset.y, renderingInfo.renderArea.extent.width, renderingInfo.renderArea.extent.height };
		vkRenderingInfo.layerCount = renderingInfo.layerCount;
		vkRenderingInfo.colorAttachmentCount = static_cast<uint32_t>(colorAttachmentInfo.size());
		vkRenderingInfo.pColorAttachments = colorAttachmentInfo.data();
		vkRenderingInfo.pStencilAttachment = nullptr;

		if (!renderingInfo.depthAttachmentInfo.view)
		{
			vkRenderingInfo.pDepthAttachment = &depthAttachmentInfo;
		}
		else
		{
			vkRenderingInfo.pDepthAttachment = nullptr;
		}

		const uint32_t index = GetCurrentCommandBufferIndex();
		VT_PROFILE_GPU_CONTEXT(m_commandBuffers.at(index).commandBuffer);

		vkCmdBeginRendering(m_commandBuffers.at(index).commandBuffer, &vkRenderingInfo);
	}

	void VulkanCommandBuffer::EndRendering()
	{
		VT_PROFILE_FUNCTION();

		const uint32_t index = GetCurrentCommandBufferIndex();
		VT_PROFILE_GPU_CONTEXT(m_commandBuffers.at(index).commandBuffer);

		vkCmdEndRendering(m_commandBuffers.at(index).commandBuffer);
	}

	void VulkanCommandBuffer::PushConstants(const void* data, const uint32_t size, const uint32_t offset)
	{
#ifndef VT_DIST
		if (!m_currentRenderPipeline && !m_currentComputePipeline)
		{
			GraphicsContext::LogTagged(Severity::Error, "[VulkanCommandBuffer]", "Unable to push constants as no pipeline is currently bound!");
		}
#endif

		const uint32_t index = GetCurrentCommandBufferIndex();
		VT_PROFILE_GPU_CONTEXT(m_commandBuffers.at(index).commandBuffer);

		VkPipelineLayout pipelineLayout = nullptr;
		VkPipelineStageFlags stageFlags = 0;

		if (m_currentRenderPipeline)
		{
			auto& vkPipeline = m_currentRenderPipeline->AsRef<VulkanRenderPipeline>();
			pipelineLayout = vkPipeline.GetPipelineLayout();
			stageFlags = static_cast<VkPipelineStageFlags>(vkPipeline.GetShader()->GetResources().constants.stageFlags);
		}
		else
		{
			auto& vkPipeline = m_currentComputePipeline->AsRef<VulkanComputePipeline>();
			pipelineLayout = vkPipeline.GetPipelineLayout();
			stageFlags = static_cast<VkPipelineStageFlags>(vkPipeline.GetShader()->GetResources().constants.stageFlags);
		}

		vkCmdPushConstants(m_commandBuffers.at(index).commandBuffer, pipelineLayout, stageFlags, offset, size, data);
	}

	void VulkanCommandBuffer::ResourceBarrier(const std::vector<ResourceBarrierInfo>& resourceBarriers)
	{
		VT_PROFILE_FUNCTION();

		std::vector<VkImageMemoryBarrier2> imageBarriers{};
		std::vector<VkBufferMemoryBarrier2> bufferBarriers{};

		for (const auto& resourceBarrier : resourceBarriers)
		{
			auto resourcePtr = resourceBarrier.resource;

			if (resourcePtr->GetType() == ResourceType::Image2D)
			{
				auto [srcStage, srcAccess] = Utility::GetSrcInfoFromResourceState(resourceBarrier.oldState);
				auto [dstStage, dstAccess] = Utility::GetDstInfoFromResourceState(resourceBarrier.newState);
				const auto dstLayout = Utility::GetDstLayoutFromResourceState(resourceBarrier.newState);

				auto& vkImage = resourceBarrier.resource->AsRef<VulkanImage2D>();

				auto& barrier = imageBarriers.emplace_back();
				barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2;
				barrier.pNext = nullptr;
				barrier.srcStageMask = srcStage;
				barrier.srcAccessMask = srcAccess;
				barrier.dstStageMask = dstStage;
				barrier.dstAccessMask = dstAccess;
				barrier.newLayout = dstLayout;
				barrier.oldLayout = static_cast<VkImageLayout>(vkImage.m_currentImageLayout);
				barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
				barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
				barrier.subresourceRange.aspectMask = static_cast<VkImageAspectFlags>(vkImage.m_imageAspect);
				barrier.subresourceRange.baseArrayLayer = 0;
				barrier.subresourceRange.baseMipLevel = 0;
				barrier.subresourceRange.layerCount = VK_REMAINING_ARRAY_LAYERS;
				barrier.subresourceRange.levelCount = VK_REMAINING_MIP_LEVELS;
				barrier.image = vkImage.GetHandle<VkImage>();

				vkImage.m_currentImageLayout = static_cast<uint32_t>(dstLayout);
			}
			else if (resourcePtr->GetType() == ResourceType::StorageBuffer)
			{
				auto [srcStage, srcAccess] = Utility::GetSrcInfoFromResourceState(resourceBarrier.oldState);
				auto [dstStage, dstAccess] = Utility::GetDstInfoFromResourceState(resourceBarrier.newState);

				auto& vkBuffer = resourceBarrier.resource->AsRef<VulkanStorageBuffer>();
				auto& barrier = bufferBarriers.emplace_back();
				barrier.sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER_2;
				barrier.pNext = nullptr;
				barrier.srcStageMask = srcStage;
				barrier.srcAccessMask = srcAccess;
				barrier.dstStageMask = dstStage;
				barrier.dstAccessMask = dstAccess;
				barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
				barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
				barrier.offset = 0;
				barrier.size = vkBuffer.GetByteSize();
				barrier.buffer = vkBuffer.GetHandle<VkBuffer>();
			}
		}

		VkDependencyInfo info{};
		info.sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO;
		info.pNext = nullptr;
		info.dependencyFlags = 0;
		info.memoryBarrierCount = 0;
		info.pMemoryBarriers = nullptr;
		info.bufferMemoryBarrierCount = static_cast<uint32_t>(bufferBarriers.size());
		info.pBufferMemoryBarriers = bufferBarriers.data();
		info.imageMemoryBarrierCount = static_cast<uint32_t>(imageBarriers.size());
		info.pImageMemoryBarriers = imageBarriers.data();

		const uint32_t index = GetCurrentCommandBufferIndex();
		VT_PROFILE_GPU_CONTEXT(m_commandBuffers.at(index).commandBuffer);

		vkCmdPipelineBarrier2(m_commandBuffers.at(index).commandBuffer, &info);
	}

	const uint32_t VulkanCommandBuffer::BeginTimestamp()
	{
		if (!m_hasTimestampSupport)
		{
			return 0;
		}

		const uint32_t index = GetCurrentCommandBufferIndex();
		const uint32_t queryId = m_nextAvailableTimestampQuery;
		m_nextAvailableTimestampQuery += 2;

		vkCmdWriteTimestamp2(m_commandBuffers.at(index).commandBuffer, VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT, m_timestampQueryPools.at(index), queryId);
		return queryId;
	}

	void VulkanCommandBuffer::EndTimestamp(uint32_t timestampIndex)
	{
		if (!m_hasTimestampSupport)
		{
			return;
		}

		const uint32_t index = GetCurrentCommandBufferIndex();
		vkCmdWriteTimestamp2(m_commandBuffers.at(index).commandBuffer, VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT, m_timestampQueryPools.at(index), timestampIndex + 1);
	}

	const float VulkanCommandBuffer::GetExecutionTime(uint32_t timestampIndex) const
	{
		if (!m_hasTimestampSupport)
		{
			return 0.f;
		}

		const uint32_t currentIndex = GetCurrentCommandBufferIndex();
		const uint32_t timestampsIndex = (currentIndex + 1) % m_commandBufferCount;

		if (timestampIndex == UINT32_MAX || timestampIndex / 2 >= m_timestampCounts.at(timestampsIndex) / 2)
		{
			return 0.f;
		}

		return m_executionTimes.at(timestampsIndex).at(timestampIndex / 2);
	}

	void VulkanCommandBuffer::CopyImageToBackBuffer(Ref<Image2D> srcImage)
	{
		VulkanImage2D& vkImage = srcImage->AsRef<VulkanImage2D>();
		VulkanSwapchain& swapchain = VulkanSwapchain::Get();

		VkImageBlit blitRegion{};
		blitRegion.srcSubresource.aspectMask = static_cast<VkImageAspectFlags>(vkImage.GetImageAspect());
		blitRegion.srcSubresource.baseArrayLayer = 0;
		blitRegion.srcSubresource.layerCount = 1;
		blitRegion.srcSubresource.mipLevel = 0;

		blitRegion.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		blitRegion.dstSubresource.baseArrayLayer = 0;
		blitRegion.dstSubresource.layerCount = 1;
		blitRegion.dstSubresource.mipLevel = 0;

		blitRegion.srcOffsets[0] = { 0, 0, 0 };
		blitRegion.srcOffsets[1] = { static_cast<int32_t>(vkImage.GetWidth()), static_cast<int32_t>(vkImage.GetHeight()), 1 };

		blitRegion.dstOffsets[0] = { 0, 0, 0 };
		blitRegion.dstOffsets[1] = { static_cast<int32_t>(swapchain.GetWidth()), static_cast<int32_t>(swapchain.GetHeight()), 1 };

		VkImageSubresourceRange range{};
		range.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		range.baseArrayLayer = 0;
		range.baseMipLevel = 0;
		range.layerCount = 1;
		range.levelCount = 1;

		const uint32_t index = GetCurrentCommandBufferIndex();

		// First Transition
		{
			VkImageMemoryBarrier2 srcImageBarrier{};
			srcImageBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2;
			srcImageBarrier.pNext = nullptr;
			srcImageBarrier.srcStageMask = VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_2_FRAGMENT_SHADER_BIT;
			srcImageBarrier.srcAccessMask = VK_ACCESS_2_SHADER_WRITE_BIT | VK_ACCESS_2_SHADER_READ_BIT | VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT;
			srcImageBarrier.dstStageMask = VK_PIPELINE_STAGE_2_TRANSFER_BIT;
			srcImageBarrier.dstAccessMask = VK_ACCESS_2_TRANSFER_READ_BIT;
			srcImageBarrier.oldLayout = static_cast<VkImageLayout>(vkImage.GetCurrentLayout());
			srcImageBarrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
			srcImageBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
			srcImageBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
			srcImageBarrier.subresourceRange = range;
			srcImageBarrier.image = vkImage.GetHandle<VkImage>();

			VkImageMemoryBarrier2 dstImageBarrier{};
			dstImageBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2;
			dstImageBarrier.pNext = nullptr;
			dstImageBarrier.srcStageMask = VK_PIPELINE_STAGE_2_NONE;
			dstImageBarrier.srcAccessMask = 0;
			dstImageBarrier.dstStageMask = VK_PIPELINE_STAGE_2_TRANSFER_BIT;
			dstImageBarrier.dstAccessMask = VK_ACCESS_2_TRANSFER_WRITE_BIT;
			dstImageBarrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
			dstImageBarrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
			dstImageBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
			dstImageBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
			dstImageBarrier.subresourceRange = range;
			dstImageBarrier.image = swapchain.GetCurrentImage();

			const VkImageMemoryBarrier2 barriers[2] = { srcImageBarrier, dstImageBarrier };

			VkDependencyInfo depInfo{};
			depInfo.sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO;
			depInfo.pNext = nullptr;
			depInfo.dependencyFlags = 0;
			depInfo.memoryBarrierCount = 0;
			depInfo.pMemoryBarriers = nullptr;
			depInfo.bufferMemoryBarrierCount = 0;
			depInfo.pBufferMemoryBarriers = nullptr;
			depInfo.imageMemoryBarrierCount = 2;
			depInfo.pImageMemoryBarriers = barriers;

			vkCmdPipelineBarrier2(m_commandBuffers.at(index).commandBuffer, &depInfo);
		}

		vkCmdBlitImage(m_commandBuffers.at(index).commandBuffer, vkImage.GetHandle<VkImage>(), VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, swapchain.GetCurrentImage(), VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &blitRegion, VK_FILTER_NEAREST);

		// Second Transition
		{
			VkImageMemoryBarrier2 srcImageBarrier{};
			srcImageBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2;
			srcImageBarrier.pNext = nullptr;
			srcImageBarrier.srcStageMask = VK_PIPELINE_STAGE_2_TRANSFER_BIT;
			srcImageBarrier.srcAccessMask = VK_ACCESS_2_TRANSFER_READ_BIT;
			srcImageBarrier.dstStageMask = VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_2_FRAGMENT_SHADER_BIT;
			srcImageBarrier.dstAccessMask = VK_ACCESS_2_SHADER_WRITE_BIT | VK_ACCESS_2_SHADER_READ_BIT | VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT;
			srcImageBarrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
			srcImageBarrier.newLayout = static_cast<VkImageLayout>(vkImage.GetCurrentLayout());
			srcImageBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
			srcImageBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
			srcImageBarrier.subresourceRange = range;
			srcImageBarrier.image = vkImage.GetHandle<VkImage>();

			VkImageMemoryBarrier2 dstImageBarrier{};
			dstImageBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2;
			dstImageBarrier.pNext = nullptr;
			dstImageBarrier.srcStageMask = VK_PIPELINE_STAGE_2_TRANSFER_BIT;
			dstImageBarrier.dstAccessMask = VK_ACCESS_2_TRANSFER_WRITE_BIT;
			dstImageBarrier.dstStageMask = VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT;
			dstImageBarrier.dstAccessMask = 0;
			dstImageBarrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
			dstImageBarrier.newLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
			dstImageBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
			dstImageBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
			dstImageBarrier.subresourceRange = range;
			dstImageBarrier.image = swapchain.GetCurrentImage();

			const VkImageMemoryBarrier2 barriers[2] = { srcImageBarrier, dstImageBarrier };

			VkDependencyInfo depInfo{};
			depInfo.sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO;
			depInfo.pNext = nullptr;
			depInfo.dependencyFlags = 0;
			depInfo.memoryBarrierCount = 0;
			depInfo.pMemoryBarriers = nullptr;
			depInfo.bufferMemoryBarrierCount = 0;
			depInfo.pBufferMemoryBarriers = nullptr;
			depInfo.imageMemoryBarrierCount = 2;
			depInfo.pImageMemoryBarriers = barriers;

			vkCmdPipelineBarrier2(m_commandBuffers.at(index).commandBuffer, &depInfo);
		}
	}

	void VulkanCommandBuffer::ClearImage(Ref<Image2D> image, std::array<float, 4> clearColor)
	{
		VulkanImage2D& vkImage = image->AsRef<VulkanImage2D>();

		const VkImageAspectFlags imageAspect = static_cast<VkImageAspectFlags>(vkImage.GetImageAspect());
		const uint32_t index = GetCurrentCommandBufferIndex();

		VkImageSubresourceRange range{};
		range.aspectMask = imageAspect;
		range.baseArrayLayer = 0;
		range.baseMipLevel = 0;
		range.layerCount = VK_REMAINING_ARRAY_LAYERS;
		range.levelCount = VK_REMAINING_MIP_LEVELS;

		// First transition
		{
			VkImageMemoryBarrier2 srcImageBarrier{};
			srcImageBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2;
			srcImageBarrier.pNext = nullptr;
			srcImageBarrier.srcStageMask = VK_PIPELINE_STAGE_2_FRAGMENT_SHADER_BIT | VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT;
			srcImageBarrier.srcAccessMask = VK_ACCESS_2_SHADER_WRITE_BIT | VK_ACCESS_2_SHADER_READ_BIT | VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT;
			srcImageBarrier.dstStageMask = VK_PIPELINE_STAGE_2_TRANSFER_BIT;
			srcImageBarrier.dstAccessMask = VK_ACCESS_2_TRANSFER_WRITE_BIT;
			srcImageBarrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
			srcImageBarrier.oldLayout = static_cast<VkImageLayout>(vkImage.GetCurrentLayout());
			srcImageBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
			srcImageBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
			srcImageBarrier.subresourceRange = range;
			srcImageBarrier.image = vkImage.GetHandle<VkImage>();

			VkDependencyInfo depInfo{};
			depInfo.sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO;
			depInfo.pNext = nullptr;
			depInfo.dependencyFlags = 0;
			depInfo.memoryBarrierCount = 0;
			depInfo.pMemoryBarriers = nullptr;
			depInfo.bufferMemoryBarrierCount = 0;
			depInfo.pBufferMemoryBarriers = nullptr;
			depInfo.imageMemoryBarrierCount = 2;
			depInfo.pImageMemoryBarriers = &srcImageBarrier;

			vkCmdPipelineBarrier2(m_commandBuffers.at(index).commandBuffer, &depInfo);
		}

		if (imageAspect & VK_IMAGE_ASPECT_COLOR_BIT)
		{
			VkClearColorValue vkClearColor{};
			vkClearColor.float32[0] = clearColor[0];
			vkClearColor.float32[1] = clearColor[1];
			vkClearColor.float32[2] = clearColor[2];
			vkClearColor.float32[3] = clearColor[3];

			vkCmdClearColorImage(m_commandBuffers.at(index).commandBuffer, image->GetHandle<VkImage>(), VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, &vkClearColor, 1, &range);
		}
		else
		{
			VkClearDepthStencilValue vkClearColor{};
			vkClearColor.depth = clearColor[0];
			vkClearColor.stencil = static_cast<uint32_t>(clearColor[1]);

			vkCmdClearDepthStencilImage(m_commandBuffers.at(index).commandBuffer, image->GetHandle<VkImage>(), VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, &vkClearColor, 1, &range);
		}

		// Second transition
		{
			VkImageMemoryBarrier2 srcImageBarrier{};
			srcImageBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2;
			srcImageBarrier.pNext = nullptr;
			srcImageBarrier.srcStageMask = VK_PIPELINE_STAGE_2_TRANSFER_BIT;
			srcImageBarrier.srcAccessMask = VK_ACCESS_2_TRANSFER_WRITE_BIT;
			srcImageBarrier.dstStageMask = VK_PIPELINE_STAGE_2_FRAGMENT_SHADER_BIT | VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT;
			srcImageBarrier.dstAccessMask = VK_ACCESS_2_SHADER_WRITE_BIT | VK_ACCESS_2_SHADER_READ_BIT | VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT;
			srcImageBarrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
			srcImageBarrier.newLayout = static_cast<VkImageLayout>(vkImage.GetCurrentLayout());
			srcImageBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
			srcImageBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
			srcImageBarrier.subresourceRange = range;
			srcImageBarrier.image = vkImage.GetHandle<VkImage>();

			VkDependencyInfo depInfo{};
			depInfo.sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO;
			depInfo.pNext = nullptr;
			depInfo.dependencyFlags = 0;
			depInfo.memoryBarrierCount = 0;
			depInfo.pMemoryBarriers = nullptr;
			depInfo.bufferMemoryBarrierCount = 0;
			depInfo.pBufferMemoryBarriers = nullptr;
			depInfo.imageMemoryBarrierCount = 2;
			depInfo.pImageMemoryBarriers = &srcImageBarrier;

			vkCmdPipelineBarrier2(m_commandBuffers.at(index).commandBuffer, &depInfo);
		}
	}

	void VulkanCommandBuffer::CopyBufferRegion(Ref<Allocation> srcResource, const size_t srcOffset, Ref<Allocation> dstResource, const size_t dstOffset, const size_t size)
	{
		const uint32_t index = GetCurrentCommandBufferIndex();

		VkBufferCopy copy{};
		copy.srcOffset = srcOffset;
		copy.dstOffset = dstOffset;
		copy.size = size;

		vkCmdCopyBuffer(m_commandBuffers.at(index).commandBuffer, srcResource->GetResourceHandle<VkBuffer>(), dstResource->GetResourceHandle<VkBuffer>(), 1, &copy);
	}

	void VulkanCommandBuffer::CopyBufferToImage(Ref<Allocation> srcBuffer, Ref<Image2D> dstImage, const uint32_t width, const uint32_t height, const uint32_t mip)
	{
		const uint32_t index = GetCurrentCommandBufferIndex();

		auto& vkImage = dstImage->AsRef<VulkanImage2D>();


		VkBufferImageCopy region{};
		region.bufferOffset = 0;
		region.bufferRowLength = 0;
		region.bufferImageHeight = 0;

		region.imageSubresource.aspectMask = static_cast<VkImageAspectFlags>(vkImage.m_imageAspect);
		region.imageSubresource.mipLevel = mip;
		region.imageSubresource.baseArrayLayer = 0;
		region.imageSubresource.layerCount = 1;

		region.imageOffset = { 0, 0, 0 };
		region.imageExtent = { width, height, 1 };

		vkCmdCopyBufferToImage(m_commandBuffers.at(index).commandBuffer, srcBuffer->GetResourceHandle<VkBuffer>(), dstImage->GetHandle<VkImage>(), static_cast<VkImageLayout>(vkImage.m_currentImageLayout), 1, &region);
	}

	const uint32_t VulkanCommandBuffer::GetCurrentIndex() const
	{
		return m_currentCommandBufferIndex;
	}

	VkFence_T* VulkanCommandBuffer::GetCurrentFence() const
	{
		return m_commandBuffers.at(m_currentCommandBufferIndex).fence;
	}

	void* VulkanCommandBuffer::GetHandleImpl() const
	{
		return m_commandBuffers.at(m_currentCommandBufferIndex).commandBuffer;
	}

	void VulkanCommandBuffer::Invalidate()
	{
		auto physicalDevice = GraphicsContext::GetPhysicalDevice()->As<VulkanPhysicalGraphicsDevice>();
		auto device = GraphicsContext::GetDevice();

		const auto& queueFamilies = physicalDevice->GetQueueFamilies();

		if (m_isSwapchainTarget)
		{
			auto& swapchain = VulkanSwapchain::Get();
			m_commandBufferCount = VulkanSwapchain::MAX_FRAMES_IN_FLIGHT;
			m_commandBuffers.resize(m_commandBufferCount);

			for (uint32_t i = 0; i < m_commandBufferCount; i++)
			{
				m_commandBuffers[i].commandBuffer = swapchain.GetCommandBuffer(i);
				m_commandBuffers[i].commandPool = swapchain.GetCommandPool(i);
			}

			return;
		}
		else
		{
			m_commandBuffers.resize(m_commandBufferCount);

			for (uint32_t i = 0; i < m_commandBufferCount; i++)
			{
				VkCommandPoolCreateInfo poolInfo{};
				poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;

				uint32_t queueFamilyIndex = 0;

				switch (m_queueType)
				{
					case QueueType::Graphics:
						queueFamilyIndex = queueFamilies.graphicsFamilyQueueIndex;
						break;
					case QueueType::Compute:
						queueFamilyIndex = queueFamilies.computeFamilyQueueIndex;
						break;
					case QueueType::TransferCopy:
						queueFamilyIndex = queueFamilies.transferFamilyQueueIndex;
						break;
					default:
						assert(false);
						break;
				}

				poolInfo.queueFamilyIndex = queueFamilyIndex;
				poolInfo.flags = 0;

				VT_VK_CHECK(vkCreateCommandPool(device->GetHandle<VkDevice>(), &poolInfo, nullptr, &m_commandBuffers[i].commandPool));
			}

			for (uint32_t i = 0; i < m_commandBufferCount; i++)
			{
				VkCommandBufferAllocateInfo allocInfo{};
				allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
				allocInfo.commandPool = m_commandBuffers[i].commandPool;
				allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
				allocInfo.commandBufferCount = 1;

				VT_VK_CHECK(vkAllocateCommandBuffers(device->GetHandle<VkDevice>(), &allocInfo, &m_commandBuffers[i].commandBuffer));
			}

			for (uint32_t i = 0; i < m_commandBufferCount; i++)
			{
				VkFenceCreateInfo info{};
				info.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
				info.flags = VK_FENCE_CREATE_SIGNALED_BIT;

				VT_VK_CHECK(vkCreateFence(device->GetHandle<VkDevice>(), &info, nullptr, &m_commandBuffers[i].fence));
			}
		}

		m_hasTimestampSupport = GraphicsContext::GetPhysicalDevice()->GetCapabilities().timestampSupport;
		if (m_hasTimestampSupport)
		{
			CreateQueryPools();
		}
	}

	void VulkanCommandBuffer::Release()
	{
		auto device = GraphicsContext::GetDevice();
		if (!m_isSwapchainTarget)
		{
			std::vector<VkFence> fences{};
			for (const auto& cmdBuffer : m_commandBuffers)
			{
				fences.emplace_back(cmdBuffer.fence);
			}

			VT_VK_CHECK(vkWaitForFences(device->GetHandle<VkDevice>(), static_cast<uint32_t>(fences.size()), fences.data(), VK_TRUE, UINT64_MAX));

			for (auto& cmdBuffer : m_commandBuffers)
			{
				vkDestroyFence(device->GetHandle<VkDevice>(), cmdBuffer.fence, nullptr);
				vkDestroyCommandPool(device->GetHandle<VkDevice>(), cmdBuffer.commandPool, nullptr);
			}
		}
		else
		{
			std::vector<VkFence> fences{};
			for (uint32_t i = 0; i < VulkanSwapchain::MAX_FRAMES_IN_FLIGHT; i++)
			{
				fences.push_back(VulkanSwapchain::Get().GetFence(i));
			}

			VT_VK_CHECK(vkWaitForFences(device->GetHandle<VkDevice>(), static_cast<uint32_t>(fences.size()), fences.data(), VK_TRUE, UINT64_MAX));
		}

		for (const auto& pool : m_timestampQueryPools)
		{
			vkDestroyQueryPool(device->GetHandle<VkDevice>(), pool, nullptr);
		}

		m_commandBuffers.clear();
	}

	void VulkanCommandBuffer::CreateQueryPools()
	{
		auto device = GraphicsContext::GetDevice();

		m_timestampQueryCount = 2 + 2 * MAX_QUERIES;

		VkQueryPoolCreateInfo info{};
		info.sType = VK_STRUCTURE_TYPE_QUERY_POOL_CREATE_INFO;
		info.pNext = nullptr;
		info.queryType = VK_QUERY_TYPE_TIMESTAMP;
		info.queryCount = m_timestampQueryCount;

		m_timestampQueryPools.resize(m_commandBufferCount);
		for (auto& pool : m_timestampQueryPools)
		{
			VT_VK_CHECK(vkCreateQueryPool(device->GetHandle<VkDevice>(), &info, nullptr, &pool));
		}

		m_timestampCounts = std::vector<uint32_t>(3, 0u);
		m_timestampQueryResults.resize(m_commandBufferCount);
		m_executionTimes.resize(m_commandBufferCount);

		for (auto& results : m_timestampQueryResults)
		{
			results.resize(m_timestampQueryCount);
		}

		for (auto& execTimes : m_executionTimes)
		{
			execTimes.resize(m_timestampQueryCount / 2);
		}
	}

	void VulkanCommandBuffer::FetchTimestampResults()
	{
		if (!m_hasTimestampSupport)
		{
			return;
		}

		const uint32_t currentIndex = GetCurrentCommandBufferIndex();
		const uint32_t timestampsIndex = (currentIndex + 1) % m_commandBufferCount;

		if (m_timestampCounts.at(timestampsIndex) == 0)
		{
			return;
		}

		auto device = GraphicsContext::GetDevice();

		vkGetQueryPoolResults(device->GetHandle<VkDevice>(), m_timestampQueryPools.at(timestampsIndex), 0, m_timestampCounts.at(timestampsIndex), m_timestampCounts.at(timestampsIndex) * sizeof(uint64_t), m_timestampQueryResults.at(timestampsIndex).data(), sizeof(uint64_t), VK_QUERY_RESULT_64_BIT);

		for (uint32_t i = 0; i < m_timestampCounts.at(timestampsIndex); i += 2)
		{
			const uint64_t startTime = m_timestampQueryResults.at(timestampsIndex).at(i);
			const uint64_t endTime = m_timestampQueryResults.at(timestampsIndex).at(i + 1);

			const float nsTime = endTime > startTime ? (endTime - startTime) * GraphicsContext::GetPhysicalDevice()->GetCapabilities().timestampPeriod : 0.f;
			m_executionTimes.at(timestampsIndex)[i / 2] = nsTime * 0.000001f; // Convert to ms
		}
	}

	const uint32_t VulkanCommandBuffer::GetCurrentCommandBufferIndex() const
	{
		return m_currentCommandBufferIndex;
	}

	VkPipelineLayout_T* VulkanCommandBuffer::GetCurrentPipelineLayout()
	{
		VkPipelineLayout pipelineLayout = nullptr;

		if (m_currentRenderPipeline)
		{
			auto& vkPipeline = m_currentRenderPipeline->AsRef<VulkanRenderPipeline>();
			pipelineLayout = vkPipeline.GetPipelineLayout();
		}
		else
		{
			auto& vkPipeline = m_currentComputePipeline->AsRef<VulkanComputePipeline>();
			pipelineLayout = vkPipeline.GetPipelineLayout();
		}

		return pipelineLayout;
	}
}
