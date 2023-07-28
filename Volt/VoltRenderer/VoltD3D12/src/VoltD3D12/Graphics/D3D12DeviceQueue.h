#pragma once

#include "VoltRHI/Graphics/DeviceQueue.h"

struct ID3D12CommandQueue;
struct ID3D12Fence;

namespace Volt::RHI
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

		ID3D12CommandQueue* m_commandQueue;

		// this is tracked internally from the most recent fence.
		ID3D12Fence* m_currentFence;
		size_t m_currentFenceValue;
	};
}
