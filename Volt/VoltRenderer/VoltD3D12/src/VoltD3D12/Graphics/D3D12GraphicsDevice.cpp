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

		m_deviceQueues[QueueType::Graphics] = CreateRefRHI<D3D12DeviceQueue>(DeviceQueueCreateInfo{ this, QueueType::Graphics });
		m_deviceQueues[QueueType::TransferCopy] = CreateRefRHI<D3D12DeviceQueue>(DeviceQueueCreateInfo{ this, QueueType::TransferCopy });
		m_deviceQueues[QueueType::Compute] = CreateRefRHI<D3D12DeviceQueue>(DeviceQueueCreateInfo{ this, QueueType::Compute });
	}

	D3D12GraphicsDevice::~D3D12GraphicsDevice()
	{
		m_deviceQueues[QueueType::Graphics].reset();
		m_deviceQueues[QueueType::TransferCopy].reset();
		m_deviceQueues[QueueType::Compute].reset();

		VT_D3D12_DELETE(m_device);
	}

	void* D3D12GraphicsDevice::GetHandleImpl()
	{
		return m_device;
	}
}
