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
		D3D12CommandBuffer(const uint32_t count, QueueType queueType, bool swapchainTarget);
		~D3D12CommandBuffer() override;

		void* GetHandleImpl() override;
		void Begin() override;
		void End() override;
		void Execute() override;
		void ExecuteAndWait() override;
		void Draw(const uint32_t vertexCount, const uint32_t instanceCount, const uint32_t firstVertex, const uint32_t firstInstance) override;
		void DrawIndexed(const uint32_t indexCount, const uint32_t instanceCount, const uint32_t firstIndex, const uint32_t vertexOffset, const uint32_t firstInstance) override;

		void SetViewports(const std::vector<Viewport>& viewports) override;
		void SetScissors(const std::vector<Rect2D>& scissors) override;

		D3D12Fence& GetFenceData();
		D3D12CommandData& GetCommandData();

	private:
		void Create(const uint32_t count, QueueType queueType, bool swapchainTarget);

		void IncrementIndex();
		
		std::vector<std::pair<D3D12CommandData, D3D12Fence>> m_perInternalBufferData;
		uint32_t m_currentCommandBufferIndex = 0;
		bool m_isSwapchainTarget = false;
	};
}