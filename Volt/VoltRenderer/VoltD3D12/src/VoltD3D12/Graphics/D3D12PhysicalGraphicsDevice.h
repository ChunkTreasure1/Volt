#pragma once
#include "VoltRHI/Graphics/PhysicalGraphicsDevice.h"


struct IDXGIFactory4;
struct IDXGIAdapter1;

namespace Volt::RHI
{
	struct PhysicalDeviceCreateInfo;


	class D3D12PhysicalGraphicsDevice final : public PhysicalGraphicsDevice
	{
	public:
		D3D12PhysicalGraphicsDevice(const PhysicalDeviceCreateInfo& info);
		~D3D12PhysicalGraphicsDevice() override;

		[[nodiscard]] inline std::string_view GetDeviceName() const override { return ""; }
		[[nodiscard]] inline const DeviceVendor GetDeviceVendor() const override { return DeviceVendor::Unknown; }

		[[nodiscard]] IDXGIFactory4* GetFactory() { return m_factory; }
		[[nodiscard]] IDXGIAdapter1* GetAdapter() { return m_adapter; }

	protected:
		void* GetHandleImpl() const override
		{
			return m_adapter;
		}

	private:
		[[nodiscard]] bool FindValidAdapter();

		IDXGIFactory4* m_factory;
		IDXGIAdapter1* m_adapter;
	};
}