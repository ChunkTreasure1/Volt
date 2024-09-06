#include "rhipch.h"

#include "RHIModule/Graphics/PhysicalGraphicsDevice.h"
#include "RHIModule/RHIProxy.h"

namespace Volt::RHI
{
	RefPtr<PhysicalGraphicsDevice> PhysicalGraphicsDevice::Create(const PhysicalDeviceCreateInfo& deviceInfo)
	{
		return RHIProxy::GetInstance().CreatePhysicalGraphicsDevice(deviceInfo);
	}
}
