#include "mkpch.h"
#include "MockGraphicsDevice.h"

namespace Volt
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
