#pragma once

#include "VoltRHI/Graphics/GraphicsDevice.h"

struct ID3D12Device2;

namespace Volt::RHI
{
	struct GraphicsDeviceCreateInfo;

	class D3D12GraphicsDevice final : public GraphicsDevice
	{
	public:
		D3D12GraphicsDevice(const GraphicsDeviceCreateInfo& info);
		~D3D12GraphicsDevice() override;



	protected:
		void* GetHandleImpl() override;

	private:
		ID3D12Device2* m_device;
	};
}
