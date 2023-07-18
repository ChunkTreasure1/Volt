#pragma once
#include "VoltD3D12/Common/D3D12Common.h"

#include "VoltRHI/Graphics/GraphicsDevice.h"

namespace Volt
{
	struct GraphicsDeviceCreateInfo;

	class D3D12GraphicsDevice final : public GraphicsDevice
	{
	public:
		D3D12GraphicsDevice(const GraphicsDeviceCreateInfo& info);
		~D3D12GraphicsDevice();

	protected:
		void* GetHandleImpl() override;

	private:
		WinRef<ID3D12Device2> m_device;
	};
}
