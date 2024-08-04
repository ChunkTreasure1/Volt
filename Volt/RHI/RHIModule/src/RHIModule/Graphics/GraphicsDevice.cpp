#include "rhipch.h"
#include "GraphicsDevice.h"

#include "RHIModule/RHIProxy.h"

namespace Volt::RHI
{
	RefPtr<GraphicsDevice> GraphicsDevice::Create(const GraphicsDeviceCreateInfo& deviceInfo)
	{
		return RHIProxy::GetInstance().CreateGraphicsDevice(deviceInfo);
	}

	GraphicsDevice::GraphicsDevice()
	{
	}
}
