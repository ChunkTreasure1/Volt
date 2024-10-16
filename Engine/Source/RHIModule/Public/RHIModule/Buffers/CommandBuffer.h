#pragma once

#include "RHIModule/Core/RHIInterface.h"
#include "RHIModule/Core/RHICommon.h"
#include "RHIModule/Images/ImageView.h"

#include "RHIModule/Descriptors/BindlessDescriptorTable.h"
#include "RHIModule/Descriptors/DescriptorTable.h"

#include <CoreUtilities/Pointers/WeakPtr.h>
#include <CoreUtilities/Containers/StackVector.h>
#include <CoreUtilities/Containers/Vector.h>

#include <span>

namespace Volt::RHI
{
	class RenderPipeline;
	class ComputePipeline;

	class VertexBuffer;
	class IndexBuffer;

	class Image;
	class StorageBuffer;
	class Allocation;
	class Swapchain;

	class Event;
	class Fence;

	enum class CommandBufferLevel
	{
		Primary,
		Secondary
	};

	class VTRHI_API CommandBuffer : public RHIInterface
	{
	public:
		VT_DELETE_COPY_MOVE(CommandBuffer);
		~CommandBuffer() override = default;

		virtual void Begin() = 0;
		virtual void End() = 0;

		virtual void Flush(RefPtr<Fence> fence) = 0;
		virtual void Execute() = 0;
		virtual void ExecuteAndWait() = 0;
		virtual void WaitForFence() = 0;

		virtual void SetEvent(WeakPtr<Event> event) = 0;

		virtual void Draw(const uint32_t vertexCount, const uint32_t instanceCount, const uint32_t firstVertex, const uint32_t firstInstance) = 0;
		virtual void DrawIndexed(const uint32_t indexCount, const uint32_t instanceCount, const uint32_t firstIndex, const uint32_t vertexOffset, const uint32_t firstInstance) = 0;
		virtual void DrawIndexedIndirect(WeakPtr<StorageBuffer> commandsBuffer, const size_t offset, const uint32_t maxDrawCount, const uint32_t stride) = 0;
		virtual void DrawIndirect(WeakPtr<StorageBuffer> commandsBuffer, const size_t offset, const uint32_t maxDrawCount, const uint32_t stride) = 0;
		virtual void DrawIndexedIndirectCount(WeakPtr<StorageBuffer> commandsBuffer, const size_t offset, WeakPtr<StorageBuffer> countBuffer, const size_t countBufferOffset, const uint32_t maxDrawCount, const uint32_t stride) = 0;
		virtual void DrawIndirectCount(WeakPtr<StorageBuffer> commandsBuffer, const size_t offset, WeakPtr<StorageBuffer> countBuffer, const size_t countBufferOffset, const uint32_t maxDrawCount, const uint32_t stride) = 0;

		virtual void Dispatch(const uint32_t groupCountX, const uint32_t groupCountY, const uint32_t groupCountZ) = 0;
		virtual void DispatchIndirect(WeakPtr<StorageBuffer> commandsBuffer, const size_t offset) = 0;

		virtual void DispatchMeshTasks(const uint32_t groupCountX, const uint32_t groupCountY, const uint32_t groupCountZ) = 0;
		virtual void DispatchMeshTasksIndirect(WeakPtr<StorageBuffer> commandsBuffer, const size_t offset, const uint32_t drawCount, const uint32_t stride) = 0;
		virtual void DispatchMeshTasksIndirectCount(WeakPtr<StorageBuffer> commandsBuffer, const size_t offset, WeakPtr<StorageBuffer> countBuffer, const size_t countBufferOffset, const uint32_t maxDrawCount, const uint32_t stride) = 0;

		virtual void SetViewports(const StackVector<Viewport, MAX_VIEWPORT_COUNT>& viewports) = 0;
		virtual void SetScissors(const StackVector<Rect2D, MAX_VIEWPORT_COUNT>& scissors) = 0;

		virtual void BindPipeline(WeakPtr<RenderPipeline> pipeline) = 0;
		virtual void BindPipeline(WeakPtr<ComputePipeline> pipeline) = 0;
		virtual void BindVertexBuffers(const StackVector<WeakPtr<VertexBuffer>, MAX_VERTEX_BUFFER_COUNT>& vertexBuffers, const uint32_t firstBinding) = 0;
		virtual void BindVertexBuffers(const StackVector<WeakPtr<StorageBuffer>, MAX_VERTEX_BUFFER_COUNT>& vertexBuffers, const uint32_t firstBinding) = 0;
		virtual void BindIndexBuffer(WeakPtr<IndexBuffer> indexBuffer) = 0;
		virtual void BindIndexBuffer(WeakPtr<StorageBuffer> indexBuffer) = 0;

		virtual void BindDescriptorTable(WeakPtr<DescriptorTable> descriptorTable) = 0;
		virtual void BindDescriptorTable(WeakPtr<BindlessDescriptorTable> descriptorTable, WeakPtr<UniformBuffer> constantsBuffer, const uint32_t offsetIndex, const uint32_t stride) = 0;

		virtual void BeginRendering(const RenderingInfo& renderingInfo) = 0;
		virtual void EndRendering() = 0;

		virtual void PushConstants(const void* data, const uint32_t size, const uint32_t offset) = 0;

		virtual void ResourceBarrier(const Vector<ResourceBarrierInfo>& resourceBarriers) = 0;

		virtual void BeginMarker(std::string_view markerLabel, const std::array<float, 4>& markerColor) = 0;
		virtual void EndMarker() = 0;

		virtual const uint32_t BeginTimestamp() = 0;
		virtual void EndTimestamp(uint32_t timestampIndex) = 0;
		virtual const float GetExecutionTime(uint32_t timestampIndex) const = 0;

		virtual void ClearImage(WeakPtr<Image> image, std::array<float, 4> clearColor) = 0;
		virtual void ClearBuffer(WeakPtr<StorageBuffer> buffer, const uint32_t value) = 0;

		virtual void UpdateBuffer(WeakPtr<StorageBuffer> dstBuffer, const size_t dstOffset, const size_t dataSize, const void* data) = 0;
		virtual void CopyBufferRegion(WeakPtr<Allocation> srcResource, const size_t srcOffset, WeakPtr<Allocation> dstResource, const size_t dstOffset, const size_t size) = 0;
		virtual void CopyBufferToImage(WeakPtr<Allocation> srcBuffer, WeakPtr<Image> dstImage, const uint32_t width, const uint32_t height, const uint32_t depth, const uint32_t mip = 0) = 0;
		virtual void CopyImageToBuffer(WeakPtr<Image> srcImage, WeakPtr<Allocation> dstBuffer, const size_t dstOffset, const uint32_t width, const uint32_t height, const uint32_t depth, const uint32_t mip) = 0;
		virtual void CopyImage(WeakPtr<Image> srcImage, WeakPtr<Image> dstImage, const uint32_t width, const uint32_t height, const uint32_t depth) = 0;

		virtual void UploadTextureData(WeakPtr<Image> dstImage, const ImageCopyData& copyData) = 0;

		virtual const QueueType GetQueueType() const = 0;
		virtual const CommandBufferLevel GetCommandBufferLevel() const = 0;
		virtual const WeakPtr<Fence> GetFence() const = 0;

		virtual RefPtr<CommandBuffer> CreateSecondaryCommandBuffer() const = 0;
		virtual void ExecuteSecondaryCommandBuffer(RefPtr<CommandBuffer> commandBuffer) const = 0;
		virtual void ExecuteSecondaryCommandBuffers(Vector<RefPtr<CommandBuffer>> commandBuffers) const = 0;

		static RefPtr<CommandBuffer> Create(QueueType queueType);
		static RefPtr<CommandBuffer> Create();

	protected:
		CommandBuffer() = default;
	};
}
