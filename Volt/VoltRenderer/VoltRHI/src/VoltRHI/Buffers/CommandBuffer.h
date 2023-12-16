#pragma once

#include "VoltRHI/Core/RHIInterface.h"
#include "VoltRHI/Core/RHICommon.h"

#include <span>

namespace Volt::RHI
{
	class RenderPipeline;
	class ComputePipeline;
	class ImageView;

	class VertexBuffer;
	class IndexBuffer;

	class DescriptorTable;

	class Image2D;
	class StorageBuffer;
	class Allocation;

	class CommandBuffer : public RHIInterface
	{
	public:
		VT_DELETE_COMMON_OPERATORS(CommandBuffer);
		~CommandBuffer() override = default;

		virtual void Begin() = 0;
		virtual void End() = 0;
		virtual void Execute() = 0;
		virtual void ExecuteAndWait() = 0;
		virtual void WaitForLastFence() = 0;
		virtual void WaitForFences() = 0;

		virtual void Draw(const uint32_t vertexCount, const uint32_t instanceCount, const uint32_t firstVertex, const uint32_t firstInstance) = 0;
		virtual void DrawIndexed(const uint32_t indexCount, const uint32_t instanceCount, const uint32_t firstIndex, const uint32_t vertexOffset, const uint32_t firstInstance) = 0;
		virtual void DrawIndexedIndirect(Ref<StorageBuffer> commandsBuffer, const size_t offset, const uint32_t maxDrawCount, const uint32_t stride) = 0;
		virtual void DrawIndirect(Ref<StorageBuffer> commandsBuffer, const size_t offset, const uint32_t maxDrawCount, const uint32_t stride) = 0;
		virtual void DrawIndexedIndirectCount(Ref<StorageBuffer> commandsBuffer, const size_t offset, Ref<StorageBuffer> countBuffer, const size_t countBufferOffset, const uint32_t maxDrawCount, const uint32_t stride) = 0;
		virtual void DrawIndirectCount(Ref<StorageBuffer> commandsBuffer, const size_t offset, Ref<StorageBuffer> countBuffer, const size_t countBufferOffset, const uint32_t maxDrawCount, const uint32_t stride) = 0;

		virtual void Dispatch(const uint32_t groupCountX, const uint32_t groupCountY, const uint32_t groupCountZ) = 0;
		virtual void DispatchIndirect(Ref<StorageBuffer> commandsBuffer, const size_t offset) = 0;

		virtual void DispatchMeshTasks(const uint32_t groupCountX, const uint32_t groupCountY, const uint32_t groupCountZ) = 0;
		virtual void DispatchMeshTasksIndirect(Ref<StorageBuffer> commandsBuffer, const size_t offset, const uint32_t drawCount, const uint32_t stride) = 0;

		virtual void SetViewports(const std::vector<Viewport>& viewports) = 0;
		virtual void SetScissors(const std::vector<Rect2D>& scissors) = 0;

		virtual void BindPipeline(Ref<RenderPipeline> pipeline) = 0;
		virtual void BindPipeline(Ref<ComputePipeline> pipeline) = 0;
		virtual void BindVertexBuffers(const std::vector<Ref<VertexBuffer>>& vertexBuffers, const uint32_t firstBinding) = 0;
		virtual void BindIndexBuffer(Ref<IndexBuffer> indexBuffer) = 0;
		virtual void BindDescriptorTable(Ref<DescriptorTable> descriptorTable) = 0;

		virtual void BeginRendering(const RenderingInfo& renderingInfo) = 0;
		virtual void EndRendering() = 0;

		virtual void PushConstants(const void* data, const uint32_t size, const uint32_t offset) = 0;

		virtual void ResourceBarrier(const std::vector<ResourceBarrierInfo>& resourceBarriers) = 0;

		virtual void BeginMarker(std::string_view markerLabel, const std::array<float, 4>& markerColor) = 0;
		virtual void EndMarker() = 0;

		virtual const uint32_t BeginTimestamp() = 0;
		virtual void EndTimestamp(uint32_t timestampIndex) = 0;
		virtual const float GetExecutionTime(uint32_t timestampIndex) const = 0;

		virtual void CopyImageToBackBuffer(Ref<Image2D> srcImage) = 0;
		virtual void ClearImage(Ref<Image2D> image, std::array<float, 4> clearColor) = 0;
		virtual void ClearBuffer(Ref<StorageBuffer> buffer, const uint32_t value) = 0;

		virtual void CopyBufferRegion(Ref<Allocation> srcResource, const size_t srcOffset, Ref<Allocation> dstResource, const size_t dstOffset, const size_t size) = 0;
		virtual void CopyBufferToImage(Ref<Allocation> srcBuffer, Ref<Image2D> dstImage, const uint32_t width, const uint32_t height, const uint32_t mip = 0) = 0;
		virtual void CopyImage(Ref<Image2D> srcImage, Ref<Image2D> dstImage, const uint32_t width, const uint32_t height) = 0;

		virtual const uint32_t GetCurrentIndex() const = 0;

		inline const QueueType GetQueueType() const { return m_queueType; }

		static Ref<CommandBuffer> Create(const uint32_t count, QueueType queueType = QueueType::Graphics, bool swapchainTarget = false);
		static Ref<CommandBuffer> Create();

	protected:
		CommandBuffer(QueueType queueType);
	
		QueueType m_queueType = QueueType::Graphics;
	};
}
