#include "mkpch.h"
#include "MockDeviceQueue.h"

namespace Volt
{
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