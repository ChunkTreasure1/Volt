#pragma once

#include "VoltD3D12/Common/ComPtr.h"
#include "VoltD3D12/Descriptors/DescriptorCommon.h"

#include <VoltRHI/Synchronization/Semaphore.h>
#include <VoltRHI/Buffers/CommandBuffer.h>

struct ID3D12Fence;
struct ID3D12CommandAllocator;
struct ID3D12GraphicsCommandList6;

namespace Volt::RHI
{
	class D3D12DescriptorHeap;
	class D3D12CommandBuffer final : public CommandBuffer
	{
	public:
		D3D12CommandBuffer(const uint32_t count, QueueType queueType);
		~D3D12CommandBuffer() override;

		void Begin() override;
		void RestartAfterFlush() override;
		void End() override;
		void Execute() override;
		void ExecuteAndWait() override;
		void WaitForFences() override;
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
		void BindDescriptorTable(WeakPtr<BindlessDescriptorTable> descriptorTable) override;

		void BeginRendering(const RenderingInfo& renderingInfo) override;
		void EndRendering() override;

		void PushConstants(const void* data, const uint32_t size, const uint32_t offset) override;

		void ResourceBarrier(const Vector<ResourceBarrierInfo>& resourceBarriers) override;

		void BeginMarker(std::string_view markerLabel, const std::array<float, 4>& markerColor) override;
		void EndMarker() override;

		const uint32_t BeginTimestamp() override;
		void EndTimestamp(uint32_t timestampIndex) override;
		const float GetExecutionTime(uint32_t timestampIndex) const override;

		void ClearImage(WeakPtr<Image2D> image, std::array<float, 4> clearColor) override;
		void ClearBuffer(WeakPtr<StorageBuffer> buffer, const uint32_t value) override;

		void UpdateBuffer(WeakPtr<StorageBuffer> dstBuffer, const size_t dstOffset, const size_t dataSize, const void* data) override;
		void CopyBufferRegion(WeakPtr<Allocation> srcResource, const size_t srcOffset, WeakPtr<Allocation> dstResource, const size_t dstOffset, const size_t size) override;
		void CopyBufferToImage(WeakPtr<Allocation> srcBuffer, WeakPtr<Image2D> dstImage, const uint32_t width, const uint32_t height, const uint32_t mip /* = 0 */) override;
		void CopyImageToBuffer(WeakPtr<Image2D> srcImage, WeakPtr<Allocation> dstBuffer, const size_t dstOffset, const uint32_t width, const uint32_t height, const uint32_t mip) override;
		void CopyImage(WeakPtr<Image2D> srcImage, WeakPtr<Image2D> dstImage, const uint32_t width, const uint32_t height) override;

		void UploadTextureData(WeakPtr<Image2D> dstImage, const ImageCopyData& copyData) override;

		const uint32_t GetCurrentIndex() const override;
		const QueueType GetQueueType() const override;

		RefPtr<Semaphore> GetCurrentSemaphore() const { return m_commandLists.at(m_currentCommandListIndex).fence; }

	protected:
		void* GetHandleImpl() const override;

	private:
		friend class D3D12DescriptorTable;
		friend class D3D12BindlessDescriptorTable;

		void Invalidate();
		void Release();

		void BindPipelineInternal();

		const uint32_t GetCurrentCommandListIndex() const;
		D3D12DescriptorPointer CreateTempDescriptorPointer();

		struct CommandListData
		{
			ComPtr<ID3D12CommandAllocator> commandAllocator;
			ComPtr<ID3D12GraphicsCommandList7> commandList;
			RefPtr<Semaphore> fence;
		};

		Vector<CommandListData> m_commandLists;

		uint32_t m_commandListCount = 0;
		uint32_t m_currentCommandListIndex = 0;

		QueueType m_queueType;

		// Internal state
		WeakPtr<RenderPipeline> m_currentRenderPipeline;
		WeakPtr<ComputePipeline> m_currentComputePipeline;

		bool m_pipelineNeedsToBeBound = false;

		Vector<Vector<D3D12DescriptorPointer>> m_allocatedDescriptors;
		Scope<D3D12DescriptorHeap> m_descriptorHeap;
	};
}
