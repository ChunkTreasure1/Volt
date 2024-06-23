#pragma once

#include "VoltD3D12/Common/ComPtr.h"
#include "VoltD3D12/Common/D3D12Fence.h"

#include <VoltRHI/Graphics/DeviceQueue.h>

struct ID3D12CommandQueue;
struct ID3D12Fence;

namespace Volt::RHI
{
	class D3D12GraphicsDevice;


	class D3D12DeviceQueue final : public DeviceQueue
	{
	public:
		D3D12DeviceQueue(const DeviceQueueCreateInfo& createInfo); 
		~D3D12DeviceQueue() override;

		void WaitForQueue() override;
		void Wait(D3D12Fence& fence);

		void Signal(D3D12Fence& fence, const size_t customID);

		void Execute(const DeviceQueueExecuteInfo& commandBuffer) override;

		ID3D12CommandQueue** GetAddesss() { return &m_commandQueue; }

	protected:
		void* GetHandleImpl() const override;

	private:
		void CreateCommandQueue(QueueType type);

		ComPtr<ID3D12CommandQueue> m_commandQueue;
		D3D12GraphicsDevice* m_device = nullptr;

		// this is tracked internally from the most recent fence.
		ComPtr<ID3D12Fence> m_currentFence;
		size_t m_currentFenceValue;
	};
}
