#pragma once

#include "VoltRHI/Graphics/DeviceQueue.h"

struct ID3D12Fence;
struct ID3D12CommandAllocator;
struct ID3D12CommandQueue;

namespace Volt
{
	class D3D12DeviceQueue final : public DeviceQueue
	{
	public:
		D3D12DeviceQueue(const DeviceQueueCreateInfo& createInfo);
		~D3D12DeviceQueue();

		void WaitForQueue() override;
		void Execute(const std::vector<Ref<CommandBuffer>>& commandBuffer) override;

	protected:
		void* GetHandleImpl() override;

	private:
		void CreateCommandQueue(QueueType type);
		void DestroyCommandQueue(QueueType type);

		inline static std::unordered_map<QueueType, std::pair<ID3D12CommandQueue*, uint32_t>> s_commandQueues;

		ID3D12CommandAllocator* m_commandAllocator;

		ID3D12Fence* m_fence;
		size_t m_fenceValue;
		size_t m_fenceStartValue;
		void* m_windowsFenceHandle;
	};
}
