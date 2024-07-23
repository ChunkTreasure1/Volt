#include "vkpch.h"
#include "VulkanDeviceQueue.h"

#include "VoltVulkan/Common/VulkanCommon.h"

#include "VoltVulkan/Graphics/VulkanGraphicsDevice.h"
#include "VoltVulkan/Graphics/VulkanPhysicalGraphicsDevice.h"
#include "VoltVulkan/Buffers/VulkanCommandBuffer.h"
#include "VoltVulkan/Synchronization/VulkanSemaphore.h"

#include <vulkan/vulkan.h>

namespace Volt::RHI
{
	VulkanDeviceQueue::VulkanDeviceQueue(const DeviceQueueCreateInfo& createInfo)
	{
		VulkanGraphicsDevice& graphicsDevice = createInfo.graphicsDevice->AsRef<VulkanGraphicsDevice>();

		const auto physicalDevice = graphicsDevice.GetPhysicalDevice();

		const auto queueFamilies = physicalDevice->GetQueueFamilies();

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
		VT_PROFILE_FUNCTION();

		std::scoped_lock lock{ m_executeMutex };
		vkQueueWaitIdle(m_queue);
	}

	void VulkanDeviceQueue::Execute(const DeviceQueueExecuteInfo& executeInfo)
	{
		Vector<VkCommandBufferSubmitInfo> vulkanCommandBuffers;
		vulkanCommandBuffers.reserve(executeInfo.commandBuffers.size());

		Vector<VkSemaphoreSubmitInfo> signalSemaphoreInfos{};
		signalSemaphoreInfos.reserve(executeInfo.signalSemaphores.size());

		VkFence waitFence = nullptr;

		for (const auto& cmdBuffer : executeInfo.commandBuffers)
		{
			auto& info = vulkanCommandBuffers.emplace_back();
			info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_SUBMIT_INFO;
			info.pNext = nullptr;
			info.deviceMask = 0;
			info.commandBuffer = cmdBuffer->GetHandle<VkCommandBuffer>();
		}

		for (const auto& semaphore : executeInfo.signalSemaphores)
		{
			auto& info = signalSemaphoreInfos.emplace_back();
			info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO;
			info.pNext = nullptr;
			info.deviceIndex = 0;
			info.stageMask = VK_PIPELINE_STAGE_2_NONE;
			info.semaphore = semaphore->GetHandle<VkSemaphore>();;
			info.value = semaphore->GetValue();
		}

		waitFence = executeInfo.commandBuffers.front()->AsRef<VulkanCommandBuffer>().GetCurrentFence();

		VkSubmitInfo2 info{};
		info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO_2;
		info.commandBufferInfoCount = static_cast<uint32_t>(vulkanCommandBuffers.size());
		info.pCommandBufferInfos = vulkanCommandBuffers.data();
		info.signalSemaphoreInfoCount = static_cast<uint32_t>(signalSemaphoreInfos.size());
		info.pSignalSemaphoreInfos = signalSemaphoreInfos.data();

		{
			std::scoped_lock lock{ m_executeMutex };
 			VT_VK_CHECK(vkQueueSubmit2(m_queue, 1, &info, waitFence));
		}
	}

	void VulkanDeviceQueue::AquireLock()
	{
		m_executeMutex.lock();
	}

	void VulkanDeviceQueue::ReleaseLock()
	{
		m_executeMutex.unlock();
	}

	void* VulkanDeviceQueue::GetHandleImpl() const
	{
		return m_queue;
	}
}
