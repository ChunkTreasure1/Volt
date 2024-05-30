#pragma once

#include "VoltRHI/Graphics/GraphicsDevice.h"

struct ID3D12Device2;

namespace Volt::RHI
{
	struct GraphicsDeviceCreateInfo;

	class D3D12GraphicsDevice final : public GraphicsDevice
	{
	public:
		D3D12GraphicsDevice(const GraphicsDeviceCreateInfo& info);
		~D3D12GraphicsDevice() override;

		RefPtr<DeviceQueue> GetDeviceQueue(QueueType queueType) const override;

	protected:
		void* GetHandleImpl() const override;

	private:
		std::unordered_map<QueueType, RefPtr<DeviceQueue>> m_deviceQueues;
		ID3D12Device2* m_device;
	};
}
