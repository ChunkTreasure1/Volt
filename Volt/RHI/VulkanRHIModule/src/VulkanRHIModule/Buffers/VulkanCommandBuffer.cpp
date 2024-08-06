#include "vkpch.h"
#include "VulkanCommandBuffer.h"

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

#include "VulkanRHIModule/Images/VulkanImage2D.h"
#include "VulkanRHIModule/Images/VulkanImage3D.h"
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

	VulkanCommandBuffer::VulkanCommandBuffer(const uint32_t count, QueueType queueType)
		: m_commandBufferCount(count), m_queueType(queueType)
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

		VT_VK_CHECK(vkWaitForFences(device->GetHandle<VkDevice>(), 1, &currentCommandBuffer.fence, VK_TRUE, UINT64_MAX));
		VT_VK_CHECK(vkResetFences(device->GetHandle<VkDevice>(), 1, &currentCommandBuffer.fence));
		VT_VK_CHECK(vkResetCommandPool(device->GetHandle<VkDevice>(), currentCommandBuffer.commandPool, 0));

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

		VT_VK_CHECK(vkWaitForFences(device->GetHandle<VkDevice>(), 1, &currentCommandBuffer.fence, VK_TRUE, UINT64_MAX));
		VT_VK_CHECK(vkResetFences(device->GetHandle<VkDevice>(), 1, &currentCommandBuffer.fence));
		VT_VK_CHECK(vkResetCommandPool(device->GetHandle<VkDevice>(), currentCommandBuffer.commandPool, 0));

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

		auto device = GraphicsContext::GetDevice();
		device->GetDeviceQueue(m_queueType)->Execute({ { this } });
	}

	void VulkanCommandBuffer::ExecuteAndWait()
	{
		VT_PROFILE_FUNCTION();

		auto device = GraphicsContext::GetDevice();
		device->GetDeviceQueue(m_queueType)->Execute({ { this } });
		VT_VK_CHECK(vkWaitForFences(device->GetHandle<VkDevice>(), 1, &m_commandBuffers.at(m_currentCommandBufferIndex).fence, VK_TRUE, UINT64_MAX));

		FetchTimestampResults();
	}

	void VulkanCommandBuffer::WaitForFences()
	{
		VT_PROFILE_FUNCTION();

		Vector<VkFence> fences{};
		for (const auto& cmdBuffer : m_commandBuffers)
		{
			fences.emplace_back(cmdBuffer.fence);
		}

		auto device = GraphicsContext::GetDevice();
		VT_VK_CHECK(vkWaitForFences(device->GetHandle<VkDevice>(), static_cast<uint32_t>(fences.size()), fences.data(), VK_TRUE, UINT64_MAX));
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

		const uint32_t index = GetCurrentCommandBufferIndex();
		VT_PROFILE_GPU_CONTEXT(m_commandBuffers.at(index).commandBuffer);
		VT_PROFILE_GPU_EVENT("Draw");

		vkCmdDraw(m_commandBuffers.at(index).commandBuffer, vertexCount, instanceCount, firstVertex, firstInstance);
	}

	void VulkanCommandBuffer::DrawIndexed(const uint32_t indexCount, const uint32_t instanceCount, const uint32_t firstIndex, const uint32_t vertexOffset, const uint32_t firstInstance)
	{
		VT_PROFILE_FUNCTION();

#ifdef VT_ENABLE_COMMAND_BUFFER_VALIDATION
		VT_ENSURE(m_currentRenderPipeline != nullptr);
#endif

		const uint32_t index = GetCurrentCommandBufferIndex();
		VT_PROFILE_GPU_CONTEXT(m_commandBuffers.at(index).commandBuffer);
		VT_PROFILE_GPU_EVENT("DrawIndexed");

		vkCmdDrawIndexed(m_commandBuffers.at(index).commandBuffer, indexCount, instanceCount, firstIndex, vertexOffset, firstInstance);
	}

	void VulkanCommandBuffer::DrawIndexedIndirect(WeakPtr<StorageBuffer> commandsBuffer, const size_t offset, const uint32_t drawCount, const uint32_t stride)
	{
		VT_PROFILE_FUNCTION();

#ifdef VT_ENABLE_COMMAND_BUFFER_VALIDATION
		VT_ENSURE(m_currentRenderPipeline != nullptr);
#endif

		const uint32_t index = GetCurrentCommandBufferIndex();
		VT_PROFILE_GPU_CONTEXT(m_commandBuffers.at(index).commandBuffer);
		VT_PROFILE_GPU_EVENT("DrawIndexedIndirect");

		vkCmdDrawIndexedIndirect(m_commandBuffers.at(index).commandBuffer, commandsBuffer->GetHandle<VkBuffer>(), offset, drawCount, stride);
	}

	void VulkanCommandBuffer::DrawIndirect(WeakPtr<StorageBuffer> commandsBuffer, const size_t offset, const uint32_t drawCount, const uint32_t stride)
	{
		VT_PROFILE_FUNCTION();

#ifdef VT_ENABLE_COMMAND_BUFFER_VALIDATION
		VT_ENSURE(m_currentRenderPipeline != nullptr);
#endif

		const uint32_t index = GetCurrentCommandBufferIndex();
		VT_PROFILE_GPU_CONTEXT(m_commandBuffers.at(index).commandBuffer);
		VT_PROFILE_GPU_EVENT("DrawIndirect");

		vkCmdDrawIndirect(m_commandBuffers.at(index).commandBuffer, commandsBuffer->GetHandle<VkBuffer>(), offset, drawCount, stride);
	}

	void VulkanCommandBuffer::DrawIndexedIndirectCount(WeakPtr<StorageBuffer> commandsBuffer, const size_t offset, WeakPtr<StorageBuffer> countBuffer, const size_t countBufferOffset, const uint32_t maxDrawCount, const uint32_t stride)
	{
		VT_PROFILE_FUNCTION();

#ifdef VT_ENABLE_COMMAND_BUFFER_VALIDATION
		VT_ENSURE(m_currentRenderPipeline != nullptr);
#endif

		const uint32_t index = GetCurrentCommandBufferIndex();
		VT_PROFILE_GPU_CONTEXT(m_commandBuffers.at(index).commandBuffer);
		VT_PROFILE_GPU_EVENT("DrawIndexedIndirectCount");

		vkCmdDrawIndexedIndirectCount(m_commandBuffers.at(index).commandBuffer, commandsBuffer->GetHandle<VkBuffer>(), offset, countBuffer->GetHandle<VkBuffer>(), countBufferOffset, maxDrawCount, stride);
	}

	void VulkanCommandBuffer::DrawIndirectCount(WeakPtr<StorageBuffer> commandsBuffer, const size_t offset, WeakPtr<StorageBuffer> countBuffer, const size_t countBufferOffset, const uint32_t maxDrawCount, const uint32_t stride)
	{
		VT_PROFILE_FUNCTION();

#ifdef VT_ENABLE_COMMAND_BUFFER_VALIDATION
		VT_ENSURE(m_currentRenderPipeline != nullptr);
#endif

		const uint32_t index = GetCurrentCommandBufferIndex();
		VT_PROFILE_GPU_CONTEXT(m_commandBuffers.at(index).commandBuffer);
		VT_PROFILE_GPU_EVENT("DrawIndirectCount");

		vkCmdDrawIndirectCount(m_commandBuffers.at(index).commandBuffer, commandsBuffer->GetHandle<VkBuffer>(), offset, countBuffer->GetHandle<VkBuffer>(), countBufferOffset, maxDrawCount, stride);
	}

	void VulkanCommandBuffer::Dispatch(const uint32_t groupCountX, const uint32_t groupCountY, const uint32_t groupCountZ)
	{
		VT_PROFILE_FUNCTION();

#ifdef VT_ENABLE_COMMAND_BUFFER_VALIDATION
		VT_ENSURE(m_currentComputePipeline != nullptr);
#endif

		const uint32_t index = GetCurrentCommandBufferIndex();
		VT_PROFILE_GPU_CONTEXT(m_commandBuffers.at(index).commandBuffer);
		VT_PROFILE_GPU_EVENT("Dispatch");

		vkCmdDispatch(m_commandBuffers.at(index).commandBuffer, groupCountX, groupCountY, groupCountZ);
	}

	void VulkanCommandBuffer::DispatchIndirect(WeakPtr<StorageBuffer> commandsBuffer, const size_t offset)
	{
		VT_PROFILE_FUNCTION();

#ifdef VT_ENABLE_COMMAND_BUFFER_VALIDATION
		VT_ENSURE(m_currentComputePipeline != nullptr);
#endif

		const uint32_t index = GetCurrentCommandBufferIndex();
		VT_PROFILE_GPU_CONTEXT(m_commandBuffers.at(index).commandBuffer);
		VT_PROFILE_GPU_EVENT("DispatchIndirect");

		vkCmdDispatchIndirect(m_commandBuffers.at(index).commandBuffer, commandsBuffer->GetHandle<VkBuffer>(), offset);
	}

	void VulkanCommandBuffer::DispatchMeshTasks(const uint32_t groupCountX, const uint32_t groupCountY, const uint32_t groupCountZ)
	{
		VT_PROFILE_FUNCTION();

#ifdef VT_ENABLE_COMMAND_BUFFER_VALIDATION
		VT_ENSURE(m_currentRenderPipeline != nullptr);
#endif

		const uint32_t index = GetCurrentCommandBufferIndex();
		vkCmdDrawMeshTasksEXT(m_commandBuffers.at(index).commandBuffer, groupCountX, groupCountY, groupCountZ);
	}

	void VulkanCommandBuffer::DispatchMeshTasksIndirect(WeakPtr<StorageBuffer> commandsBuffer, const size_t offset, const uint32_t drawCount, const uint32_t stride)
	{
		VT_PROFILE_FUNCTION();

#ifdef VT_ENABLE_COMMAND_BUFFER_VALIDATION
		VT_ENSURE(m_currentRenderPipeline != nullptr);
#endif

		const uint32_t index = GetCurrentCommandBufferIndex();
		vkCmdDrawMeshTasksIndirectEXT(m_commandBuffers.at(index).commandBuffer, commandsBuffer->GetHandle<VkBuffer>(), offset, drawCount, stride);
	}

	void VulkanCommandBuffer::DispatchMeshTasksIndirectCount(WeakPtr<StorageBuffer> commandsBuffer, const size_t offset, WeakPtr<StorageBuffer> countBuffer, const size_t countBufferOffset, const uint32_t maxDrawCount, const uint32_t stride)
	{
		VT_PROFILE_FUNCTION();

#ifdef VT_ENABLE_COMMAND_BUFFER_VALIDATION
		VT_ENSURE(m_currentRenderPipeline != nullptr);
#endif

		const uint32_t index = GetCurrentCommandBufferIndex();
		vkCmdDrawMeshTasksIndirectCountEXT(m_commandBuffers.at(index).commandBuffer, commandsBuffer->GetHandle<VkBuffer>(), offset, countBuffer->GetHandle<VkBuffer>(), countBufferOffset, maxDrawCount, stride);
	}

	void VulkanCommandBuffer::SetViewports(const StackVector<Viewport, MAX_VIEWPORT_COUNT>& viewports)
	{
		VT_PROFILE_FUNCTION();

		const uint32_t index = GetCurrentCommandBufferIndex();
		VT_PROFILE_GPU_CONTEXT(m_commandBuffers.at(index).commandBuffer);

		vkCmdSetViewport(m_commandBuffers.at(index).commandBuffer, 0, static_cast<uint32_t>(viewports.Size()), reinterpret_cast<const VkViewport*>(viewports.Data()));
	}

	void VulkanCommandBuffer::SetScissors(const StackVector<Rect2D, MAX_VIEWPORT_COUNT>& scissors)
	{
		VT_PROFILE_FUNCTION();

		const uint32_t index = GetCurrentCommandBufferIndex();
		VT_PROFILE_GPU_CONTEXT(m_commandBuffers.at(index).commandBuffer);

		vkCmdSetScissor(m_commandBuffers.at(index).commandBuffer, 0, static_cast<uint32_t>(scissors.Size()), reinterpret_cast<const VkRect2D*>(scissors.Data()));
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

		const uint32_t index = GetCurrentCommandBufferIndex();
		VT_PROFILE_GPU_CONTEXT(m_commandBuffers.at(index).commandBuffer);

		vkCmdBindPipeline(m_commandBuffers.at(index).commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline->GetHandle<VkPipeline>());
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

		const uint32_t index = GetCurrentCommandBufferIndex();
		VT_PROFILE_GPU_CONTEXT(m_commandBuffers.at(index).commandBuffer);

		vkCmdBindPipeline(m_commandBuffers.at(index).commandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, pipeline->GetHandle<VkPipeline>());
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

		const uint32_t index = GetCurrentCommandBufferIndex();
		VT_PROFILE_GPU_CONTEXT(m_commandBuffers.at(index).commandBuffer);

		vkCmdBindVertexBuffers(m_commandBuffers.at(index).commandBuffer, firstBinding, static_cast<uint32_t>(vkBuffers.Size()), vkBuffers.Data(), offsets.Data());
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

		const uint32_t index = GetCurrentCommandBufferIndex();
		VT_PROFILE_GPU_CONTEXT(m_commandBuffers.at(index).commandBuffer);

		vkCmdBindVertexBuffers(m_commandBuffers.at(index).commandBuffer, firstBinding, static_cast<uint32_t>(vkBuffers.Size()), vkBuffers.Data(), offsets.Data());
	}

	void VulkanCommandBuffer::BindIndexBuffer(WeakPtr<IndexBuffer> indexBuffer)
	{
		VT_PROFILE_FUNCTION();

		const uint32_t index = GetCurrentCommandBufferIndex();
		VT_PROFILE_GPU_CONTEXT(m_commandBuffers.at(index).commandBuffer);

		constexpr VkDeviceSize offset = 0;
		vkCmdBindIndexBuffer(m_commandBuffers.at(index).commandBuffer, indexBuffer->GetHandle<VkBuffer>(), offset, VK_INDEX_TYPE_UINT32);
	}

	void VulkanCommandBuffer::BindIndexBuffer(WeakPtr<StorageBuffer> indexBuffer)
	{
		VT_PROFILE_FUNCTION();

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

	void VulkanCommandBuffer::BindDescriptorTable(WeakPtr<BindlessDescriptorTable> descriptorTable)
	{
		VT_PROFILE_FUNCTION();
		VT_PROFILE_GPU_CONTEXT(m_commandBuffers.at(m_currentCommandBufferIndex).commandBuffer);
		descriptorTable->AsRef<VulkanBindlessDescriptorTable>().Bind(*this);
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
			VT_LOGC(LogVerbosity::Error, LogVulkanRHI, "Unable to push constants as no pipeline is currently bound!");
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

		VkImageAspectFlags aspectFlags = VK_IMAGE_ASPECT_FLAG_BITS_MAX_ENUM;

		if (barrierInfo.resource->GetType() == ResourceType::Image2D)
		{
			auto& vkImage = barrierInfo.resource->AsRef<VulkanImage2D>();
			aspectFlags = static_cast<VkImageAspectFlags>(vkImage.GetImageAspect());
		}
		else if (barrierInfo.resource->GetType() == ResourceType::Image3D)
		{
			aspectFlags = VK_IMAGE_ASPECT_COLOR_BIT;
		}

		VT_ASSERT(aspectFlags != VK_IMAGE_ASPECT_FLAG_BITS_MAX_ENUM);

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
		VT_PROFILE_FUNCTION();

		VulkanImage2D& vkImage = image->AsRef<VulkanImage2D>();

		VkImageAspectFlags imageAspect = static_cast<VkImageAspectFlags>(vkImage.GetImageAspect());
		const uint32_t index = GetCurrentCommandBufferIndex();

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


			vkCmdClearColorImage(m_commandBuffers.at(index).commandBuffer, image->GetHandle<VkImage>(), layout, &vkClearColor, 1, &range);
		}
		else
		{
			VkClearDepthStencilValue vkClearColor{};
			vkClearColor.depth = clearColor[0];
			vkClearColor.stencil = static_cast<uint32_t>(clearColor[1]);

			vkCmdClearDepthStencilImage(m_commandBuffers.at(index).commandBuffer, image->GetHandle<VkImage>(), layout, &vkClearColor, 1, &range);
		}
	}

	void VulkanCommandBuffer::ClearImage(WeakPtr<Image3D> image, std::array<float, 4> clearColor)
	{
		VT_PROFILE_FUNCTION();

		const VkImageAspectFlags imageAspect = VK_IMAGE_ASPECT_COLOR_BIT;
		const uint32_t index = GetCurrentCommandBufferIndex();

		VkImageSubresourceRange range{};
		range.aspectMask = imageAspect;
		range.baseArrayLayer = 0;
		range.baseMipLevel = 0;
		range.layerCount = VK_REMAINING_ARRAY_LAYERS;
		range.levelCount = VK_REMAINING_MIP_LEVELS;

		const auto& currentState = GraphicsContext::GetResourceStateTracker()->GetCurrentResourceState(image);

		// This is a bit of a hack due to the differences in clearing images between Vulkan and D3D12
		const VkImageLayout layout = EnumValueContainsFlag(currentState.stage, BarrierStage::Clear) ? VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL : Utility::GetVkImageLayoutFromImageLayout(currentState.layout);

		VkClearColorValue vkClearColor{};
		vkClearColor.float32[0] = clearColor[0];
		vkClearColor.float32[1] = clearColor[1];
		vkClearColor.float32[2] = clearColor[2];
		vkClearColor.float32[3] = clearColor[3];

		vkCmdClearColorImage(m_commandBuffers.at(index).commandBuffer, image->GetHandle<VkImage>(), layout, &vkClearColor, 1, &range);
	}

	void VulkanCommandBuffer::ClearBuffer(WeakPtr<StorageBuffer> buffer, const uint32_t value)
	{
		VT_PROFILE_FUNCTION();

		VulkanStorageBuffer& vkBuffer = buffer->AsRef<VulkanStorageBuffer>();
		const uint32_t index = GetCurrentCommandBufferIndex();
		vkCmdFillBuffer(m_commandBuffers.at(index).commandBuffer, vkBuffer.GetHandle<VkBuffer>(), 0, vkBuffer.GetByteSize(), value);
	}

	void VulkanCommandBuffer::UpdateBuffer(WeakPtr<StorageBuffer> dstBuffer, const size_t dstOffset, const size_t dataSize, const void* data)
	{
		VT_PROFILE_FUNCTION();

		VT_ASSERT(dataSize <= 65536 && "Size must not exceed MAX_UPDATE_SIZE!");

		VulkanStorageBuffer& vkBuffer = dstBuffer->AsRef<VulkanStorageBuffer>();
		const uint32_t index = GetCurrentCommandBufferIndex();
		vkCmdUpdateBuffer(m_commandBuffers.at(index).commandBuffer, vkBuffer.GetHandle<VkBuffer>(), dstOffset, dataSize, data);
	}

	void VulkanCommandBuffer::CopyBufferRegion(WeakPtr<Allocation> srcResource, const size_t srcOffset, WeakPtr<Allocation> dstResource, const size_t dstOffset, const size_t size)
	{
		VT_PROFILE_FUNCTION();

		const uint32_t index = GetCurrentCommandBufferIndex();

		VkBufferCopy copy{};
		copy.srcOffset = srcOffset;
		copy.dstOffset = dstOffset;
		copy.size = size;

		vkCmdCopyBuffer(m_commandBuffers.at(index).commandBuffer, srcResource->GetResourceHandle<VkBuffer>(), dstResource->GetResourceHandle<VkBuffer>(), 1, &copy);
	}

	void VulkanCommandBuffer::CopyBufferToImage(WeakPtr<Allocation> srcBuffer, WeakPtr<Image2D> dstImage, const uint32_t width, const uint32_t height, const uint32_t mip)
	{
		VT_PROFILE_FUNCTION();

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

		const auto& currentState = GraphicsContext::GetResourceStateTracker()->GetCurrentResourceState(dstImage);
		vkCmdCopyBufferToImage(m_commandBuffers.at(index).commandBuffer, srcBuffer->GetResourceHandle<VkBuffer>(), dstImage->GetHandle<VkImage>(), Utility::GetVkImageLayoutFromImageLayout(currentState.layout), 1, &region);
	}

	void VulkanCommandBuffer::CopyBufferToImage(WeakPtr<Allocation> srcBuffer, WeakPtr<Image3D> dstImage, const uint32_t width, const uint32_t height, const uint32_t depth, const uint32_t mip)
	{
		VT_PROFILE_FUNCTION();

		const uint32_t index = GetCurrentCommandBufferIndex();

		VkBufferImageCopy region{};
		region.bufferOffset = 0;
		region.bufferRowLength = 0;
		region.bufferImageHeight = 0;

		region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		region.imageSubresource.mipLevel = mip;
		region.imageSubresource.baseArrayLayer = 0;
		region.imageSubresource.layerCount = 1;

		region.imageOffset = { 0, 0, 0 };
		region.imageExtent = { width, height, depth };

		const auto& currentState = GraphicsContext::GetResourceStateTracker()->GetCurrentResourceState(dstImage);
		vkCmdCopyBufferToImage(m_commandBuffers.at(index).commandBuffer, srcBuffer->GetResourceHandle<VkBuffer>(), dstImage->GetHandle<VkImage>(), Utility::GetVkImageLayoutFromImageLayout(currentState.layout), 1, &region);
	}

	void VulkanCommandBuffer::CopyImageToBuffer(WeakPtr<Image2D> srcImage, WeakPtr<Allocation> dstBuffer, const size_t dstOffset, const uint32_t width, const uint32_t height, const uint32_t mip)
	{
		VT_PROFILE_FUNCTION();

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

		const auto& currentState = GraphicsContext::GetResourceStateTracker()->GetCurrentResourceState(srcImage);
		vkCmdCopyImageToBuffer(m_commandBuffers.at(index).commandBuffer, srcImage->GetHandle<VkImage>(), Utility::GetVkImageLayoutFromImageLayout(currentState.layout), dstBuffer->GetResourceHandle<VkBuffer>(), 1, &region);
	}

	void VulkanCommandBuffer::CopyImage(WeakPtr<Image2D> srcImage, WeakPtr<Image2D> dstImage, const uint32_t width, const uint32_t height)
	{
		VT_PROFILE_FUNCTION();

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

		vkCmdCopyImage2(m_commandBuffers.at(index).commandBuffer, &cpyInfo);
	}

	void VulkanCommandBuffer::UploadTextureData(WeakPtr<Image2D> dstImage, const ImageCopyData& copyData)
	{
		VT_PROFILE_FUNCTION();
		
		auto& vkImage = dstImage->AsRef<VulkanImage2D>();

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

		const uint32_t index = GetCurrentCommandBufferIndex();
		vkCmdCopyBufferToImage(m_commandBuffers.at(index).commandBuffer, stagingAlloc->GetResourceHandle<VkBuffer>(), dstImage->GetHandle<VkImage>(), Utility::GetVkImageLayoutFromImageLayout(currentState.layout), static_cast<uint32_t>(copyRegions.size()), copyRegions.data());
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
					VT_ASSERT(false);
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

		m_hasTimestampSupport = GraphicsContext::GetPhysicalDevice()->AsRef<VulkanPhysicalGraphicsDevice>().GetProperties().limits.timestampComputeAndGraphics;
		if (m_hasTimestampSupport)
		{
			CreateQueryPools();
		}
	}

	void VulkanCommandBuffer::Release()
	{
		auto device = GraphicsContext::GetDevice();
		Vector<VkFence> fences{};
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

		m_timestampCounts = Vector<uint32_t>(3, 0u);
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
		VT_PROFILE_FUNCTION();

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

			const float nsTime = endTime > startTime ? (endTime - startTime) * GraphicsContext::GetPhysicalDevice()->AsRef<VulkanPhysicalGraphicsDevice>().GetProperties().limits.timestampPeriod : 0.f;
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
