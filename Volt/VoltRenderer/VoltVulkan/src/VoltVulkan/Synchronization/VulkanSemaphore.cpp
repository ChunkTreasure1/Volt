#include "vkpch.h"
#include "VulkanSemaphore.h"

#include "VoltVulkan/Common/VulkanCommon.h"

#include <VoltRHI/Graphics/GraphicsContext.h>
#include <VoltRHI/Graphics/GraphicsDevice.h>

#include <vulkan/vulkan.h>

namespace Volt::RHI
{
	VulkanSemaphore::VulkanSemaphore(const SemaphoreCreateInfo& info)
	{
		VkSemaphoreTypeCreateInfo timelineCreateInfo{};
		timelineCreateInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_TYPE_CREATE_INFO;
		timelineCreateInfo.pNext = nullptr;
		timelineCreateInfo.semaphoreType = VK_SEMAPHORE_TYPE_TIMELINE;
		timelineCreateInfo.initialValue = info.initialValue;

		VkSemaphoreCreateInfo createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
		createInfo.pNext = &timelineCreateInfo;
		createInfo.flags = 0;

		auto device = GraphicsContext::GetDevice();
		VT_VK_CHECK(vkCreateSemaphore(device->GetHandle<VkDevice>(), &createInfo, nullptr, &m_semaphore));
	}

	VulkanSemaphore::~VulkanSemaphore()
	{
		auto device = GraphicsContext::GetDevice();
		vkDestroySemaphore(device->GetHandle<VkDevice>(), m_semaphore, nullptr);
	}

	void* VulkanSemaphore::GetHandleImpl() const
	{
		return m_semaphore;
	}

	void VulkanSemaphore::Signal(const uint64_t signalValue)
	{
		VkSemaphoreSignalInfo signalInfo{};
		signalInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_SIGNAL_INFO;
		signalInfo.pNext = nullptr;
		signalInfo.semaphore = m_semaphore;
		signalInfo.value = signalValue;

		auto device = GraphicsContext::GetDevice();
		VT_VK_CHECK(vkSignalSemaphore(device->GetHandle<VkDevice>(), &signalInfo));
	}

	void VulkanSemaphore::Wait(const uint64_t waitValue)
	{
		VkSemaphoreWaitInfo waitInfo{};
		waitInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
		waitInfo.pNext = nullptr;
		waitInfo.flags = 0;
		waitInfo.semaphoreCount = 1;
		waitInfo.pSemaphores = &m_semaphore;
		waitInfo.pValues = &waitValue;

		auto device = GraphicsContext::GetDevice();
		VT_VK_CHECK(vkWaitSemaphores(device->GetHandle<VkDevice>(), &waitInfo, UINT64_MAX));
	}

	const uint64_t VulkanSemaphore::GetValue() const
	{
		uint64_t value;
		auto device = GraphicsContext::GetDevice();
		VT_VK_CHECK(vkGetSemaphoreCounterValue(device->GetHandle<VkDevice>(), m_semaphore, &value));

		return value;
	}
}
