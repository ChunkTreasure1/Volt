#pragma once

#include "D3D12RHIModule/Common/ComPtr.h"
#include "D3D12RHIModule/Descriptors/DescriptorCommon.h"

#include <RHIModule/Synchronization/Semaphore.h>
#include <RHIModule/Buffers/CommandBuffer.h>

struct ID3D12Fence;
struct ID3D12CommandAllocator;
struct ID3D12GraphicsCommandList6;

namespace Volt::RHI
{
	class D3D12DescriptorHeap;
	class D3D12CommandBuffer final : public CommandBuffer
	{
	public:
		D3D12CommandBuffer(QueueType queueType);
		~D3D12CommandBuffer() override;

		void Begin() override;
		void End() override;

		void Flush(RefPtr<Fence> fence) override;
		void Execute() override;
		void ExecuteAndWait() override;
		void WaitForFence() override;
		void SetEvent(WeakPtr<Event> event) override;

		void Draw(const uint32_t vertexCount, const uint32_t instanceCount, const uint32_t firstVertex, const uint32_t firstInstance) override;
		void DrawIndexed(const uint32_t indexCount, const uint32_t instanceCount, const uint32_t firstIndex, const uint32_t vertexOffset, const uint32_t firstInstance) override;
		void DrawIndexedIndirect(WeakPtr<StorageBuffer> commandsBuffer, const size_t offset, const uint32_t drawCount, const uint32_t stride) override;
		void DrawIndirect(WeakPtr<StorageBuffer> commandsBuffer, const size_t offset, const uint32_t drawCount, const uint32_t stride) override;
		void DrawIndirectCount(WeakPtr<StorageBuffer> commandsBuffer, const size_t offset, WeakPtr<StorageBuffer> countBuffer, const size_t countBufferOffset, const uint32_t maxDrawCount, const uint32_t stride) override;
		void DrawIndexedIndirectCount(WeakPtr<StorageBuffer> commandsBuffer, const size_t offset, WeakPtr<StorageBuffer> countBuffer, const size_t countBufferOffset, const uint32_t maxDrawCount, const uint32_t stride) override;

		void DispatchMeshTasks(const uint32_t groupCountX, const uint32_t groupCountY, const uint32_t groupCountZ) override;
		void DispatchMeshTasksIndirect(WeakPtr<StorageBuffer> commandsBuffer, const size_t offset, const uint32_t drawCount, const uint32_t stride) override;
		void DispatchMeshTasksIndirectCount(WeakPtr<StorageBuffer> commandsBuffer, const size_t offset, WeakPtr<StorageBuffer> countBuffer, const size_t countBufferOffset, const uint32_t maxDrawCount, const uint32_t stride) override;

		void Dispatch(const uint32_t groupCountX, const uint32_t groupCountY, const uint32_t groupCountZ) override;
		void DispatchIndirect(WeakPtr<StorageBuffer> commandsBuffer, const size_t offset) override;

		void SetViewports(const StackVector<Viewport, MAX_VIEWPORT_COUNT>& viewports) override;
		void SetScissors(const StackVector<Rect2D, MAX_VIEWPORT_COUNT>& scissors) override;

		void BindPipeline(WeakPtr<RenderPipeline> pipeline) override;
		void BindPipeline(WeakPtr<ComputePipeline> pipeline) override;

		void BindVertexBuffers(const StackVector<WeakPtr<VertexBuffer>, RHI::MAX_VERTEX_BUFFER_COUNT>& vertexBuffers, const uint32_t firstBinding) override;
		void BindVertexBuffers(const StackVector<WeakPtr<StorageBuffer>, RHI::MAX_VERTEX_BUFFER_COUNT>& vertexBuffers, const uint32_t firstBinding) override;
		void BindIndexBuffer(WeakPtr<IndexBuffer> indexBuffer) override;
		void BindIndexBuffer(WeakPtr<StorageBuffer> indexBuffer) override;

		void BindDescriptorTable(WeakPtr<DescriptorTable> descriptorTable) override;
		void BindDescriptorTable(WeakPtr<BindlessDescriptorTable> descriptorTable, WeakPtr<UniformBuffer> constantsBuffer, const uint32_t offsetIndex, const uint32_t stride) override;

		void BeginRendering(const RenderingInfo& renderingInfo) override;
		void EndRendering() override;

		void PushConstants(const void* data, const uint32_t size, const uint32_t offset) override;

		void ResourceBarrier(const Vector<ResourceBarrierInfo>& resourceBarriers) override;

		void BeginMarker(std::string_view markerLabel, const std::array<float, 4>& markerColor) override;
		void EndMarker() override;

		const uint32_t BeginTimestamp() override;
		void EndTimestamp(uint32_t timestampIndex) override;
		const float GetExecutionTime(uint32_t timestampIndex) const override;

		void ClearImage(WeakPtr<Image> image, std::array<float, 4> clearColor) override;
		void ClearBuffer(WeakPtr<StorageBuffer> buffer, const uint32_t value) override;

		void UpdateBuffer(WeakPtr<StorageBuffer> dstBuffer, const size_t dstOffset, const size_t dataSize, const void* data) override;
		void CopyBufferRegion(WeakPtr<Allocation> srcResource, const size_t srcOffset, WeakPtr<Allocation> dstResource, const size_t dstOffset, const size_t size) override;
		void CopyBufferToImage(WeakPtr<Allocation> srcBuffer, WeakPtr<Image> dstImage, const uint32_t width, const uint32_t height, const uint32_t depth, const uint32_t mip /* = 0 */) override;
		void CopyImageToBuffer(WeakPtr<Image> srcImage, WeakPtr<Allocation> dstBuffer, const size_t dstOffset, const uint32_t width, const uint32_t height, const uint32_t depth, const uint32_t mip) override;
		void CopyImage(WeakPtr<Image> srcImage, WeakPtr<Image> dstImage, const uint32_t width, const uint32_t height, const uint32_t depth) override;

		void UploadTextureData(WeakPtr<Image> dstImage, const ImageCopyData& copyData) override;

		RefPtr<Semaphore> GetSemaphore() const { return m_commandListData.fence; }

		const QueueType GetQueueType() const override;
		const CommandBufferLevel GetCommandBufferLevel() const override;
		const WeakPtr<Fence> GetFence() const override;

		RefPtr<CommandBuffer> CreateSecondaryCommandBuffer() const override;
		void ExecuteSecondaryCommandBuffer(RefPtr<CommandBuffer> commandBuffer) const override;
		void ExecuteSecondaryCommandBuffers(Vector<RefPtr<CommandBuffer>> commandBuffers) const override;

	protected:
		void* GetHandleImpl() const override;

	private:
		friend class D3D12DescriptorTable;
		friend class D3D12BindlessDescriptorTable;

		void Invalidate();
		void Release();

		void BindPipelineInternal();

		D3D12DescriptorPointer CreateTempDescriptorPointer();

		struct CommandListData
		{
			ComPtr<ID3D12CommandAllocator> commandAllocator;
			ComPtr<ID3D12GraphicsCommandList7> commandList;
			RefPtr<Semaphore> fence;
		};

		CommandListData m_commandListData;
		QueueType m_queueType;

		// Internal state
		WeakPtr<RenderPipeline> m_currentRenderPipeline;
		WeakPtr<ComputePipeline> m_currentComputePipeline;

		bool m_pipelineNeedsToBeBound = false;

		Vector<D3D12DescriptorPointer> m_allocatedDescriptors;
		Scope<D3D12DescriptorHeap> m_descriptorHeap;
	};
}
