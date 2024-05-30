#pragma once
#include "VoltRHI/Buffers/CommandBuffer.h"

#include "VoltD3D12/Common/D3D12Fence.h"

struct ID3D12Fence;
struct ID3D12CommandAllocator;
struct ID3D12GraphicsCommandList1;

namespace Volt::RHI
{
	struct D3D12CommandData
	{
		ID3D12CommandAllocator* commandAllocator;
		ID3D12GraphicsCommandList1* commandList;
	};

	class D3D12CommandBuffer final : public CommandBuffer
	{
	public:
		D3D12CommandBuffer(const uint32_t count, QueueType queueType);
		D3D12CommandBuffer(Weak<Swapchain> swapchain);
		~D3D12CommandBuffer() override;

		void* GetHandleImpl() const override;
		void Begin() override;
		void RestartAfterFlush() override;
		void End() override;
		void Execute() override;
		void ExecuteAndWait() override;
		void WaitForLastFence() override;
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

		void SetViewports(const std::vector<Viewport>& viewports) override;
		void SetScissors(const std::vector<Rect2D>& scissors) override;

		void BindPipeline(WeakPtr<RenderPipeline> pipeline) override;
		void BindPipeline(WeakPtr<ComputePipeline> pipeline) override;

		void BindVertexBuffers(const std::vector<WeakPtr<VertexBuffer>>& vertexBuffers, const uint32_t firstBinding) override;
		void BindIndexBuffer(WeakPtr<IndexBuffer> indexBuffer) override;
		void BindIndexBuffer(WeakPtr<StorageBuffer> indexBuffer) override;
		void BindDescriptorTable(WeakPtr<DescriptorTable> descriptorTable) override;

		void BeginRendering(const RenderingInfo& renderingInfo) override;
		void EndRendering() override;

		void PushConstants(const void* data, const uint32_t size, const uint32_t offset) override;

		void ResourceBarrier(const std::vector<ResourceBarrierInfo>& resourceBarriers) override;

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

		const uint32_t GetCurrentIndex() const override;
		const QueueType GetQueueType() const override;

		D3D12Fence& GetFenceData();
		D3D12CommandData& GetCommandData();

	private:
		void Create(const uint32_t count, QueueType queueType, bool swapchainTarget);

		void IncrementIndex();

		uint32_t m_amountOfTargetsbound;

		std::vector<std::pair<D3D12CommandData, D3D12Fence>> m_perInternalBufferData;
		uint32_t m_currentCommandBufferIndex = 0;
		bool m_isSwapchainTarget = false;
		QueueType m_queueType;
	};
}
