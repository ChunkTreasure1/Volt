#include "vkpch.h"
#include "VulkanRHIModule/Buffers/VulkanCommandBuffer.h"

#include "VulkanRHIModule/Common/VulkanCommon.h"
#include "VulkanRHIModule/Common/VulkanHelpers.h"
#include "VulkanRHIModule/Common/VulkanFunctions.h"

#include "VulkanRHIModule/Graphics/VulkanPhysicalGraphicsDevice.h"
#include "VulkanRHIModule/Graphics/VulkanSwapchain.h"

#include "VulkanRHIModule/Pipelines/VulkanRenderPipeline.h"
#include "VulkanRHIModule/Pipelines/VulkanComputePipeline.h"

#include "VulkanRHIModule/Descriptors/VulkanDescriptorTable.h"
#include "VulkanRHIModule/Descriptors/VulkanBindlessDescriptorTable.h"
#include "VulkanRHIModule/Descriptors/VulkanDescriptorBufferTable.h"

#include "VulkanRHIModule/Images/VulkanImage.h"

#include "VulkanRHIModule/Buffers/VulkanStorageBuffer.h"
#include "VulkanRHIModule/Synchronization/VulkanSemaphore.h"

#include "VulkanRHIModule/Synchronization/VulkanEvent.h"

#include <RHIModule/Graphics/GraphicsContext.h>
#include <RHIModule/Graphics/GraphicsDevice.h>
#include <RHIModule/Graphics/DeviceQueue.h>

#include <RHIModule/Memory/Allocation.h>

#include <RHIModule/Buffers/IndexBuffer.h>
#include <RHIModule/Buffers/VertexBuffer.h>

#include <RHIModule/Images/ImageView.h>

#include <RHIModule/Shader/Shader.h>
#include <RHIModule/Core/Profiling.h>
#include <RHIModule/RHIProxy.h>
#include <RHIModule/Synchronization/Fence.h>

#include <CoreUtilities/EnumUtils.h>

#ifdef VT_ENABLE_NV_AFTERMATH

#include <GFSDK_Aftermath.h>
#include <GFSDK_Aftermath_Defines.h>
#include <GFSDK_Aftermath_GpuCrashDump.h>

#include <RHIModule/Utility/NsightAftermathHelpers.h>

#endif

#include <vulkan/vulkan.h>

namespace Volt::RHI
{
	namespace Utility
	{
		const VkPipelineStageFlags2 GetStageFromBarrierStage(const BarrierStage barrierStage)
		{
			VkPipelineStageFlags2 result = VK_PIPELINE_STAGE_2_NONE;

			if (EnumValueContainsFlag(barrierStage, BarrierStage::All))
			{
				result |= VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT;
			}

			if (EnumValueContainsFlag(barrierStage, BarrierStage::IndexInput))
			{
				result |= VK_PIPELINE_STAGE_2_INDEX_INPUT_BIT;
			}

			if (EnumValueContainsFlag(barrierStage, BarrierStage::VertexInput))
			{
				result |= VK_PIPELINE_STAGE_2_VERTEX_ATTRIBUTE_INPUT_BIT;
			}

			if (EnumValueContainsFlag(barrierStage, BarrierStage::VertexShader))
			{
				result |= VK_PIPELINE_STAGE_2_VERTEX_SHADER_BIT;
			}

			if (EnumValueContainsFlag(barrierStage, BarrierStage::PixelShader))
			{
				result |= VK_PIPELINE_STAGE_2_FRAGMENT_SHADER_BIT;
			}

			if (EnumValueContainsFlag(barrierStage, BarrierStage::DepthStencil))
			{
				result |= VK_PIPELINE_STAGE_2_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_2_LATE_FRAGMENT_TESTS_BIT;
			}

			if (EnumValueContainsFlag(barrierStage, BarrierStage::RenderTarget))
			{
				result |= VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT;
			}

			if (EnumValueContainsFlag(barrierStage, BarrierStage::ComputeShader))
			{
				result |= VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT;
			}

			if (EnumValueContainsFlag(barrierStage, BarrierStage::RayTracing))
			{
				result |= VK_PIPELINE_STAGE_2_RAY_TRACING_SHADER_BIT_KHR;
			}

			if (EnumValueContainsFlag(barrierStage, BarrierStage::Copy))
			{
				result |= VK_PIPELINE_STAGE_2_COPY_BIT;
			}

			if (EnumValueContainsFlag(barrierStage, BarrierStage::Resolve))
			{
				result |= VK_PIPELINE_STAGE_2_RESOLVE_BIT;
			}

			if (EnumValueContainsFlag(barrierStage, BarrierStage::DrawIndirect))
			{
				result |= VK_PIPELINE_STAGE_2_DRAW_INDIRECT_BIT;
			}

			if (EnumValueContainsFlag(barrierStage, BarrierStage::AllGraphics))
			{
				result |= VK_PIPELINE_STAGE_2_ALL_GRAPHICS_BIT;
			}

			if (EnumValueContainsFlag(barrierStage, BarrierStage::VideoDecode))
			{
				result |= VK_PIPELINE_STAGE_2_VIDEO_DECODE_BIT_KHR;
			}

			if (EnumValueContainsFlag(barrierStage, BarrierStage::VideoEncode))
			{
				result |= VK_PIPELINE_STAGE_2_VIDEO_ENCODE_BIT_KHR;
			}

			if (EnumValueContainsFlag(barrierStage, BarrierStage::MeshShader))
			{
				result |= VK_PIPELINE_STAGE_2_MESH_SHADER_BIT_EXT;
			}

			if (EnumValueContainsFlag(barrierStage, BarrierStage::AmplificationShader))
			{
				result |= VK_PIPELINE_STAGE_2_TASK_SHADER_BIT_EXT;
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

			if (EnumValueContainsFlag(barrierAccess, BarrierAccess::ShaderRead))
			{
				result |= VK_ACCESS_2_SHADER_READ_BIT;
			}

			if (EnumValueContainsFlag(barrierAccess, BarrierAccess::IndirectArgument))
			{
				result |= VK_ACCESS_2_INDIRECT_COMMAND_READ_BIT;
			}

			if (EnumValueContainsFlag(barrierAccess, BarrierAccess::CopyDest))
			{
				result |= VK_ACCESS_2_TRANSFER_WRITE_BIT;
			}

			if (EnumValueContainsFlag(barrierAccess, BarrierAccess::CopySource))
			{
				result |= VK_ACCESS_2_TRANSFER_READ_BIT;
			}

			if (EnumValueContainsFlag(barrierAccess, BarrierAccess::ResolveDest))
			{
				result |= VK_ACCESS_2_TRANSFER_WRITE_BIT;
			}

			if (EnumValueContainsFlag(barrierAccess, BarrierAccess::ResolveSource))
			{
				result |= VK_ACCESS_2_TRANSFER_READ_BIT;
			}

			if (EnumValueContainsFlag(barrierAccess, BarrierAccess::VideoEncodeRead))
			{
				result |= VK_ACCESS_2_VIDEO_ENCODE_READ_BIT_KHR;
			}

			if (EnumValueContainsFlag(barrierAccess, BarrierAccess::VideoEncodeWrite))
			{
				result |= VK_ACCESS_2_VIDEO_ENCODE_WRITE_BIT_KHR;
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

		inline uint64_t CalculateStagingBufferSize(const ImageCopyData& copyData)
		{
			uint64_t size = 0;
			for (const auto& data : copyData.copySubData)
			{
				size += data.slicePitch;
			}

			return size;
		}
	}

	VulkanCommandBuffer::VulkanCommandBuffer(QueueType queueType)
		: m_queueType(queueType)
	{
		Invalidate();
	}

	VulkanCommandBuffer::VulkanCommandBuffer(const CommandBuffer* parentCommandBuffer)
		: m_queueType(parentCommandBuffer->GetQueueType()), m_commandBufferLevel(CommandBufferLevel::Secondary), m_parentCommandBuffer(parentCommandBuffer)
	{
		Invalidate();
	}

	VulkanCommandBuffer::~VulkanCommandBuffer()
	{
		Release(m_fence);
		m_fence = nullptr;
	}

	void VulkanCommandBuffer::Begin()
	{
		VT_PROFILE_FUNCTION();

		auto device = GraphicsContext::GetDevice();

		m_fence->WaitUntilSignaled();
		m_fence->Reset();

		VT_VK_CHECK(vkResetCommandPool(device->GetHandle<VkDevice>(), m_commandBufferData.commandPool, 0));

		// If the next timestamp query is zero, no frame has run before
		if (m_nextAvailableTimestampQuery > 0)
		{
			FetchTimestampResults();
		}

		// Begin command buffer
		{
			if (m_commandBufferLevel == CommandBufferLevel::Primary) BeginPrimaryInternal();
			else													 BeginSecondaryInternal();
		}

		if (m_hasTimestampSupport)
		{
			vkCmdResetQueryPool(m_commandBufferData.commandBuffer, m_timestampQueryPool, 0, m_timestampQueryCount);
			vkCmdWriteTimestamp2(m_commandBufferData.commandBuffer, VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT, m_timestampQueryPool, 0);

			m_timestampCount = m_nextAvailableTimestampQuery;
			m_nextAvailableTimestampQuery = 2;
		}

		if (m_commandBufferLevel == CommandBufferLevel::Primary)
		{
			BeginMarker("CommandBuffer", { 1.f, 1.f, 1.f, 1.f });
		}
	}

	void VulkanCommandBuffer::End()
	{
		VT_PROFILE_FUNCTION();

		if (m_commandBufferLevel == CommandBufferLevel::Primary)
		{
			EndMarker();
		}

		if (m_hasTimestampSupport)
		{
			vkCmdWriteTimestamp2(m_commandBufferData.commandBuffer, VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT, m_timestampQueryPool, 1);
		}

		VT_VK_CHECK(vkEndCommandBuffer(m_commandBufferData.commandBuffer));
	}

	void VulkanCommandBuffer::Execute()
	{
		VT_PROFILE_FUNCTION();
		VT_ENSURE(m_commandBufferLevel == CommandBufferLevel::Primary);

		auto device = GraphicsContext::GetDevice();
		DeviceQueueExecuteInfo execInfo{};
		execInfo.commandBuffers = { this };
		execInfo.fence = m_fence;

		device->GetDeviceQueue(m_queueType)->Execute(execInfo);
	}

	void VulkanCommandBuffer::Flush(RefPtr<Fence> fence)
	{
		VT_PROFILE_FUNCTION();
		VT_ENSURE(m_commandBufferLevel == CommandBufferLevel::Primary);

		// End the current command buffer
		End();

		// Execute current command buffer and use the supplied fence
		{
			auto device = GraphicsContext::GetDevice();

			DeviceQueueExecuteInfo execInfo{};
			execInfo.commandBuffers = { this };
			execInfo.fence = fence;
			device->GetDeviceQueue(m_queueType)->Execute(execInfo);
		}

		// Now we destroy the current command buffer and create a new one.
		// #TODO_Ivar: Investigate the overhead of creating a new command buffer every frame.
		Release(fence);
		Invalidate();

		// And lastly we restart the command buffer
		Begin();
	}

	void VulkanCommandBuffer::ExecuteAndWait()
	{
		VT_PROFILE_FUNCTION();
		VT_ENSURE(m_commandBufferLevel == CommandBufferLevel::Primary);

		auto device = GraphicsContext::GetDevice();

		DeviceQueueExecuteInfo execInfo{};
		execInfo.commandBuffers = { this };
		execInfo.fence = m_fence;

		device->GetDeviceQueue(m_queueType)->Execute(execInfo);
		m_fence->WaitUntilSignaled();

		FetchTimestampResults();
	}

	void VulkanCommandBuffer::WaitForFence()
	{
		VT_PROFILE_FUNCTION();
		m_fence->WaitUntilSignaled();
	}

	void VulkanCommandBuffer::SetEvent(WeakPtr<Event> event)
	{
		VT_PROFILE_FUNCTION();

		VkDependencyInfo depInfo{};
		depInfo.sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO;
		depInfo.pNext = nullptr;
		depInfo.dependencyFlags = 0;
		//depInfo.

		//vkCmdSetEvent2()
	}

	void VulkanCommandBuffer::Draw(const uint32_t vertexCount, const uint32_t instanceCount, const uint32_t firstVertex, const uint32_t firstInstance)
	{
		VT_PROFILE_FUNCTION();

#ifdef VT_ENABLE_COMMAND_BUFFER_VALIDATION
		VT_ENSURE(m_currentRenderPipeline != nullptr);
#endif

		vkCmdDraw(m_commandBufferData.commandBuffer, vertexCount, instanceCount, firstVertex, firstInstance);
	}

	void VulkanCommandBuffer::DrawIndexed(const uint32_t indexCount, const uint32_t instanceCount, const uint32_t firstIndex, const uint32_t vertexOffset, const uint32_t firstInstance)
	{
		VT_PROFILE_FUNCTION();

#ifdef VT_ENABLE_COMMAND_BUFFER_VALIDATION
		VT_ENSURE(m_currentRenderPipeline != nullptr);
#endif

		vkCmdDrawIndexed(m_commandBufferData.commandBuffer, indexCount, instanceCount, firstIndex, vertexOffset, firstInstance);
	}

	void VulkanCommandBuffer::DrawIndexedIndirect(WeakPtr<StorageBuffer> commandsBuffer, const size_t offset, const uint32_t drawCount, const uint32_t stride)
	{
		VT_PROFILE_FUNCTION();

#ifdef VT_ENABLE_COMMAND_BUFFER_VALIDATION
		VT_ENSURE(m_currentRenderPipeline != nullptr);
#endif

		vkCmdDrawIndexedIndirect(m_commandBufferData.commandBuffer, commandsBuffer->GetHandle<VkBuffer>(), offset, drawCount, stride);
	}

	void VulkanCommandBuffer::DrawIndirect(WeakPtr<StorageBuffer> commandsBuffer, const size_t offset, const uint32_t drawCount, const uint32_t stride)
	{
		VT_PROFILE_FUNCTION();

#ifdef VT_ENABLE_COMMAND_BUFFER_VALIDATION
		VT_ENSURE(m_currentRenderPipeline != nullptr);
#endif

		vkCmdDrawIndirect(m_commandBufferData.commandBuffer, commandsBuffer->GetHandle<VkBuffer>(), offset, drawCount, stride);
	}

	void VulkanCommandBuffer::DrawIndexedIndirectCount(WeakPtr<StorageBuffer> commandsBuffer, const size_t offset, WeakPtr<StorageBuffer> countBuffer, const size_t countBufferOffset, const uint32_t maxDrawCount, const uint32_t stride)
	{
		VT_PROFILE_FUNCTION();

#ifdef VT_ENABLE_COMMAND_BUFFER_VALIDATION
		VT_ENSURE(m_currentRenderPipeline != nullptr);
#endif

		vkCmdDrawIndexedIndirectCount(m_commandBufferData.commandBuffer, commandsBuffer->GetHandle<VkBuffer>(), offset, countBuffer->GetHandle<VkBuffer>(), countBufferOffset, maxDrawCount, stride);
	}

	void VulkanCommandBuffer::DrawIndirectCount(WeakPtr<StorageBuffer> commandsBuffer, const size_t offset, WeakPtr<StorageBuffer> countBuffer, const size_t countBufferOffset, const uint32_t maxDrawCount, const uint32_t stride)
	{
		VT_PROFILE_FUNCTION();

#ifdef VT_ENABLE_COMMAND_BUFFER_VALIDATION
		VT_ENSURE(m_currentRenderPipeline != nullptr);
#endif

		vkCmdDrawIndirectCount(m_commandBufferData.commandBuffer, commandsBuffer->GetHandle<VkBuffer>(), offset, countBuffer->GetHandle<VkBuffer>(), countBufferOffset, maxDrawCount, stride);
	}

	void VulkanCommandBuffer::Dispatch(const uint32_t groupCountX, const uint32_t groupCountY, const uint32_t groupCountZ)
	{
		VT_PROFILE_FUNCTION();

#ifdef VT_ENABLE_COMMAND_BUFFER_VALIDATION
		VT_ENSURE(m_currentComputePipeline != nullptr);
#endif

		vkCmdDispatch(m_commandBufferData.commandBuffer, groupCountX, groupCountY, groupCountZ);
	}

	void VulkanCommandBuffer::DispatchIndirect(WeakPtr<StorageBuffer> commandsBuffer, const size_t offset)
	{
		VT_PROFILE_FUNCTION();

#ifdef VT_ENABLE_COMMAND_BUFFER_VALIDATION
		VT_ENSURE(m_currentComputePipeline != nullptr);
#endif

		vkCmdDispatchIndirect(m_commandBufferData.commandBuffer, commandsBuffer->GetHandle<VkBuffer>(), offset);
	}

	void VulkanCommandBuffer::DispatchMeshTasks(const uint32_t groupCountX, const uint32_t groupCountY, const uint32_t groupCountZ)
	{
		VT_PROFILE_FUNCTION();

#ifdef VT_ENABLE_COMMAND_BUFFER_VALIDATION
		VT_ENSURE(m_currentRenderPipeline != nullptr);
#endif

		vkCmdDrawMeshTasksEXT(m_commandBufferData.commandBuffer, groupCountX, groupCountY, groupCountZ);
	}

	void VulkanCommandBuffer::DispatchMeshTasksIndirect(WeakPtr<StorageBuffer> commandsBuffer, const size_t offset, const uint32_t drawCount, const uint32_t stride)
	{
		VT_PROFILE_FUNCTION();

#ifdef VT_ENABLE_COMMAND_BUFFER_VALIDATION
		VT_ENSURE(m_currentRenderPipeline != nullptr);
#endif

		vkCmdDrawMeshTasksIndirectEXT(m_commandBufferData.commandBuffer, commandsBuffer->GetHandle<VkBuffer>(), offset, drawCount, stride);
	}

	void VulkanCommandBuffer::DispatchMeshTasksIndirectCount(WeakPtr<StorageBuffer> commandsBuffer, const size_t offset, WeakPtr<StorageBuffer> countBuffer, const size_t countBufferOffset, const uint32_t maxDrawCount, const uint32_t stride)
	{
		VT_PROFILE_FUNCTION();

#ifdef VT_ENABLE_COMMAND_BUFFER_VALIDATION
		VT_ENSURE(m_currentRenderPipeline != nullptr);
#endif

		vkCmdDrawMeshTasksIndirectCountEXT(m_commandBufferData.commandBuffer, commandsBuffer->GetHandle<VkBuffer>(), offset, countBuffer->GetHandle<VkBuffer>(), countBufferOffset, maxDrawCount, stride);
	}

	void VulkanCommandBuffer::SetViewports(const StackVector<Viewport, MAX_VIEWPORT_COUNT>& viewports)
	{
		VT_PROFILE_FUNCTION();

		vkCmdSetViewport(m_commandBufferData.commandBuffer, 0, static_cast<uint32_t>(viewports.Size()), reinterpret_cast<const VkViewport*>(viewports.Data()));
	}

	void VulkanCommandBuffer::SetScissors(const StackVector<Rect2D, MAX_VIEWPORT_COUNT>& scissors)
	{
		VT_PROFILE_FUNCTION();


		vkCmdSetScissor(m_commandBufferData.commandBuffer, 0, static_cast<uint32_t>(scissors.Size()), reinterpret_cast<const VkRect2D*>(scissors.Data()));
	}

	void VulkanCommandBuffer::BindPipeline(WeakPtr<RenderPipeline> pipeline)
	{
		VT_PROFILE_FUNCTION();

		m_currentComputePipeline.Reset();

		if (!pipeline)
		{
			m_currentRenderPipeline.Reset();
			return;
		}

		m_currentRenderPipeline = pipeline;
		vkCmdBindPipeline(m_commandBufferData.commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline->GetHandle<VkPipeline>());
	}

	void VulkanCommandBuffer::BindPipeline(WeakPtr<ComputePipeline> pipeline)
	{
		VT_PROFILE_FUNCTION();

		m_currentRenderPipeline.Reset();

		if (!pipeline)
		{
			m_currentComputePipeline.Reset();
			return;
		}

		m_currentComputePipeline = pipeline;
		vkCmdBindPipeline(m_commandBufferData.commandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, pipeline->GetHandle<VkPipeline>());
	}

	void VulkanCommandBuffer::BindVertexBuffers(const StackVector<WeakPtr<VertexBuffer>, MAX_VERTEX_BUFFER_COUNT>& vertexBuffers, const uint32_t firstBinding)
	{
		VT_PROFILE_FUNCTION();

		StackVector<VkBuffer, MAX_VERTEX_BUFFER_COUNT> vkBuffers;
		StackVector<VkDeviceSize, MAX_VERTEX_BUFFER_COUNT> offsets;

		for (size_t i = 0; i < vertexBuffers.Size(); i++)
		{
			vkBuffers.EmplaceBack() = vertexBuffers[i]->GetHandle<VkBuffer>();
			offsets.EmplaceBack(0u);
		}

		vkCmdBindVertexBuffers(m_commandBufferData.commandBuffer, firstBinding, static_cast<uint32_t>(vkBuffers.Size()), vkBuffers.Data(), offsets.Data());
	}

	void VulkanCommandBuffer::BindVertexBuffers(const StackVector<WeakPtr<StorageBuffer>, MAX_VERTEX_BUFFER_COUNT>& vertexBuffers, const uint32_t firstBinding)
	{
		VT_PROFILE_FUNCTION();

		StackVector<VkBuffer, MAX_VERTEX_BUFFER_COUNT> vkBuffers;
		StackVector<VkDeviceSize, MAX_VERTEX_BUFFER_COUNT> offsets;

		for (size_t i = 0; i < vertexBuffers.Size(); i++)
		{
			vkBuffers.EmplaceBack() = vertexBuffers[i]->GetHandle<VkBuffer>();
			offsets.EmplaceBack(0u);
		}

		vkCmdBindVertexBuffers(m_commandBufferData.commandBuffer, firstBinding, static_cast<uint32_t>(vkBuffers.Size()), vkBuffers.Data(), offsets.Data());
	}

	void VulkanCommandBuffer::BindIndexBuffer(WeakPtr<IndexBuffer> indexBuffer)
	{
		VT_PROFILE_FUNCTION();

		constexpr VkDeviceSize offset = 0;
		vkCmdBindIndexBuffer(m_commandBufferData.commandBuffer, indexBuffer->GetHandle<VkBuffer>(), offset, VK_INDEX_TYPE_UINT32);
	}

	void VulkanCommandBuffer::BindIndexBuffer(WeakPtr<StorageBuffer> indexBuffer)
	{
		VT_PROFILE_FUNCTION();

		constexpr VkDeviceSize offset = 0;
		vkCmdBindIndexBuffer(m_commandBufferData.commandBuffer, indexBuffer->GetHandle<VkBuffer>(), offset, VK_INDEX_TYPE_UINT32);
	}

	void VulkanCommandBuffer::BindDescriptorTable(WeakPtr<DescriptorTable> descriptorTable)
	{
		VT_PROFILE_FUNCTION();

		if (GraphicsContext::GetPhysicalDevice()->AsRef<VulkanPhysicalGraphicsDevice>().AreDescriptorBuffersEnabled())
		{
			descriptorTable->AsRef<VulkanDescriptorBufferTable>().Bind(*this);
		}
		else
		{
			descriptorTable->AsRef<VulkanDescriptorTable>().Bind(*this);
		}
	}

	void VulkanCommandBuffer::BindDescriptorTable(WeakPtr<BindlessDescriptorTable> descriptorTable, WeakPtr<UniformBuffer> constantsBuffer, const uint32_t offsetIndex, const uint32_t stride)
	{
		VT_PROFILE_FUNCTION();
		descriptorTable->AsRef<VulkanBindlessDescriptorTable>().Bind(*this, constantsBuffer, offsetIndex, stride);
	}

	void VulkanCommandBuffer::BeginRendering(const RenderingInfo& renderingInfo)
	{
		VT_PROFILE_FUNCTION();

		StackVector<VkRenderingAttachmentInfo, MAX_COLOR_ATTACHMENT_COUNT> colorAttachmentInfo{};
		VkRenderingAttachmentInfo depthAttachmentInfo{};

		for (const auto& colorAtt : renderingInfo.colorAttachments)
		{
			VkRenderingAttachmentInfo& newInfo = colorAttachmentInfo.EmplaceBack();
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

		vkCmdBeginRendering(m_commandBufferData.commandBuffer, &vkRenderingInfo);
	}

	void VulkanCommandBuffer::EndRendering()
	{
		VT_PROFILE_FUNCTION();
		vkCmdEndRendering(m_commandBufferData.commandBuffer);
	}

	void VulkanCommandBuffer::PushConstants(const void* data, const uint32_t size, const uint32_t offset)
	{
		VT_PROFILE_FUNCTION();

#ifndef VT_DIST
		if (!m_currentRenderPipeline && !m_currentComputePipeline)
		{
			VT_LOGC(Error, LogVulkanRHI, "Unable to push constants as no pipeline is currently bound!");
		}
#endif

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

		vkCmdPushConstants(m_commandBufferData.commandBuffer, pipelineLayout, stageFlags, offset, size, data);
	}

	void AddGlobalBarrier(const GlobalBarrier& barrierInfo, VkMemoryBarrier2& outBarrier)
	{
		VT_PROFILE_FUNCTION();

		outBarrier.sType = VK_STRUCTURE_TYPE_MEMORY_BARRIER_2;
		outBarrier.pNext = nullptr;
		outBarrier.srcAccessMask = Utility::GetAccessFromBarrierAccess(barrierInfo.srcAccess);
		outBarrier.dstAccessMask = Utility::GetAccessFromBarrierAccess(barrierInfo.dstAccess);
		outBarrier.srcStageMask = Utility::GetStageFromBarrierStage(barrierInfo.srcStage);
		outBarrier.dstStageMask = Utility::GetStageFromBarrierStage(barrierInfo.dstStage);
	}

	void AddBufferBarrier(const BufferBarrier& barrierInfo, VkBufferMemoryBarrier2& outBarrier)
	{
		VT_PROFILE_FUNCTION();

		VT_ENSURE(barrierInfo.resource != nullptr);
		auto& vkBuffer = barrierInfo.resource->AsRef<VulkanStorageBuffer>();

		outBarrier.sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER_2;
		outBarrier.pNext = nullptr;

		if (barrierInfo.srcStage == BarrierStage::Clear)
		{
			outBarrier.srcAccessMask = VK_ACCESS_2_TRANSFER_WRITE_BIT;
			outBarrier.srcStageMask = VK_PIPELINE_STAGE_2_TRANSFER_BIT;
		}
		else
		{
			outBarrier.srcAccessMask = Utility::GetAccessFromBarrierAccess(barrierInfo.srcAccess);
			outBarrier.srcStageMask = Utility::GetStageFromBarrierStage(barrierInfo.srcStage);
		}

		if (barrierInfo.dstStage == BarrierStage::Clear)
		{
			outBarrier.dstAccessMask = VK_ACCESS_2_TRANSFER_WRITE_BIT;
			outBarrier.dstStageMask = VK_PIPELINE_STAGE_2_TRANSFER_BIT;
		}
		else
		{
			outBarrier.dstAccessMask = Utility::GetAccessFromBarrierAccess(barrierInfo.dstAccess);
			outBarrier.dstStageMask = Utility::GetStageFromBarrierStage(barrierInfo.dstStage);
		}

		outBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		outBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		outBarrier.offset = barrierInfo.offset;
		outBarrier.size = barrierInfo.size;
		outBarrier.buffer = vkBuffer.GetHandle<VkBuffer>();

		GraphicsContext::GetResourceStateTracker()->TransitionResource(barrierInfo.resource, barrierInfo.dstStage, barrierInfo.dstAccess);
	}

	void AddImageBarrier(const ImageBarrier& barrierInfo, VkImageMemoryBarrier2& outBarrier)
	{
		VT_PROFILE_FUNCTION();

		VT_ENSURE(barrierInfo.resource != nullptr);

		VkImageAspectFlags aspectFlags = Utility::GetVkImageAspect(barrierInfo.resource->As<Image>()->GetImageAspect());
		VT_ASSERT(aspectFlags != VK_IMAGE_ASPECT_FLAG_BITS_MAX_ENUM && aspectFlags != VK_IMAGE_ASPECT_NONE);

		outBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2;
		outBarrier.pNext = nullptr;

		if (barrierInfo.srcStage == BarrierStage::Clear)
		{
			outBarrier.srcAccessMask = VK_ACCESS_2_TRANSFER_WRITE_BIT;
			outBarrier.srcStageMask = VK_PIPELINE_STAGE_2_TRANSFER_BIT;
			outBarrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
		}
		else
		{
			outBarrier.srcAccessMask = Utility::GetAccessFromBarrierAccess(barrierInfo.srcAccess);
			outBarrier.srcStageMask = Utility::GetStageFromBarrierStage(barrierInfo.srcStage);
			outBarrier.oldLayout = Utility::GetVkImageLayoutFromImageLayout(barrierInfo.srcLayout);
		}

		if (barrierInfo.dstStage == BarrierStage::Clear)
		{
			outBarrier.dstAccessMask = VK_ACCESS_2_TRANSFER_WRITE_BIT;
			outBarrier.dstStageMask = VK_PIPELINE_STAGE_2_TRANSFER_BIT;
			outBarrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
		}
		else
		{
			outBarrier.dstAccessMask = Utility::GetAccessFromBarrierAccess(barrierInfo.dstAccess);
			outBarrier.dstStageMask = Utility::GetStageFromBarrierStage(barrierInfo.dstStage);
			outBarrier.newLayout = Utility::GetVkImageLayoutFromImageLayout(barrierInfo.dstLayout);
		}

		outBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		outBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		outBarrier.subresourceRange.aspectMask = aspectFlags;
		outBarrier.subresourceRange.baseArrayLayer = barrierInfo.subResource.baseArrayLayer;
		outBarrier.subresourceRange.baseMipLevel = barrierInfo.subResource.baseMipLevel;
		outBarrier.subresourceRange.layerCount = barrierInfo.subResource.layerCount == ALL_LAYERS ? VK_REMAINING_ARRAY_LAYERS : barrierInfo.subResource.layerCount;
		outBarrier.subresourceRange.levelCount = barrierInfo.subResource.levelCount == ALL_MIPS ? VK_REMAINING_MIP_LEVELS : barrierInfo.subResource.levelCount;
		outBarrier.image = barrierInfo.resource->GetHandle<VkImage>();

		GraphicsContext::GetResourceStateTracker()->TransitionResource(barrierInfo.resource, barrierInfo.dstStage, barrierInfo.dstAccess, barrierInfo.dstLayout);
	}

	void VulkanCommandBuffer::ResourceBarrier(const Vector<ResourceBarrierInfo>& resourceBarriers)
	{
		VT_PROFILE_FUNCTION();

		Vector<VkImageMemoryBarrier2> imageBarriers{};
		Vector<VkBufferMemoryBarrier2> bufferBarriers{};
		Vector<VkMemoryBarrier2> memoryBarriers{};

		for (const auto& resourceBarrier : resourceBarriers)
		{
			VT_ENSURE(resourceBarrier.type != BarrierType::None);

			switch (resourceBarrier.type)
			{
				case BarrierType::Global:
					AddGlobalBarrier(resourceBarrier.globalBarrier(), memoryBarriers.emplace_back());
					break;

				case BarrierType::Buffer:
					AddBufferBarrier(resourceBarrier.bufferBarrier(), bufferBarriers.emplace_back());
					break;

				case BarrierType::Image:
					AddImageBarrier(resourceBarrier.imageBarrier(), imageBarriers.emplace_back());
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

		vkCmdPipelineBarrier2(m_commandBufferData.commandBuffer, &info);
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

			Volt::RHI::vkCmdBeginDebugUtilsLabelEXT(m_commandBufferData.commandBuffer, &markerInfo);
		}
	}

	void VulkanCommandBuffer::EndMarker()
	{
		VT_PROFILE_FUNCTION();

		if (Volt::RHI::vkCmdEndDebugUtilsLabelEXT)
		{
			Volt::RHI::vkCmdEndDebugUtilsLabelEXT(m_commandBufferData.commandBuffer);
		}
	}

	const uint32_t VulkanCommandBuffer::BeginTimestamp()
	{
		VT_PROFILE_FUNCTION();

		if (!m_hasTimestampSupport)
		{
			return 0;
		}

		const uint32_t queryId = m_nextAvailableTimestampQuery;
		m_nextAvailableTimestampQuery += 2;

		vkCmdWriteTimestamp2(m_commandBufferData.commandBuffer, VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT, m_timestampQueryPool, queryId);
		return queryId;
	}

	void VulkanCommandBuffer::EndTimestamp(uint32_t timestampIndex)
	{
		VT_PROFILE_FUNCTION();

		if (!m_hasTimestampSupport)
		{
			return;
		}

		vkCmdWriteTimestamp2(m_commandBufferData.commandBuffer, VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT, m_timestampQueryPool, timestampIndex + 1);
	}

	const float VulkanCommandBuffer::GetExecutionTime(uint32_t timestampIndex) const
	{
		VT_PROFILE_FUNCTION();

		if (!m_hasTimestampSupport)
		{
			return 0.f;
		}

		if (timestampIndex == UINT32_MAX || timestampIndex / 2 >= m_timestampCount / 2)
		{
			return 0.f;
		}

		return m_executionTimes.at(timestampIndex / 2);
	}

	void VulkanCommandBuffer::ClearImage(WeakPtr<Image> image, std::array<float, 4> clearColor)
	{
		VT_PROFILE_FUNCTION();

		VulkanImage& vkImage = image->AsRef<VulkanImage>();

		VkImageAspectFlags imageAspect = static_cast<VkImageAspectFlags>(vkImage.GetImageAspect());

		VkImageSubresourceRange range{};
		range.aspectMask = imageAspect;
		range.baseArrayLayer = 0;
		range.baseMipLevel = 0;
		range.layerCount = VK_REMAINING_ARRAY_LAYERS;
		range.levelCount = VK_REMAINING_MIP_LEVELS;

		const auto& currentState = GraphicsContext::GetResourceStateTracker()->GetCurrentResourceState(image);

		// This is a bit of a hack due to the differences in clearing images between Vulkan and D3D12
		const VkImageLayout layout = EnumValueContainsFlag(currentState.stage, BarrierStage::Clear) ? VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL : Utility::GetVkImageLayoutFromImageLayout(currentState.layout);

		if (imageAspect & VK_IMAGE_ASPECT_COLOR_BIT)
		{
			VkClearColorValue vkClearColor{};
			vkClearColor.float32[0] = clearColor[0];
			vkClearColor.float32[1] = clearColor[1];
			vkClearColor.float32[2] = clearColor[2];
			vkClearColor.float32[3] = clearColor[3];


			vkCmdClearColorImage(m_commandBufferData.commandBuffer, image->GetHandle<VkImage>(), layout, &vkClearColor, 1, &range);
		}
		else
		{
			VkClearDepthStencilValue vkClearColor{};
			vkClearColor.depth = clearColor[0];
			vkClearColor.stencil = static_cast<uint32_t>(clearColor[1]);

			vkCmdClearDepthStencilImage(m_commandBufferData.commandBuffer, image->GetHandle<VkImage>(), layout, &vkClearColor, 1, &range);
		}
	}

	void VulkanCommandBuffer::ClearBuffer(WeakPtr<StorageBuffer> buffer, const uint32_t value)
	{
		VT_PROFILE_FUNCTION();

		VulkanStorageBuffer& vkBuffer = buffer->AsRef<VulkanStorageBuffer>();
		vkCmdFillBuffer(m_commandBufferData.commandBuffer, vkBuffer.GetHandle<VkBuffer>(), 0, vkBuffer.GetByteSize(), value);
	}

	void VulkanCommandBuffer::UpdateBuffer(WeakPtr<StorageBuffer> dstBuffer, const size_t dstOffset, const size_t dataSize, const void* data)
	{
		VT_PROFILE_FUNCTION();

		VT_ASSERT(dataSize <= 65536 && "Size must not exceed MAX_UPDATE_SIZE!");

		VulkanStorageBuffer& vkBuffer = dstBuffer->AsRef<VulkanStorageBuffer>();
		vkCmdUpdateBuffer(m_commandBufferData.commandBuffer, vkBuffer.GetHandle<VkBuffer>(), dstOffset, dataSize, data);
	}

	void VulkanCommandBuffer::CopyBufferRegion(WeakPtr<Allocation> srcResource, const size_t srcOffset, WeakPtr<Allocation> dstResource, const size_t dstOffset, const size_t size)
	{
		VT_PROFILE_FUNCTION();

		VkBufferCopy copy{};
		copy.srcOffset = srcOffset;
		copy.dstOffset = dstOffset;
		copy.size = size;

		vkCmdCopyBuffer(m_commandBufferData.commandBuffer, srcResource->GetResourceHandle<VkBuffer>(), dstResource->GetResourceHandle<VkBuffer>(), 1, &copy);
	}

	void VulkanCommandBuffer::CopyBufferToImage(WeakPtr<Allocation> srcBuffer, WeakPtr<Image> dstImage, const uint32_t width, const uint32_t height, const uint32_t depth, const uint32_t mip)
	{
		VT_PROFILE_FUNCTION();

		VT_ENSURE_MSG(height >= 1 && width >= 1 && depth >= 1, "All dimensions must be equal to or greater than one!");

		auto& vkImage = dstImage->AsRef<VulkanImage>();

		VkBufferImageCopy region{};
		region.bufferOffset = 0;
		region.bufferRowLength = 0;
		region.bufferImageHeight = 0;

		region.imageSubresource.aspectMask = static_cast<VkImageAspectFlags>(vkImage.GetImageAspect());
		region.imageSubresource.mipLevel = mip;
		region.imageSubresource.baseArrayLayer = 0;
		region.imageSubresource.layerCount = 1;

		region.imageOffset = { 0, 0, 0 };
		region.imageExtent = { width, height, depth };

		const auto& currentState = GraphicsContext::GetResourceStateTracker()->GetCurrentResourceState(dstImage);
		vkCmdCopyBufferToImage(m_commandBufferData.commandBuffer, srcBuffer->GetResourceHandle<VkBuffer>(), dstImage->GetHandle<VkImage>(), Utility::GetVkImageLayoutFromImageLayout(currentState.layout), 1, &region);
	}

	void VulkanCommandBuffer::CopyImageToBuffer(WeakPtr<Image> srcImage, WeakPtr<Allocation> dstBuffer, const size_t dstOffset, const uint32_t width, const uint32_t height, const uint32_t depth, const uint32_t mip)
	{
		VT_PROFILE_FUNCTION();

		VT_ENSURE_MSG(height >= 1 && width >= 1 && depth >= 1, "All dimensions must be equal to or greater than one!");
		VT_ENSURE_MSG(mip < srcImage->CalculateMipCount(), "Mip level is not valid!");

		auto& vkImage = srcImage->AsRef<VulkanImage>();

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

		const auto& currentState = GraphicsContext::GetResourceStateTracker()->GetCurrentResourceState(srcImage);
		vkCmdCopyImageToBuffer(m_commandBufferData.commandBuffer, srcImage->GetHandle<VkImage>(), Utility::GetVkImageLayoutFromImageLayout(currentState.layout), dstBuffer->GetResourceHandle<VkBuffer>(), 1, &region);
	}

	void VulkanCommandBuffer::CopyImage(WeakPtr<Image> srcImage, WeakPtr<Image> dstImage, const uint32_t width, const uint32_t height, const uint32_t depth)
	{
		VT_PROFILE_FUNCTION();

		VT_ENSURE_MSG(height >= 1 && width >= 1 && depth >= 1, "All dimensions must be equal to or greater than one!");

		VulkanImage& srcVkImage = srcImage->AsRef<VulkanImage>();
		VulkanImage& dstVkImage = dstImage->AsRef<VulkanImage>();

		const VkImageAspectFlags srcImageAspect = static_cast<VkImageAspectFlags>(srcVkImage.GetImageAspect());
		const VkImageAspectFlags dstImageAspect = static_cast<VkImageAspectFlags>(dstVkImage.GetImageAspect());

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

		const auto& currentSrcState = GraphicsContext::GetResourceStateTracker()->GetCurrentResourceState(srcImage);
		const auto& currentDstState = GraphicsContext::GetResourceStateTracker()->GetCurrentResourceState(dstImage);

		VkCopyImageInfo2 cpyInfo{};
		cpyInfo.sType = VK_STRUCTURE_TYPE_COPY_IMAGE_INFO_2;
		cpyInfo.pNext = nullptr;
		cpyInfo.srcImage = srcVkImage.GetHandle<VkImage>();
		cpyInfo.srcImageLayout = Utility::GetVkImageLayoutFromImageLayout(currentSrcState.layout);
		cpyInfo.dstImage = dstVkImage.GetHandle<VkImage>();
		cpyInfo.dstImageLayout = Utility::GetVkImageLayoutFromImageLayout(currentDstState.layout);
		cpyInfo.pRegions = &info;
		cpyInfo.regionCount = 1;

		vkCmdCopyImage2(m_commandBufferData.commandBuffer, &cpyInfo);
	}

	void VulkanCommandBuffer::UploadTextureData(WeakPtr<Image> dstImage, const ImageCopyData& copyData)
	{
		VT_PROFILE_FUNCTION();

		auto& vkImage = dstImage->AsRef<VulkanImage>();

		Vector<VkBufferImageCopy> copyRegions;
		copyRegions.reserve(copyData.copySubData.size());

		const uint64_t stagingSize = Utility::CalculateStagingBufferSize(copyData);
		RefPtr<Allocation> stagingAlloc = GraphicsContext::GetDefaultAllocator()->CreateBuffer(stagingSize, BufferUsage::TransferSrc, MemoryUsage::CPUToGPU);

		uint8_t* stagingPtr = stagingAlloc->Map<uint8_t>();

		uint64_t offset = 0;
		for (const auto& subData : copyData.copySubData)
		{
			auto& newRegion = copyRegions.emplace_back();
			newRegion.bufferOffset = offset;
			newRegion.bufferRowLength = 0;
			newRegion.bufferImageHeight = 0;

			newRegion.imageSubresource.aspectMask = static_cast<VkImageAspectFlags>(vkImage.GetImageAspect());
			newRegion.imageSubresource.mipLevel = subData.subResource.baseMipLevel;
			newRegion.imageSubresource.baseArrayLayer = subData.subResource.baseArrayLayer;
			newRegion.imageSubresource.layerCount = subData.subResource.layerCount;

			newRegion.imageOffset = { 0, 0, 0 };
			newRegion.imageExtent = { subData.width, subData.height, subData.depth };

			memcpy_s(&stagingPtr[offset], stagingSize, subData.data, subData.slicePitch);
			offset += subData.slicePitch;
		}

		stagingAlloc->Unmap();

		const auto& currentState = GraphicsContext::GetResourceStateTracker()->GetCurrentResourceState(dstImage);
		vkCmdCopyBufferToImage(m_commandBufferData.commandBuffer, stagingAlloc->GetResourceHandle<VkBuffer>(), dstImage->GetHandle<VkImage>(), Utility::GetVkImageLayoutFromImageLayout(currentState.layout), static_cast<uint32_t>(copyRegions.size()), copyRegions.data());
	}

	const QueueType VulkanCommandBuffer::GetQueueType() const
	{
		return m_queueType;
	}

	const CommandBufferLevel VulkanCommandBuffer::GetCommandBufferLevel() const
	{
		return m_commandBufferLevel;
	}

	const WeakPtr<Fence> VulkanCommandBuffer::GetFence() const
	{
		return m_fence;
	}

	RefPtr<CommandBuffer> VulkanCommandBuffer::CreateSecondaryCommandBuffer() const
	{
		VT_PROFILE_FUNCTION();
		VT_ENSURE(m_commandBufferLevel == CommandBufferLevel::Primary);
		return RefPtr<VulkanCommandBuffer>::Create(this);
	}

	void VulkanCommandBuffer::ExecuteSecondaryCommandBuffer(RefPtr<CommandBuffer> commandBuffer) const
	{
		VT_ENSURE(m_commandBufferLevel == CommandBufferLevel::Primary);
		VT_ENSURE(commandBuffer->GetCommandBufferLevel() == CommandBufferLevel::Secondary);

		VkCommandBuffer cmdBuffer = commandBuffer->GetHandle<VkCommandBuffer>();
		vkCmdExecuteCommands(m_commandBufferData.commandBuffer, 1, &cmdBuffer);
	}

	void VulkanCommandBuffer::ExecuteSecondaryCommandBuffers(Vector<RefPtr<CommandBuffer>> commandBuffers) const
	{
		VT_ENSURE(m_commandBufferLevel == CommandBufferLevel::Primary);

		Vector<VkCommandBuffer> vkCommandBuffers;
		vkCommandBuffers.reserve(commandBuffers.size());

		for (const auto& cmdBuffer : commandBuffers)
		{
			VT_ENSURE(cmdBuffer->GetCommandBufferLevel() == CommandBufferLevel::Secondary);
			vkCommandBuffers.emplace_back(cmdBuffer->GetHandle<VkCommandBuffer>());
		}

		vkCmdExecuteCommands(m_commandBufferData.commandBuffer, static_cast<uint32_t>(vkCommandBuffers.size()), vkCommandBuffers.data());
	}

	void* VulkanCommandBuffer::GetHandleImpl() const
	{
		return m_commandBufferData.commandBuffer;
	}

	void VulkanCommandBuffer::Invalidate()
	{
		VT_PROFILE_FUNCTION();

		auto physicalDevice = GraphicsContext::GetPhysicalDevice()->As<VulkanPhysicalGraphicsDevice>();
		auto device = GraphicsContext::GetDevice();

		const auto& queueFamilies = physicalDevice->GetQueueFamilies();

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
				VT_ASSERT(false);
				break;
		}

		poolInfo.queueFamilyIndex = queueFamilyIndex;
		poolInfo.flags = 0;

		VT_VK_CHECK(vkCreateCommandPool(device->GetHandle<VkDevice>(), &poolInfo, nullptr, &m_commandBufferData.commandPool));

		VkCommandBufferAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		allocInfo.commandPool = m_commandBufferData.commandPool;
		allocInfo.level = m_commandBufferLevel == CommandBufferLevel::Primary ? VK_COMMAND_BUFFER_LEVEL_PRIMARY : VK_COMMAND_BUFFER_LEVEL_SECONDARY;
		allocInfo.commandBufferCount = 1;

		VT_VK_CHECK(vkAllocateCommandBuffers(device->GetHandle<VkDevice>(), &allocInfo, &m_commandBufferData.commandBuffer));

		FenceCreateInfo fenceInfo{};
		fenceInfo.createSignaled = true;

		m_fence = Fence::Create(fenceInfo);

		m_hasTimestampSupport = GraphicsContext::GetPhysicalDevice()->AsRef<VulkanPhysicalGraphicsDevice>().GetProperties().limits.timestampComputeAndGraphics;
		if (m_hasTimestampSupport)
		{
			CreateQueryPools();
		}
	}

	void VulkanCommandBuffer::Release(RefPtr<Fence> waitFence)
	{
		VT_PROFILE_FUNCTION();

		if (!m_commandBufferData.commandBuffer)
		{
			return;
		}

		VkFence fencePtr = waitFence->GetHandle<VkFence>();
		RHIProxy::GetInstance().DestroyResource([fence = fencePtr, commandPool = m_commandBufferData.commandPool, timestampPool = m_timestampQueryPool, level = m_commandBufferLevel]()
		{
			auto device = GraphicsContext::GetDevice();
		
			if (level == CommandBufferLevel::Primary)
			{
				VT_VK_CHECK(vkWaitForFences(device->GetHandle<VkDevice>(), 1, &fence, VK_TRUE, UINT64_MAX));
			}

			vkDestroyCommandPool(device->GetHandle<VkDevice>(), commandPool, nullptr);
			vkDestroyQueryPool(device->GetHandle<VkDevice>(), timestampPool, nullptr);
		});

		m_commandBufferData = {};
	}

	void VulkanCommandBuffer::CreateQueryPools()
	{
		VT_PROFILE_FUNCTION();

		auto device = GraphicsContext::GetDevice();

		m_timestampQueryCount = 2 + 2 * MAX_QUERIES;

		VkQueryPoolCreateInfo info{};
		info.sType = VK_STRUCTURE_TYPE_QUERY_POOL_CREATE_INFO;
		info.pNext = nullptr;
		info.queryType = VK_QUERY_TYPE_TIMESTAMP;
		info.queryCount = m_timestampQueryCount;

		VT_VK_CHECK(vkCreateQueryPool(device->GetHandle<VkDevice>(), &info, nullptr, &m_timestampQueryPool));

		m_timestampCount = 0u;

		m_timestampQueryResults.resize_uninitialized(m_timestampQueryCount);
		m_executionTimes.resize_uninitialized(m_timestampQueryCount / 2);
	}

	void VulkanCommandBuffer::FetchTimestampResults()
	{
		VT_PROFILE_FUNCTION();

		if (!m_hasTimestampSupport)
		{
			return;
		}

		if (m_timestampCount == 0)
		{
			return;
		}

		auto device = GraphicsContext::GetDevice();

		vkGetQueryPoolResults(device->GetHandle<VkDevice>(), m_timestampQueryPool, 0, m_timestampCount, m_timestampCount * sizeof(uint64_t), m_timestampQueryResults.data(), sizeof(uint64_t), VK_QUERY_RESULT_64_BIT);

		for (uint32_t i = 0; i < m_timestampCount; i += 2)
		{
			const uint64_t startTime = m_timestampQueryResults.at(i);
			const uint64_t endTime = m_timestampQueryResults.at(i + 1);

			const float nsTime = endTime > startTime ? (endTime - startTime) * GraphicsContext::GetPhysicalDevice()->AsRef<VulkanPhysicalGraphicsDevice>().GetProperties().limits.timestampPeriod : 0.f;
			m_executionTimes[i / 2] = nsTime * 0.000001f; // Convert to ms
		}
	}

	void VulkanCommandBuffer::BeginPrimaryInternal()
	{
		VT_PROFILE_FUNCTION();

		VkCommandBufferBeginInfo beginInfo{};
		beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		beginInfo.pNext = nullptr;
		beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
		beginInfo.pInheritanceInfo = nullptr;

		VT_VK_CHECK(vkBeginCommandBuffer(m_commandBufferData.commandBuffer, &beginInfo));
	}

	void VulkanCommandBuffer::BeginSecondaryInternal()
	{
		VT_PROFILE_FUNCTION();

		VkCommandBufferInheritanceInfo inheritanceInfo{};
		inheritanceInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_INHERITANCE_INFO;
		inheritanceInfo.pNext = nullptr;
		inheritanceInfo.renderPass = nullptr;
		inheritanceInfo.subpass = 0;
		inheritanceInfo.framebuffer = nullptr;
		inheritanceInfo.occlusionQueryEnable = VK_FALSE;
		inheritanceInfo.queryFlags = 0;
		inheritanceInfo.pipelineStatistics = 0;

		VkCommandBufferBeginInfo beginInfo{};
		beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		beginInfo.pNext = nullptr;
		beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
		beginInfo.pInheritanceInfo = &inheritanceInfo;

		VT_VK_CHECK(vkBeginCommandBuffer(m_commandBufferData.commandBuffer, &beginInfo));
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
