#include "mkpch.h"
#include "MockDeviceQueue.h"

namespace Volt::RHI
{
	MockDeviceQueue::MockDeviceQueue(const DeviceQueueCreateInfo& createInfo)
	{
	}
	
	void* MockDeviceQueue::GetHandleImpl()
	{
		return nullptr;
	}
	
	void MockDeviceQueue::WaitForQueue()
	{
	}

	void MockDeviceQueue::Execute(const std::vector<Ref<CommandBuffer>>& commandBuffer)
	{
	}
}
