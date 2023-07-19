#pragma once

#include "VoltD3D12/Common/D3D12Common.h"

#include "VoltRHI/Graphics/PhysicalGraphicsDevice.h"


namespace Volt
{
	struct PhysicalDeviceCreateInfo;


	class D3D12PhysicalGraphicsDevice final : public PhysicalGraphicsDevice
	{
	public:
		D3D12PhysicalGraphicsDevice(const PhysicalDeviceCreateInfo& info);
		~D3D12PhysicalGraphicsDevice() override;

		[[nodiscard]] WinRef<IDXGIFactory4>& GetFactory() { return m_factory; }
		[[nodiscard]] WinRef<IDXGIAdapter1>& GetAdapter() { return m_adapter; }

	protected:
		void* GetHandleImpl() override
		{
			return nullptr;
		}

	private:
		[[nodiscard]] bool FindValidAdapter();

		WinRef<IDXGIFactory4> m_factory;
		WinRef<IDXGIAdapter1> m_adapter;
	};
}
