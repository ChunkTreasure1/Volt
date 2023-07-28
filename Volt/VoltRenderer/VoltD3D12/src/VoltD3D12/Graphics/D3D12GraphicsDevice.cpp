#include "dxpch.h"
#include "D3D12GraphicsDevice.h"

#include "VoltD3D12/Graphics/D3D12PhysicalGraphicsDevice.h"

namespace Volt::RHI
{
	D3D12GraphicsDevice::D3D12GraphicsDevice(const GraphicsDeviceCreateInfo& info)
	{
		auto physicalDevice = info.physicalDevice->As<D3D12PhysicalGraphicsDevice>();

		VT_D3D12_CHECK(D3D12CreateDevice(physicalDevice->GetAdapter(), D3D_FEATURE_LEVEL_11_0, VT_D3D12_ID(m_device)));
	}

	D3D12GraphicsDevice::~D3D12GraphicsDevice()
	{
		VT_D3D12_DELETE(m_device);
	}

	void* D3D12GraphicsDevice::GetHandleImpl()
	{
		return m_device;
	}
}
