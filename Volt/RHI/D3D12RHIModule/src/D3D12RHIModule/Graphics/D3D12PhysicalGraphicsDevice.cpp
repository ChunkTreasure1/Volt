#include "dxpch.h"
#include "D3D12PhysicalGraphicsDevice.h"

#include <CoreUtilities/StringUtility.h>

namespace Volt::RHI
{
	D3D12PhysicalGraphicsDevice::D3D12PhysicalGraphicsDevice(const PhysicalDeviceCreateInfo& info)
	{
		m_adapter = FindValidAdapter();
		if (m_adapter)
		{
			DXGI_ADAPTER_DESC1 desc{};
			m_adapter->GetDesc1(&desc);
		
			m_name = Utility::ToString(std::wstring(desc.Description));
			m_vendor = VendorIDToVendor(desc.VendorId);
		}
	}

	D3D12PhysicalGraphicsDevice::~D3D12PhysicalGraphicsDevice()
	{
	}

	void* D3D12PhysicalGraphicsDevice::GetHandleImpl() const
	{
		return m_adapter.Get();
	}

	ComPtr<IDXGIAdapter4> D3D12PhysicalGraphicsDevice::FindValidAdapter()
	{
		uint32_t factoryFlags = 0;
#ifdef VT_ENABLE_VALIDATION
		factoryFlags = DXGI_CREATE_FACTORY_DEBUG;
#endif

		ComPtr<IDXGIFactory4> factory;
		VT_D3D12_CHECK(CreateDXGIFactory2(factoryFlags, VT_D3D12_ID(factory)));

		ComPtr<IDXGIAdapter1> adapter1;
		ComPtr<IDXGIAdapter4> adapter4;

		size_t maxDedicatedMemory = 0;
		for (uint32_t i = 0; factory->EnumAdapters1(i, &adapter1) != DXGI_ERROR_NOT_FOUND; i++)
		{
			DXGI_ADAPTER_DESC1 desc1{};
			adapter1->GetDesc1(&desc1);

			if ((desc1.Flags & DXGI_ADAPTER_FLAG_SOFTWARE) == 0 &&
				SUCCEEDED(D3D12CreateDevice(adapter1.Get(), D3D_FEATURE_LEVEL_11_0, __uuidof(ID3D12Device), nullptr)) &&
				desc1.DedicatedVideoMemory > maxDedicatedMemory)
			{
				maxDedicatedMemory = desc1.DedicatedVideoMemory;
				VT_D3D12_CHECK(adapter1.As(&adapter4));
			}
		}

		return adapter4;
	}
}
