#pragma once

#include "VoltD3D12/Common/ComPtr.h"

#include <VoltRHI/Graphics/PhysicalGraphicsDevice.h>


struct IDXGIFactory4;
struct IDXGIAdapter4;

namespace Volt::RHI
{
	struct PhysicalDeviceCreateInfo;


	class D3D12PhysicalGraphicsDevice final : public PhysicalGraphicsDevice
	{
	public:
		D3D12PhysicalGraphicsDevice(const PhysicalDeviceCreateInfo& info);
		~D3D12PhysicalGraphicsDevice() override;

		VT_NODISCARD VT_INLINE std::string_view GetDeviceName() const override { return ""; }
		VT_NODISCARD VT_INLINE const DeviceVendor GetDeviceVendor() const override { return DeviceVendor::Unknown; }

	protected:
		void* GetHandleImpl() const override;

	private:
		VT_NODISCARD ComPtr<IDXGIAdapter4> FindValidAdapter();
		ComPtr<IDXGIAdapter4> m_adapter = nullptr;
	};
}
