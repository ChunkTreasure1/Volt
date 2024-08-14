#include "rhipch.h"
#include "DeviceQueue.h"

#include "RHIModule/RHIProxy.h"

namespace Volt::RHI
{
	RefPtr<DeviceQueue> DeviceQueue::Create(const DeviceQueueCreateInfo& createInfo)
	{
		return RHIProxy::GetInstance().CreateDeviceQueue(createInfo);
	}
}
