#include "mkpch.h"
#include "MockGraphicsDevice.h"

namespace Volt::RHI
{
	MockGraphicsDevice::MockGraphicsDevice(const GraphicsDeviceCreateInfo& deviceInfo)
	{
		m_graphicsDeviceInfo = deviceInfo;
	}

	void* MockGraphicsDevice::GetHandleImpl()
	{
		return m_tempDevice;
	}
}
