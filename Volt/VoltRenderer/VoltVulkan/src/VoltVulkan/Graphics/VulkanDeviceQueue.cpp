#include "vkpch.h"
#include "VulkanDeviceQueue.h"

#include "VoltVulkan/Graphics/VulkanGraphicsDevice.h"
#include "VoltVulkan/Graphics/VulkanPhysicalGraphicsDevice.h"

#include <vulkan/vulkan.h>

namespace Volt
{
	VulkanDeviceQueue::VulkanDeviceQueue(const DeviceQueueCreateInfo& createInfo)
	{
		Ref<VulkanGraphicsDevice> graphicsDevice = createInfo.graphicsDevice->As<VulkanGraphicsDevice>();
		
		const auto physicalDevice = graphicsDevice->GetPhysicalDevice();
		auto physicalDevicePtr = physicalDevice.lock();

		const auto queueFamilies = physicalDevicePtr->GetQueueFamilies();

		uint32_t queueFamily = 0;

		switch (createInfo.queueType)
		{
			case QueueType::Compute: queueFamily = queueFamilies.computeFamilyQueueIndex; break;
			case QueueType::Graphics: queueFamily = queueFamilies.graphicsFamilyQueueIndex; break;
			case QueueType::TransferCopy: queueFamily = queueFamilies.transferFamilyQueueIndex; break;

			default: queueFamily = 0;  break;
		}

		vkGetDeviceQueue(graphicsDevice->GetHandle<VkDevice>(), queueFamily, 0, &m_queue);
	}

	VulkanDeviceQueue::~VulkanDeviceQueue()
	{
	}

	void VulkanDeviceQueue::WaitForQueue()
	{
		vkQueueWaitIdle(m_queue);
	}

	void VulkanDeviceQueue::Execute(const std::vector<Ref<CommandBuffer>>& commandBuffer)
	{
	}

	void* VulkanDeviceQueue::GetHandleImpl()
	{
		return m_queue;
	}
}
