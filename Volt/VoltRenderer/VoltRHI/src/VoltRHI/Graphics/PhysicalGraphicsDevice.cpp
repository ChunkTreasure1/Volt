#include "rhipch.h"
#include "PhysicalGraphicsDevice.h"

#include "VoltRHI/RHIProxy.h"

namespace Volt::RHI
{
	RefPtr<PhysicalGraphicsDevice> PhysicalGraphicsDevice::Create(const PhysicalDeviceCreateInfo& deviceInfo)
	{
		return RHIProxy::GetInstance().CreatePhysicalGraphicsDevice(deviceInfo);
	}
}
