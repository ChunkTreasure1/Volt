#pragma once

#include "D3D12RHIModule/Common/ComPtr.h"

#include <RHIModule/Graphics/DeviceQueue.h>

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
		void Execute(const DeviceQueueExecuteInfo& commandBuffer) override;

		ID3D12CommandQueue** GetAddesss() { return &m_commandQueue; }

	protected:
		void* GetHandleImpl() const override;

	private:
		void CreateCommandQueue(QueueType type);

		ComPtr<ID3D12CommandQueue> m_commandQueue;
		D3D12GraphicsDevice* m_device = nullptr;
	};
}
