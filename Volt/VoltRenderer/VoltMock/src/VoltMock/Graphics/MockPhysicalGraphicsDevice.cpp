#include "mkpch.h"
#include "MockPhysicalGraphicsDevice.h"

namespace Volt
{
	MockPhysicalGraphicsDevice::MockPhysicalGraphicsDevice(const PhysicalDeviceCreateInfo& createInfo)
	{
	}
	
	void* MockPhysicalGraphicsDevice::GetHandleImpl()
	{
		return nullptr;
	}
}
