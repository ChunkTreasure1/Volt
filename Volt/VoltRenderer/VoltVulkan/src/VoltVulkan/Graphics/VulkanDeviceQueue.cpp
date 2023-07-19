#include "vkpch.h"
#include "VulkanDeviceQueue.h"

#include "VoltVulkan/Common/VulkanCommon.h"

#include "VoltVulkan/Graphics/VulkanGraphicsDevice.h"
#include "VoltVulkan/Graphics/VulkanPhysicalGraphicsDevice.h"
#include "VoltVulkan/Buffers/VulkanCommandBuffer.h"

#include <vulkan/vulkan.h>

namespace Volt
{
	VulkanDeviceQueue::VulkanDeviceQueue(const DeviceQueueCreateInfo& createInfo)
	{
		VulkanGraphicsDevice& graphicsDevice = createInfo.graphicsDevice->AsRef<VulkanGraphicsDevice>();

		const auto physicalDevice = graphicsDevice.GetPhysicalDevice();
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

		vkGetDeviceQueue(graphicsDevice.GetHandle<VkDevice>(), queueFamily, 0, &m_queue);
	}

	VulkanDeviceQueue::~VulkanDeviceQueue()
	{
		m_queue = nullptr;
	}

	void VulkanDeviceQueue::WaitForQueue()
	{
		vkQueueWaitIdle(m_queue);
	}

	void VulkanDeviceQueue::Execute(const std::vector<Ref<CommandBuffer>>& commandBuffers)
	{
		std::vector<VkCommandBuffer> vulkanCommandBuffers;
		VkFence waitFence = nullptr;

		for (const auto& cmdBuffer : commandBuffers)
		{
			vulkanCommandBuffers.push_back(cmdBuffer->GetHandle<VkCommandBuffer>());
		}

		waitFence = commandBuffers.front()->AsRef<VulkanCommandBuffer>().GetCurrentFence();

		VkSubmitInfo info{};
		info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
		info.commandBufferCount = static_cast<uint32_t>(vulkanCommandBuffers.size());
		info.pCommandBuffers = vulkanCommandBuffers.data();

		{
			std::scoped_lock lock{ m_executeMutex };
			VT_VK_CHECK(vkQueueSubmit(m_queue, 1, &info, waitFence));
		}
	}

	void* VulkanDeviceQueue::GetHandleImpl()
	{
		return m_queue;
	}
}
