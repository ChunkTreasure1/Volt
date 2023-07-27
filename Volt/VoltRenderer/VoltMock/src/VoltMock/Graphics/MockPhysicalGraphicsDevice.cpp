#include "mkpch.h"
#include "MockPhysicalGraphicsDevice.h"

namespace Volt::RHI
{
	MockPhysicalGraphicsDevice::MockPhysicalGraphicsDevice(const PhysicalDeviceCreateInfo& createInfo)
	{
	}
	
	void* MockPhysicalGraphicsDevice::GetHandleImpl()
	{
		return nullptr;
	}
}
