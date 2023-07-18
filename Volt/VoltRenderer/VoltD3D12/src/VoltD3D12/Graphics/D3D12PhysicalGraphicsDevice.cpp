#include "dxpch.h"
#include "D3D12PhysicalGraphicsDevice.h"

namespace Volt
{
	D3D12PhysicalGraphicsDevice::D3D12PhysicalGraphicsDevice(const PhysicalDeviceCreateInfo& info)
	{
		if (!FindValidAdapter())
		{
			int32_t crash = 3;
			(void)crash;
		}
	}

	D3D12PhysicalGraphicsDevice::~D3D12PhysicalGraphicsDevice()
	{
		m_factory->Release();
	}

	bool D3D12PhysicalGraphicsDevice::FindValidAdapter()
	{
		uint32_t adapterIndex = 0;
		VT_D3D12_CHECK(CreateDXGIFactory1(VT_D3D12_ID(m_factory)));

		while (m_factory->EnumAdapters1(adapterIndex, m_adapter.GetAddressOf()) != DXGI_ERROR_NOT_FOUND)
		{
			DXGI_ADAPTER_DESC1 desc;
			m_adapter->GetDesc1(&desc);

			if (desc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE)
			{
				// we dont want a software device
				adapterIndex++;
				continue;
			}

			auto hr = D3D12CreateDevice(m_adapter.Get(), D3D_FEATURE_LEVEL_11_0, __uuidof(ID3D12Device), nullptr);
			if (SUCCEEDED(hr))
			{
				return true;
			}

			adapterIndex++;
		}

		return false;
	}
}
