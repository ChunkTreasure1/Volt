#include "vkpch.h"
#include "VulkanCommandBuffer.h"

#include "VoltVulkan/Common/VulkanCommon.h"
#include "VoltVulkan/Common/VulkanHelpers.h"
#include "VoltVulkan/Common/VulkanFunctions.h"

#include "VoltVulkan/Graphics/VulkanPhysicalGraphicsDevice.h"
#include "VoltVulkan/Graphics/VulkanSwapchain.h"

#include "VoltVulkan/Pipelines/VulkanRenderPipeline.h"
#include "VoltVulkan/Pipelines/VulkanComputePipeline.h"

#include "VoltVulkan/Descriptors/VulkanDescriptorTable.h"
#include "VoltVulkan/Descriptors/VulkanDescriptorBufferTable.h"

#include "VoltVulkan/Images/VulkanImage2D.h"
#include "VoltVulkan/Buffers/VulkanStorageBuffer.h"
#include "VoltVulkan/Synchronization/VulkanSemaphore.h"

#include "VoltVulkan/Synchronization/VulkanEvent.h"

#include <VoltRHI/Graphics/GraphicsContext.h>
#include <VoltRHI/Graphics/GraphicsDevice.h>
#include <VoltRHI/Graphics/DeviceQueue.h>

#include <VoltRHI/Memory/Allocation.h>

#include <VoltRHI/Buffers/IndexBuffer.h>
#include <VoltRHI/Buffers/VertexBuffer.h>

#include <VoltRHI/Images/ImageView.h>

#include <VoltRHI/Shader/Shader.h>
#include <VoltRHI/Core/Profiling.h>

#include <CoreUtilities/EnumUtils.h>

#ifdef VT_ENABLE_NV_AFTERMATH

#include <GFSDK_Aftermath.h>
#include <GFSDK_Aftermath_Defines.h>
#include <GFSDK_Aftermath_GpuCrashDump.h>

#include <VoltRHI/Utility/NsightAftermathHelpers.h>

#endif

#include <vulkan/vulkan.h>

namespace Volt::RHI
{
	namespace Utility
	{
		const VkPipelineStageFlags2 GetStageFromBarrierSync(const BarrierStage barrierSync)
		{
			VkPipelineStageFlags2 result = VK_PIPELINE_STAGE_2_NONE;

			if (EnumValueContainsFlag(barrierSync, BarrierStage::Draw))
			{
				result |= VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT;
			}

			if (EnumValueContainsFlag(barrierSync, BarrierStage::IndexInput))
			{
				result |= VK_PIPELINE_STAGE_2_INDEX_INPUT_BIT;
			}

			if (EnumValueContainsFlag(barrierSync, BarrierStage::VertexShader))
			{
				result |= VK_PIPELINE_STAGE_2_VERTEX_SHADER_BIT | VK_PIPELINE_STAGE_2_VERTEX_INPUT_BIT;
			}

			if (EnumValueContainsFlag(barrierSync, BarrierStage::PixelShader))
			{
				result |= VK_PIPELINE_STAGE_2_FRAGMENT_SHADER_BIT;
			}

			if (EnumValueContainsFlag(barrierSync, BarrierStage::DepthStencil))
			{
				result |= VK_PIPELINE_STAGE_2_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_2_LATE_FRAGMENT_TESTS_BIT;
			}

			if (EnumValueContainsFlag(barrierSync, BarrierStage::RenderTarget))
			{
				result |= VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT;
			}

			if (EnumValueContainsFlag(barrierSync, BarrierStage::ComputeShader))
			{
				result |= VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT;
			}

			if (EnumValueContainsFlag(barrierSync, BarrierStage::RayTracing))
			{
				result |= VK_PIPELINE_STAGE_2_RAY_TRACING_SHADER_BIT_KHR;
			}

			if (EnumValueContainsFlag(barrierSync, BarrierStage::Copy))
			{
				result |= VK_PIPELINE_STAGE_2_ALL_TRANSFER_BIT;
			}

			if (EnumValueContainsFlag(barrierSync, BarrierStage::Resolve))
			{
				result |= VK_PIPELINE_STAGE_2_RESOLVE_BIT;
			}

			if (EnumValueContainsFlag(barrierSync, BarrierStage::Indirect))
			{
				result |= VK_PIPELINE_STAGE_2_DRAW_INDIRECT_BIT;
			}

			if (EnumValueContainsFlag(barrierSync, BarrierStage::AllGraphics))
			{
				result |= VK_PIPELINE_STAGE_2_ALL_GRAPHICS_BIT;
			}

			if (EnumValueContainsFlag(barrierSync, BarrierStage::VideoDecode))
			{
				result |= VK_PIPELINE_STAGE_2_VIDEO_DECODE_BIT_KHR;
			}

			if (EnumValueContainsFlag(barrierSync, BarrierStage::BuildAccelerationStructure))
			{
				result |= VK_PIPELINE_STAGE_2_ACCELERATION_STRUCTURE_BUILD_BIT_KHR;
			}

			if (EnumValueContainsFlag(barrierSync, BarrierStage::All))
			{
				result |= VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT;
			}

			return result;
		}

		const VkAccessFlags2 GetAccessFromBarrierAccess(const BarrierAccess barrierAccess)
		{
			VkAccessFlags2 result = VK_ACCESS_2_NONE;

			if (EnumValueContainsFlag(barrierAccess, BarrierAccess::VertexBuffer))
			{
				result |= VK_ACCESS_2_VERTEX_ATTRIBUTE_READ_BIT;
			}

			if (EnumValueContainsFlag(barrierAccess, BarrierAccess::UniformBuffer))
			{
				result |= VK_ACCESS_2_UNIFORM_READ_BIT;
			}

			if (EnumValueContainsFlag(barrierAccess, BarrierAccess::IndexBuffer))
			{
				result |= VK_ACCESS_2_INDEX_READ_BIT;
			}

			if (EnumValueContainsFlag(barrierAccess, BarrierAccess::RenderTarget))
			{
				result |= VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT;
			}

			if (EnumValueContainsFlag(barrierAccess, BarrierAccess::ShaderWrite))
			{
				result |= VK_ACCESS_2_SHADER_WRITE_BIT;
			}

			if (EnumValueContainsFlag(barrierAccess, BarrierAccess::DepthStencilWrite))
			{
				result |= VK_ACCESS_2_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
			}

			if (EnumValueContainsFlag(barrierAccess, BarrierAccess::DepthStencilRead))
			{
				result |= VK_ACCESS_2_DEPTH_STENCIL_ATTACHMENT_READ_BIT;
			}

			if (EnumValueContainsFlag(barrierAccess, BarrierAccess::IndirectArgument))
			{
				result |= VK_ACCESS_2_INDIRECT_COMMAND_READ_BIT;
			}

			if (EnumValueContainsFlag(barrierAccess, BarrierAccess::TransferSource))
			{
				result |= VK_ACCESS_2_TRANSFER_READ_BIT;
			}

			if (EnumValueContainsFlag(barrierAccess, BarrierAccess::TransferDestination))
			{
				result |= VK_ACCESS_2_TRANSFER_WRITE_BIT;
			}

			if (EnumValueContainsFlag(barrierAccess, BarrierAccess::ShaderRead))
			{
				result |= VK_ACCESS_2_SHADER_READ_BIT;
			}

			if (EnumValueContainsFlag(barrierAccess, BarrierAccess::AccelerationStructureRead))
			{
				result |= VK_ACCESS_2_ACCELERATION_STRUCTURE_READ_BIT_KHR;
			}

			if (EnumValueContainsFlag(barrierAccess, BarrierAccess::AccelerationStructureWrite))
			{
				result |= VK_ACCESS_2_ACCELERATION_STRUCTURE_WRITE_BIT_KHR;
			}

			if (EnumValueContainsFlag(barrierAccess, BarrierAccess::VideoDecodeRead))
			{
				result |= VK_ACCESS_2_VIDEO_DECODE_READ_BIT_KHR;
			}

			if (EnumValueContainsFlag(barrierAccess, BarrierAccess::VideoDecodeWrite))
			{
				result |= VK_ACCESS_2_VIDEO_DECODE_WRITE_BIT_KHR;
			}

			return result;
		}
	}

	VulkanCommandBuffer::VulkanCommandBuffer(const uint32_t count, QueueType queueType)
		: m_commandBufferCount(count), m_queueType(queueType)
	{
		m_currentCommandBufferIndex = m_commandBufferCount - 1; // This makes sure that we start at command buffer index 0 for clarity

		Invalidate();
	}

	VulkanCommandBuffer::VulkanCommandBuffer(WeakPtr<Swapchain> swapchain)
		: m_swapchainTarget(swapchain), m_queueType(QueueType::Graphics)
	{
		m_currentCommandBufferIndex = m_commandBufferCount - 1;
		m_isSwapchainTarget = true;

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

		// If the next timestamp query is zero, no frame has run before
		if (m_nextAvailableTimestampQuery > 0)
		{
			FetchTimestampResults();
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

		BeginMarker("CommandBuffer", { 1.f, 1.f, 1.f, 1.f });
	}

	void VulkanCommandBuffer::End()
	{
		VT_PROFILE_FUNCTION();

		const uint32_t index = GetCurrentCommandBufferIndex();

		EndMarker();

		if (m_hasTimestampSupport)
		{
			vkCmdWriteTimestamp2(m_commandBuffers.at(index).commandBuffer, VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT, m_timestampQueryPools.at(index), 1);
		}

		VT_VK_CHECK(vkEndCommandBuffer(m_commandBuffers.at(index).commandBuffer));
	}

	void VulkanCommandBuffer::RestartAfterFlush()
	{
		VT_PROFILE_FUNCTION();

		auto device = GraphicsContext::GetDevice();
		const uint32_t index = GetCurrentCommandBufferIndex();
		const auto& currentCommandBuffer = m_commandBuffers.at(index);

		if (!m_isSwapchainTarget)
		{
			VT_VK_CHECK(vkWaitForFences(device->GetHandle<VkDevice>(), 1, &currentCommandBuffer.fence, VK_TRUE, UINT64_MAX));
			VT_VK_CHECK(vkResetFences(device->GetHandle<VkDevice>(), 1, &currentCommandBuffer.fence));
			VT_VK_CHECK(vkResetCommandPool(device->GetHandle<VkDevice>(), currentCommandBuffer.commandPool, 0));
		}

		// If the next timestamp query is zero, no frame has run before
		if (m_nextAvailableTimestampQuery > 0)
		{
			FetchTimestampResults();
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

			m_timestampCounts[m_currentCommandBufferIndex] = m_nextAvailableTimestampQuery;
			m_nextAvailableTimestampQuery = 2;
		}

		BeginMarker("CommandBuffer", { 1.f, 1.f, 1.f, 1.f });
	}

	void VulkanCommandBuffer::Execute()
	{
		VT_PROFILE_FUNCTION();

		if (!m_isSwapchainTarget)
		{
			auto device = GraphicsContext::GetDevice();
			device->GetDeviceQueue(m_queueType)->Execute({ { this } });
		}
	}

	void VulkanCommandBuffer::ExecuteAndWait()
	{
		VT_PROFILE_FUNCTION();

		if (!m_isSwapchainTarget)
		{
			auto device = GraphicsContext::GetDevice();
			device->GetDeviceQueue(m_queueType)->Execute({ { this } });
			VT_VK_CHECK(vkWaitForFences(device->GetHandle<VkDevice>(), 1, &m_commandBuffers.at(m_currentCommandBufferIndex).fence, VK_TRUE, UINT64_MAX));
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

	void VulkanCommandBuffer::SetEvent(WeakPtr<Event> event)
	{
		VkDependencyInfo depInfo{};
		depInfo.sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO;
		depInfo.pNext = nullptr;
		depInfo.dependencyFlags = 0;
		//depInfo.

		//vkCmdSetEvent2()
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

	void VulkanCommandBuffer::DrawIndexedIndirect(WeakPtr<StorageBuffer> commandsBuffer, const size_t offset, const uint32_t drawCount, const uint32_t stride)
	{
		const uint32_t index = GetCurrentCommandBufferIndex();
		VT_PROFILE_GPU_CONTEXT(m_commandBuffers.at(index).commandBuffer);
		VT_PROFILE_GPU_EVENT("DrawIndexedIndirect");

		vkCmdDrawIndexedIndirect(m_commandBuffers.at(index).commandBuffer, commandsBuffer->GetHandle<VkBuffer>(), offset, drawCount, stride);
	}

	void VulkanCommandBuffer::DrawIndirect(WeakPtr<StorageBuffer> commandsBuffer, const size_t offset, const uint32_t drawCount, const uint32_t stride)
	{
		const uint32_t index = GetCurrentCommandBufferIndex();
		VT_PROFILE_GPU_CONTEXT(m_commandBuffers.at(index).commandBuffer);
		VT_PROFILE_GPU_EVENT("DrawIndirect");

		vkCmdDrawIndirect(m_commandBuffers.at(index).commandBuffer, commandsBuffer->GetHandle<VkBuffer>(), offset, drawCount, stride);
	}

	void VulkanCommandBuffer::DrawIndexedIndirectCount(WeakPtr<StorageBuffer> commandsBuffer, const size_t offset, WeakPtr<StorageBuffer> countBuffer, const size_t countBufferOffset, const uint32_t maxDrawCount, const uint32_t stride)
	{
		const uint32_t index = GetCurrentCommandBufferIndex();
		VT_PROFILE_GPU_CONTEXT(m_commandBuffers.at(index).commandBuffer);
		VT_PROFILE_GPU_EVENT("DrawIndexedIndirectCount");

		vkCmdDrawIndexedIndirectCount(m_commandBuffers.at(index).commandBuffer, commandsBuffer->GetHandle<VkBuffer>(), offset, countBuffer->GetHandle<VkBuffer>(), countBufferOffset, maxDrawCount, stride);
	}

	void VulkanCommandBuffer::DrawIndirectCount(WeakPtr<StorageBuffer> commandsBuffer, const size_t offset, WeakPtr<StorageBuffer> countBuffer, const size_t countBufferOffset, const uint32_t maxDrawCount, const uint32_t stride)
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

	void VulkanCommandBuffer::DispatchIndirect(WeakPtr<StorageBuffer> commandsBuffer, const size_t offset)
	{
		const uint32_t index = GetCurrentCommandBufferIndex();
		VT_PROFILE_GPU_CONTEXT(m_commandBuffers.at(index).commandBuffer);
		VT_PROFILE_GPU_EVENT("DispatchIndirect");

		vkCmdDispatchIndirect(m_commandBuffers.at(index).commandBuffer, commandsBuffer->GetHandle<VkBuffer>(), offset);
	}

	void VulkanCommandBuffer::DispatchMeshTasks(const uint32_t groupCountX, const uint32_t groupCountY, const uint32_t groupCountZ)
	{
		const uint32_t index = GetCurrentCommandBufferIndex();
		vkCmdDrawMeshTasksEXT(m_commandBuffers.at(index).commandBuffer, groupCountX, groupCountY, groupCountZ);
	}

	void VulkanCommandBuffer::DispatchMeshTasksIndirect(WeakPtr<StorageBuffer> commandsBuffer, const size_t offset, const uint32_t drawCount, const uint32_t stride)
	{
		const uint32_t index = GetCurrentCommandBufferIndex();
		vkCmdDrawMeshTasksIndirectEXT(m_commandBuffers.at(index).commandBuffer, commandsBuffer->GetHandle<VkBuffer>(), offset, drawCount, stride);
	}

	void VulkanCommandBuffer::DispatchMeshTasksIndirectCount(WeakPtr<StorageBuffer> commandsBuffer, const size_t offset, WeakPtr<StorageBuffer> countBuffer, const size_t countBufferOffset, const uint32_t maxDrawCount, const uint32_t stride)
	{
		const uint32_t index = GetCurrentCommandBufferIndex();
		vkCmdDrawMeshTasksIndirectCountEXT(m_commandBuffers.at(index).commandBuffer, commandsBuffer->GetHandle<VkBuffer>(), offset, countBuffer->GetHandle<VkBuffer>(), countBufferOffset, maxDrawCount, stride);
	}

	void VulkanCommandBuffer::SetViewports(const std::vector<Viewport>& viewports)
	{
		const uint32_t index = GetCurrentCommandBufferIndex();
		VT_PROFILE_GPU_CONTEXT(m_commandBuffers.at(index).commandBuffer);

		vkCmdSetViewport(m_commandBuffers.at(index).commandBuffer, 0, static_cast<uint32_t>(viewports.size()), reinterpret_cast<const VkViewport*>(viewports.data()));
	}

	void VulkanCommandBuffer::SetScissors(const std::vector<Rect2D>& scissors)
	{
		const uint32_t index = GetCurrentCommandBufferIndex();
		VT_PROFILE_GPU_CONTEXT(m_commandBuffers.at(index).commandBuffer);

		vkCmdSetScissor(m_commandBuffers.at(index).commandBuffer, 0, static_cast<uint32_t>(scissors.size()), reinterpret_cast<const VkRect2D*>(scissors.data()));
	}

	void VulkanCommandBuffer::BindPipeline(WeakPtr<RenderPipeline> pipeline)
	{
		m_currentComputePipeline.Reset();

		if (!pipeline)
		{
			m_currentRenderPipeline.Reset();
			return;
		}

		m_currentRenderPipeline = pipeline;

		const uint32_t index = GetCurrentCommandBufferIndex();
		VT_PROFILE_GPU_CONTEXT(m_commandBuffers.at(index).commandBuffer);

		vkCmdBindPipeline(m_commandBuffers.at(index).commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline->GetHandle<VkPipeline>());
	}

	void VulkanCommandBuffer::BindPipeline(WeakPtr<ComputePipeline> pipeline)
	{
		m_currentRenderPipeline.Reset();

		if (!pipeline)
		{
			m_currentComputePipeline.Reset();
			return;
		}

		m_currentComputePipeline = pipeline;

		const uint32_t index = GetCurrentCommandBufferIndex();
		VT_PROFILE_GPU_CONTEXT(m_commandBuffers.at(index).commandBuffer);

		vkCmdBindPipeline(m_commandBuffers.at(index).commandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, pipeline->GetHandle<VkPipeline>());
	}

	void VulkanCommandBuffer::BindVertexBuffers(const std::vector<WeakPtr<VertexBuffer>>& vertexBuffers, const uint32_t firstBinding)
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

	void VulkanCommandBuffer::BindIndexBuffer(WeakPtr<IndexBuffer> indexBuffer)
	{
		const uint32_t index = GetCurrentCommandBufferIndex();
		VT_PROFILE_GPU_CONTEXT(m_commandBuffers.at(index).commandBuffer);

		constexpr VkDeviceSize offset = 0;
		vkCmdBindIndexBuffer(m_commandBuffers.at(index).commandBuffer, indexBuffer->GetHandle<VkBuffer>(), offset, VK_INDEX_TYPE_UINT32);
	}

	void VulkanCommandBuffer::BindIndexBuffer(WeakPtr<StorageBuffer> indexBuffer)
	{
		const uint32_t index = GetCurrentCommandBufferIndex();
		VT_PROFILE_GPU_CONTEXT(m_commandBuffers.at(index).commandBuffer);

		constexpr VkDeviceSize offset = 0;
		vkCmdBindIndexBuffer(m_commandBuffers.at(index).commandBuffer, indexBuffer->GetHandle<VkBuffer>(), offset, VK_INDEX_TYPE_UINT32);
	}

	void VulkanCommandBuffer::BindDescriptorTable(WeakPtr<DescriptorTable> descriptorTable)
	{
		VT_PROFILE_FUNCTION();

		VT_PROFILE_GPU_CONTEXT(m_commandBuffers.at(m_currentCommandBufferIndex).commandBuffer);

		if (GraphicsContext::GetPhysicalDevice()->AsRef<VulkanPhysicalGraphicsDevice>().AreDescriptorBuffersEnabled())
		{
			descriptorTable->AsRef<VulkanDescriptorBufferTable>().Bind(*this);
		}
		else
		{
			descriptorTable->AsRef<VulkanDescriptorTable>().Bind(*this);
		}

	}

	void VulkanCommandBuffer::BeginRendering(const RenderingInfo& renderingInfo)
	{
		VT_PROFILE_FUNCTION();

		StackVector<VkRenderingAttachmentInfo, MAX_COLOR_ATTACHMENT_COUNT> colorAttachmentInfo{};
		VkRenderingAttachmentInfo depthAttachmentInfo{};

		for (const auto& colorAtt : renderingInfo.colorAttachments)
		{
			VkRenderingAttachmentInfo& newInfo = colorAttachmentInfo.Emplace();
			newInfo.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO;
			newInfo.imageView = colorAtt.view->GetHandle<VkImageView>();
			newInfo.imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
			newInfo.loadOp = Utility::VoltToVulkanLoadOp(colorAtt.clearMode);
			newInfo.storeOp = VK_ATTACHMENT_STORE_OP_STORE;

			memcpy_s(&newInfo.clearValue, sizeof(uint32_t) * 4, &colorAtt.clearColor, sizeof(uint32_t) * 4);
		}

		const bool hasDepth = renderingInfo.depthAttachmentInfo.view;

		if (hasDepth)
		{
			depthAttachmentInfo.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO;
			depthAttachmentInfo.imageView = renderingInfo.depthAttachmentInfo.view->GetHandle<VkImageView>();
			depthAttachmentInfo.imageLayout = VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL;
			depthAttachmentInfo.loadOp = Utility::VoltToVulkanLoadOp(renderingInfo.depthAttachmentInfo.clearMode);
			depthAttachmentInfo.storeOp = VK_ATTACHMENT_STORE_OP_STORE;

			memcpy_s(&depthAttachmentInfo.clearValue.depthStencil, sizeof(uint32_t) * 2, &renderingInfo.depthAttachmentInfo.clearColor, sizeof(uint32_t) * 2);
		}

		VkRenderingInfo vkRenderingInfo{};
		vkRenderingInfo.sType = VK_STRUCTURE_TYPE_RENDERING_INFO;
		vkRenderingInfo.renderArea = { renderingInfo.renderArea.offset.x, renderingInfo.renderArea.offset.y, renderingInfo.renderArea.extent.width, renderingInfo.renderArea.extent.height };
		vkRenderingInfo.layerCount = renderingInfo.layerCount;
		vkRenderingInfo.colorAttachmentCount = static_cast<uint32_t>(colorAttachmentInfo.Size());
		vkRenderingInfo.pColorAttachments = colorAttachmentInfo.Data();
		vkRenderingInfo.pStencilAttachment = nullptr;

		if (hasDepth)
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
		VT_PROFILE_FUNCTION();

#ifndef VT_DIST
		if (!m_currentRenderPipeline && !m_currentComputePipeline)
		{
			RHILog::LogTagged(LogSeverity::Error, "[VulkanCommandBuffer]", "Unable to push constants as no pipeline is currently bound!");
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

#ifdef VT_ENABLE_COMMAND_BUFFER_VALIDATION
		if (stageFlags == 0)
		{
			return;
		}
#endif

		vkCmdPushConstants(m_commandBuffers.at(index).commandBuffer, pipelineLayout, stageFlags, offset, size, data);
	}

	void AddGlobalBarrier(const ResourceBarrierInfo& barrierInfo, VkMemoryBarrier2& outBarrier)
	{
		VT_PROFILE_FUNCTION();

		outBarrier.sType = VK_STRUCTURE_TYPE_MEMORY_BARRIER_2;
		outBarrier.pNext = nullptr;
		outBarrier.srcAccessMask = Utility::GetAccessFromBarrierAccess(barrierInfo.globalBarrier().srcAccess);
		outBarrier.dstAccessMask = Utility::GetAccessFromBarrierAccess(barrierInfo.globalBarrier().dstAccess);
		outBarrier.srcStageMask = Utility::GetStageFromBarrierSync(barrierInfo.globalBarrier().srcStage);
		outBarrier.dstStageMask = Utility::GetStageFromBarrierSync(barrierInfo.globalBarrier().dstStage);
	}

	void AddBufferBarrier(const ResourceBarrierInfo& barrierInfo, VkBufferMemoryBarrier2& outBarrier)
	{
		VT_PROFILE_FUNCTION();

		auto& vkBuffer = barrierInfo.bufferBarrier().resource->AsRef<VulkanStorageBuffer>();

		outBarrier.sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER_2;
		outBarrier.pNext = nullptr;
		outBarrier.srcAccessMask = Utility::GetAccessFromBarrierAccess(barrierInfo.bufferBarrier().srcAccess);
		outBarrier.dstAccessMask = Utility::GetAccessFromBarrierAccess(barrierInfo.bufferBarrier().dstAccess);
		outBarrier.srcStageMask = Utility::GetStageFromBarrierSync(barrierInfo.bufferBarrier().srcStage);
		outBarrier.dstStageMask = Utility::GetStageFromBarrierSync(barrierInfo.bufferBarrier().dstStage);
		outBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		outBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		outBarrier.offset = barrierInfo.bufferBarrier().offset;
		outBarrier.size = barrierInfo.bufferBarrier().size;
		outBarrier.buffer = vkBuffer.GetHandle<VkBuffer>();
	}

	void AddImageBarrier(const ResourceBarrierInfo& barrierInfo, VkImageMemoryBarrier2& outBarrier)
	{
		VT_PROFILE_FUNCTION();

		auto& vkImage = barrierInfo.imageBarrier().resource->AsRef<VulkanImage2D>();
		
		outBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2;
		outBarrier.pNext = nullptr;
		outBarrier.srcAccessMask = Utility::GetAccessFromBarrierAccess(barrierInfo.imageBarrier().srcAccess);
		outBarrier.dstAccessMask = Utility::GetAccessFromBarrierAccess(barrierInfo.imageBarrier().dstAccess);
		outBarrier.srcStageMask = Utility::GetStageFromBarrierSync(barrierInfo.imageBarrier().srcStage);
		outBarrier.dstStageMask = Utility::GetStageFromBarrierSync(barrierInfo.imageBarrier().dstStage);
		outBarrier.oldLayout = Utility::GetVkImageLayoutFromImageLayout(barrierInfo.imageBarrier().srcLayout);
		outBarrier.newLayout = Utility::GetVkImageLayoutFromImageLayout(barrierInfo.imageBarrier().dstLayout);
		outBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		outBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		outBarrier.subresourceRange.aspectMask = static_cast<VkImageAspectFlags>(vkImage.GetImageAspect());
		outBarrier.subresourceRange.baseArrayLayer = barrierInfo.imageBarrier().subResource.baseArrayLayer;
		outBarrier.subresourceRange.baseMipLevel = barrierInfo.imageBarrier().subResource.baseMipLevel;
		outBarrier.subresourceRange.layerCount = barrierInfo.imageBarrier().subResource.layerCount == std::numeric_limits<uint32_t>::max() ? VK_REMAINING_ARRAY_LAYERS : barrierInfo.imageBarrier().subResource.layerCount;
		outBarrier.subresourceRange.levelCount = barrierInfo.imageBarrier().subResource.levelCount == std::numeric_limits<uint32_t>::max() ? VK_REMAINING_MIP_LEVELS : barrierInfo.imageBarrier().subResource.levelCount;
		outBarrier.image = vkImage.GetHandle<VkImage>();

		vkImage.SetCurrentLayout(static_cast<VulkanImage2D::ImageLayoutInt>(outBarrier.newLayout));
	}

	void VulkanCommandBuffer::ResourceBarrier(const std::vector<ResourceBarrierInfo>& resourceBarriers)
	{
		VT_PROFILE_FUNCTION();

		std::vector<VkImageMemoryBarrier2> imageBarriers{};
		std::vector<VkBufferMemoryBarrier2> bufferBarriers{};
		std::vector<VkMemoryBarrier2> memoryBarriers{};

		for (const auto& resourceBarrier : resourceBarriers)
		{
			switch (resourceBarrier.type)
			{
				case BarrierType::Global:
					AddGlobalBarrier(resourceBarrier, memoryBarriers.emplace_back());
					break;

				case BarrierType::Buffer:
					AddBufferBarrier(resourceBarrier, bufferBarriers.emplace_back());
					break;

				case BarrierType::Image:
					AddImageBarrier(resourceBarrier, imageBarriers.emplace_back());
					break;
			}
		}

		VkDependencyInfo info{};
		info.sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO;
		info.pNext = nullptr;
		info.dependencyFlags = 0;
		info.memoryBarrierCount = static_cast<uint32_t>(memoryBarriers.size());
		info.pMemoryBarriers = memoryBarriers.data();
		info.bufferMemoryBarrierCount = static_cast<uint32_t>(bufferBarriers.size());
		info.pBufferMemoryBarriers = bufferBarriers.data();
		info.imageMemoryBarrierCount = static_cast<uint32_t>(imageBarriers.size());
		info.pImageMemoryBarriers = imageBarriers.data();

		const uint32_t index = GetCurrentCommandBufferIndex();
		VT_PROFILE_GPU_CONTEXT(m_commandBuffers.at(index).commandBuffer);

		vkCmdPipelineBarrier2(m_commandBuffers.at(index).commandBuffer, &info);
	}

	void VulkanCommandBuffer::BeginMarker(std::string_view markerLabel, const std::array<float, 4>& markerColor)
	{
		VT_PROFILE_FUNCTION();

		if (Volt::RHI::vkCmdBeginDebugUtilsLabelEXT)
		{
			VkDebugUtilsLabelEXT markerInfo{};
			markerInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_LABEL_EXT;
			markerInfo.pLabelName = markerLabel.data();
			markerInfo.color[0] = markerColor[0];
			markerInfo.color[1] = markerColor[1];
			markerInfo.color[2] = markerColor[2];
			markerInfo.color[3] = markerColor[3];

			const uint32_t index = GetCurrentCommandBufferIndex();
			Volt::RHI::vkCmdBeginDebugUtilsLabelEXT(m_commandBuffers.at(index).commandBuffer, &markerInfo);
		}
	}

	void VulkanCommandBuffer::EndMarker()
	{
		VT_PROFILE_FUNCTION();

		if (Volt::RHI::vkCmdEndDebugUtilsLabelEXT)
		{
			const uint32_t index = GetCurrentCommandBufferIndex();
			Volt::RHI::vkCmdEndDebugUtilsLabelEXT(m_commandBuffers.at(index).commandBuffer);
		}
	}

	const uint32_t VulkanCommandBuffer::BeginTimestamp()
	{
		VT_PROFILE_FUNCTION();

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
		VT_PROFILE_FUNCTION();

		if (!m_hasTimestampSupport)
		{
			return;
		}

		const uint32_t index = GetCurrentCommandBufferIndex();
		vkCmdWriteTimestamp2(m_commandBuffers.at(index).commandBuffer, VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT, m_timestampQueryPools.at(index), timestampIndex + 1);
	}

	const float VulkanCommandBuffer::GetExecutionTime(uint32_t timestampIndex) const
	{
		VT_PROFILE_FUNCTION();

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

	void VulkanCommandBuffer::ClearImage(WeakPtr<Image2D> image, std::array<float, 4> clearColor)
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

		if (imageAspect & VK_IMAGE_ASPECT_COLOR_BIT)
		{
			VkClearColorValue vkClearColor{};
			vkClearColor.float32[0] = clearColor[0];
			vkClearColor.float32[1] = clearColor[1];
			vkClearColor.float32[2] = clearColor[2];
			vkClearColor.float32[3] = clearColor[3];

			vkCmdClearColorImage(m_commandBuffers.at(index).commandBuffer, image->GetHandle<VkImage>(), static_cast<VkImageLayout>(vkImage.GetCurrentLayout()), &vkClearColor, 1, &range);
		}
		else
		{
			VkClearDepthStencilValue vkClearColor{};
			vkClearColor.depth = clearColor[0];
			vkClearColor.stencil = static_cast<uint32_t>(clearColor[1]);

			vkCmdClearDepthStencilImage(m_commandBuffers.at(index).commandBuffer, image->GetHandle<VkImage>(), static_cast<VkImageLayout>(vkImage.GetCurrentLayout()), &vkClearColor, 1, &range);
		}
	}

	void VulkanCommandBuffer::ClearBuffer(WeakPtr<StorageBuffer> buffer, const uint32_t value)
	{
		VulkanStorageBuffer& vkBuffer = buffer->AsRef<VulkanStorageBuffer>();
		const uint32_t index = GetCurrentCommandBufferIndex();
		vkCmdFillBuffer(m_commandBuffers.at(index).commandBuffer, vkBuffer.GetHandle<VkBuffer>(), 0, vkBuffer.GetByteSize(), value);
	}

	void VulkanCommandBuffer::UpdateBuffer(WeakPtr<StorageBuffer> dstBuffer, const size_t dstOffset, const size_t dataSize, const void* data)
	{
		assert(dataSize <= 65536 && "Size must not exceed MAX_UPDATE_SIZE!");
		
		VulkanStorageBuffer& vkBuffer = dstBuffer->AsRef<VulkanStorageBuffer>();
		const uint32_t index = GetCurrentCommandBufferIndex();
		vkCmdUpdateBuffer(m_commandBuffers.at(index).commandBuffer, vkBuffer.GetHandle<VkBuffer>(), dstOffset, dataSize, data);
	}

	void VulkanCommandBuffer::CopyBufferRegion(WeakPtr<Allocation> srcResource, const size_t srcOffset, WeakPtr<Allocation> dstResource, const size_t dstOffset, const size_t size)
	{
		const uint32_t index = GetCurrentCommandBufferIndex();

		VkBufferCopy copy{};
		copy.srcOffset = srcOffset;
		copy.dstOffset = dstOffset;
		copy.size = size;

		vkCmdCopyBuffer(m_commandBuffers.at(index).commandBuffer, srcResource->GetResourceHandle<VkBuffer>(), dstResource->GetResourceHandle<VkBuffer>(), 1, &copy);
	}

	void VulkanCommandBuffer::CopyBufferToImage(WeakPtr<Allocation> srcBuffer, WeakPtr<Image2D> dstImage, const uint32_t width, const uint32_t height, const uint32_t mip)
	{
		const uint32_t index = GetCurrentCommandBufferIndex();
		auto& vkImage = dstImage->AsRef<VulkanImage2D>();

		VkBufferImageCopy region{};
		region.bufferOffset = 0;
		region.bufferRowLength = 0;
		region.bufferImageHeight = 0;

		region.imageSubresource.aspectMask = static_cast<VkImageAspectFlags>(vkImage.GetImageAspect());
		region.imageSubresource.mipLevel = mip;
		region.imageSubresource.baseArrayLayer = 0;
		region.imageSubresource.layerCount = 1;

		region.imageOffset = { 0, 0, 0 };
		region.imageExtent = { width, height, 1 };

		vkCmdCopyBufferToImage(m_commandBuffers.at(index).commandBuffer, srcBuffer->GetResourceHandle<VkBuffer>(), dstImage->GetHandle<VkImage>(), static_cast<VkImageLayout>(vkImage.GetCurrentLayout()), 1, &region);
	}

	void VulkanCommandBuffer::CopyImageToBuffer(WeakPtr<Image2D> srcImage, WeakPtr<Allocation> dstBuffer, const size_t dstOffset, const uint32_t width, const uint32_t height, const uint32_t mip)
	{
		const uint32_t index = GetCurrentCommandBufferIndex();
		auto& vkImage = srcImage->AsRef<VulkanImage2D>();

		VkBufferImageCopy region{};
		region.bufferOffset = dstOffset;
		region.bufferRowLength = 0;
		region.bufferImageHeight = 0;

		region.imageSubresource.aspectMask = static_cast<VkImageAspectFlags>(vkImage.GetImageAspect());
		region.imageSubresource.mipLevel = mip;
		region.imageSubresource.baseArrayLayer = 0;
		region.imageSubresource.layerCount = 1;

		region.imageOffset = { 0, 0, 0 };
		region.imageExtent = { width, height, 1 };

		vkCmdCopyImageToBuffer(m_commandBuffers.at(index).commandBuffer, srcImage->GetHandle<VkImage>(), static_cast<VkImageLayout>(vkImage.GetCurrentLayout()), dstBuffer->GetResourceHandle<VkBuffer>(), 1, &region);
	}

	void VulkanCommandBuffer::CopyImage(WeakPtr<Image2D> srcImage, WeakPtr<Image2D> dstImage, const uint32_t width, const uint32_t height)
	{
		VulkanImage2D& srcVkImage = srcImage->AsRef<VulkanImage2D>();
		VulkanImage2D& dstVkImage = dstImage->AsRef<VulkanImage2D>();

		const VkImageAspectFlags srcImageAspect = static_cast<VkImageAspectFlags>(srcVkImage.GetImageAspect());
		const VkImageAspectFlags dstImageAspect = static_cast<VkImageAspectFlags>(dstVkImage.GetImageAspect());
		const uint32_t index = GetCurrentCommandBufferIndex();

		VkImageSubresourceRange srcRange{};
		srcRange.aspectMask = srcImageAspect;
		srcRange.baseArrayLayer = 0;
		srcRange.baseMipLevel = 0;
		srcRange.layerCount = VK_REMAINING_ARRAY_LAYERS;
		srcRange.levelCount = VK_REMAINING_MIP_LEVELS;

		VkImageSubresourceRange dstRange{};
		srcRange.aspectMask = dstImageAspect;
		srcRange.baseArrayLayer = 0;
		srcRange.baseMipLevel = 0;
		srcRange.layerCount = VK_REMAINING_ARRAY_LAYERS;
		srcRange.levelCount = VK_REMAINING_MIP_LEVELS;

		VkImageCopy2 info{};
		info.sType = VK_STRUCTURE_TYPE_IMAGE_COPY_2;
		info.pNext = nullptr;
		info.srcSubresource.aspectMask = srcImageAspect;
		info.srcSubresource.baseArrayLayer = 0;
		info.srcSubresource.layerCount = VK_REMAINING_ARRAY_LAYERS;
		info.srcSubresource.mipLevel = 0;
		info.dstSubresource.aspectMask = dstImageAspect;
		info.dstSubresource.baseArrayLayer = 0;
		info.dstSubresource.layerCount = VK_REMAINING_ARRAY_LAYERS;
		info.dstSubresource.mipLevel = 0;
		info.srcOffset.x = 0;
		info.srcOffset.y = 0;
		info.srcOffset.z = 0;
		info.dstOffset.x = 0;
		info.dstOffset.y = 0;
		info.dstOffset.z = 0;
		info.extent.width = width;
		info.extent.height = height;
		info.extent.depth = 1;

		VkCopyImageInfo2 cpyInfo{};
		cpyInfo.sType = VK_STRUCTURE_TYPE_COPY_IMAGE_INFO_2;
		cpyInfo.pNext = nullptr;
		cpyInfo.srcImage = srcVkImage.GetHandle<VkImage>();
		cpyInfo.srcImageLayout = static_cast<VkImageLayout>(srcVkImage.GetCurrentLayout());
		cpyInfo.dstImage = dstVkImage.GetHandle<VkImage>();
		cpyInfo.dstImageLayout = static_cast<VkImageLayout>(dstVkImage.GetCurrentLayout());
		cpyInfo.pRegions = &info;
		cpyInfo.regionCount = 1;

		vkCmdCopyImage2(m_commandBuffers.at(index).commandBuffer, &cpyInfo);
	}

	const uint32_t VulkanCommandBuffer::GetCurrentIndex() const
	{
		return m_currentCommandBufferIndex;
	}

	const QueueType VulkanCommandBuffer::GetQueueType() const
	{
		return m_queueType;
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
			auto& swapchain = m_swapchainTarget->AsRef<VulkanSwapchain>();
			m_commandBufferCount = swapchain.GetFramesInFlight();
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

		m_hasTimestampSupport = GraphicsContext::GetPhysicalDevice()->AsRef<VulkanPhysicalGraphicsDevice>().GetProperties().hasTimestampSupport;
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
				fences.push_back(m_swapchainTarget->AsRef<VulkanSwapchain>().GetFence(i));
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

			const float nsTime = endTime > startTime ? (endTime - startTime) * GraphicsContext::GetPhysicalDevice()->AsRef<VulkanPhysicalGraphicsDevice>().GetProperties().timestampPeriod : 0.f;
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
