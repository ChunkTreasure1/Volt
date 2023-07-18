#include "dxpch.h"
#include "D3D12GraphicsContext.h"

#include "VoltRHI/Graphics/PhysicalGraphicsDevice.h"
#include "VoltRHI/Graphics/GraphicsDevice.h"

namespace Volt
{
	D3D12GraphicsContext::D3D12GraphicsContext(const GraphicsContextCreateInfo& info)
	{
		Initalize(info);
	}

	D3D12GraphicsContext::~D3D12GraphicsContext()
	{
		Shutdown();
	}

	void* D3D12GraphicsContext::GetHandleImpl()
	{
		return nullptr;
	}

	void D3D12GraphicsContext::Initalize(const GraphicsContextCreateInfo& info)
	{
		m_physicalDevice = PhysicalGraphicsDevice::Create(info.physicalDeviceInfo);

		GraphicsDeviceCreateInfo deviceCreateInfo = {};
		deviceCreateInfo.physicalDevice = m_physicalDevice;

		m_device = GraphicsDevice::Create(deviceCreateInfo);
		
	}

	void D3D12GraphicsContext::Shutdown()
	{
	}
}
