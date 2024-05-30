#include "dxpch.h"
#include "D3D12GraphicsDevice.h"

#include "VoltD3D12/Graphics/D3D12PhysicalGraphicsDevice.h"
#include "VoltD3D12/Graphics/D3D12DeviceQueue.h"

namespace Volt::RHI
{
	D3D12GraphicsDevice::D3D12GraphicsDevice(const GraphicsDeviceCreateInfo& info)
	{
		auto physicalDevice = info.physicalDevice->As<D3D12PhysicalGraphicsDevice>();

		VT_D3D12_CHECK(D3D12CreateDevice(physicalDevice->GetAdapter(), D3D_FEATURE_LEVEL_11_0, VT_D3D12_ID(m_device)));

		m_deviceQueues[QueueType::Graphics] = RefPtr<D3D12DeviceQueue>::Create(DeviceQueueCreateInfo{ this, QueueType::Graphics });
		m_deviceQueues[QueueType::TransferCopy] = RefPtr<D3D12DeviceQueue>::Create(DeviceQueueCreateInfo{ this, QueueType::TransferCopy });
		m_deviceQueues[QueueType::Compute] = RefPtr<D3D12DeviceQueue>::Create(DeviceQueueCreateInfo{ this, QueueType::Compute });
	}

	D3D12GraphicsDevice::~D3D12GraphicsDevice()
	{
		m_deviceQueues[QueueType::Graphics].Reset();
		m_deviceQueues[QueueType::TransferCopy].Reset();
		m_deviceQueues[QueueType::Compute].Reset();

		VT_D3D12_DELETE(m_device);
	}

	RefPtr<DeviceQueue> D3D12GraphicsDevice::GetDeviceQueue(QueueType queueType) const
	{
		return m_deviceQueues.at(queueType);
	}

	void* D3D12GraphicsDevice::GetHandleImpl() const
	{
		return m_device;
	}
}
