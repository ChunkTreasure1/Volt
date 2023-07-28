#pragma once
#include "VoltRHI/Graphics/GraphicsDevice.h"

namespace Volt::RHI
{
	class MockGraphicsDevice final : public GraphicsDevice
	{
	public:
		MockGraphicsDevice(const GraphicsDeviceCreateInfo& deviceInfo);
		~MockGraphicsDevice() = default;

		void* GetHandleImpl() override;

	private:
		GraphicsDeviceCreateInfo m_graphicsDeviceInfo;

		void* m_tempDevice = nullptr;
	};
}

