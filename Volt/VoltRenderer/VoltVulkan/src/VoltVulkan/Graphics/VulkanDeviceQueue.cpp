#include "vkpch.h"
#include "VulkanDeviceQueue.h"

#include "VoltVulkan/Graphics/VulkanGraphicsDevice.h"

namespace Volt
{
	VulkanDeviceQueue::VulkanDeviceQueue(const DeviceQueueCreateInfo& createInfo)
	{
		Ref<VulkanGraphicsDevice> graphicsDevice = createInfo.graphicsDevice->As<VulkanGraphicsDevice>();
	
		
	}

	VulkanDeviceQueue::~VulkanDeviceQueue()
	{
	}

	void* VulkanDeviceQueue::GetHandleImpl()
	{
		return nullptr;
	}
}
