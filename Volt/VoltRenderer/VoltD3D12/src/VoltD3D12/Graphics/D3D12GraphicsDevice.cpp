#include "dxpch.h"
#include "D3D12GraphicsDevice.h"

#include "VoltD3D12/Graphics/D3D12PhysicalGraphicsDevice.h"
#include "VoltD3D12/Graphics/D3D12DeviceQueue.h"

namespace Volt::RHI
{
	D3D12GraphicsDevice::D3D12GraphicsDevice(const GraphicsDeviceCreateInfo& info)
	{
		VT_D3D12_CHECK(D3D12CreateDevice(info.physicalDevice->GetHandle<IDXGIAdapter4*>(), D3D_FEATURE_LEVEL_11_0, VT_D3D12_ID(m_device)));

		m_deviceQueues[QueueType::Graphics] = RefPtr<D3D12DeviceQueue>::Create(DeviceQueueCreateInfo{ this, QueueType::Graphics });
		m_deviceQueues[QueueType::TransferCopy] = RefPtr<D3D12DeviceQueue>::Create(DeviceQueueCreateInfo{ this, QueueType::TransferCopy });
		m_deviceQueues[QueueType::Compute] = RefPtr<D3D12DeviceQueue>::Create(DeviceQueueCreateInfo{ this, QueueType::Compute });
	
		InitializeProperties();
	}

	D3D12GraphicsDevice::~D3D12GraphicsDevice()
	{
		m_deviceQueues[QueueType::Graphics].Reset();
		m_deviceQueues[QueueType::TransferCopy].Reset();
		m_deviceQueues[QueueType::Compute].Reset();
	}

	RefPtr<DeviceQueue> D3D12GraphicsDevice::GetDeviceQueue(QueueType queueType) const
	{
		return m_deviceQueues.at(queueType);
	}

	void* D3D12GraphicsDevice::GetHandleImpl() const
	{
		return m_device.Get();
	}

	void D3D12GraphicsDevice::InitializeProperties()
	{
		m_properties.rtvDescriptorSize = m_device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
		m_properties.dsvDescriptorSize = m_device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_DSV);
		m_properties.cbvSrvUavDescriptorSize = m_device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
		m_properties.samplerDescriptorSize = m_device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER);
	}
}
