#include "rhipch.h"
#include "DeviceQueue.h"

#include "VoltRHI/RHIProxy.h"

namespace Volt::RHI
{
	Ref<DeviceQueue> DeviceQueue::Create(const DeviceQueueCreateInfo& createInfo)
	{
		return RHIProxy::GetInstance().CreateDeviceQueue(createInfo);
	}
}
