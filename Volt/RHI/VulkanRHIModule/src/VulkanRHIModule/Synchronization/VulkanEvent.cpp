#include "vkpch.h"
#include "VulkanEvent.h"

#include "VulkanRHIModule/Common/VulkanCommon.h"

#include <RHIModule/Graphics/GraphicsContext.h>
#include <RHIModule/Graphics/GraphicsDevice.h>

#include <RHIModule/Utility/ValidationTimer.h>

#include <RHIModule/RHIProxy.h>

#include <vulkan/vulkan.h>

namespace Volt::RHI
{
	VulkanEvent::VulkanEvent(const EventCreateInfo& createInfo)
	{
		VkEventCreateInfo info{};
		info.sType = VK_STRUCTURE_TYPE_EVENT_CREATE_INFO;
		info.pNext = nullptr;
		info.flags = createInfo.deviceOnly ? VK_EVENT_CREATE_DEVICE_ONLY_BIT : 0;

		auto device = GraphicsContext::GetDevice();
		VT_VK_CHECK(vkCreateEvent(device->GetHandle<VkDevice>(), &info, nullptr, &m_event));
	}
	
	VulkanEvent::~VulkanEvent()
	{
		RHIProxy::GetInstance().DestroyResource([event = m_event]()
		{
			auto device = GraphicsContext::GetDevice();
			vkDestroyEvent(device->GetHandle<VkDevice>(), event, nullptr);
		});
	}

	void VulkanEvent::Reset()
	{
		auto device = GraphicsContext::GetDevice();
		vkResetEvent(device->GetHandle<VkDevice>(), m_event);
	}

	EventStatus VulkanEvent::GetStatus() const
	{
		auto device = GraphicsContext::GetDevice();
		VkResult result = vkGetEventStatus(device->GetHandle<VkDevice>(), m_event);

		return result == VK_EVENT_SET ? EventStatus::Signaled : EventStatus::Unsignaled;
	}

	void VulkanEvent::WaitUntilSignaled() const
	{
		auto device = GraphicsContext::GetDevice();
		VkDevice vkDevice = device->GetHandle<VkDevice>();

#ifdef VT_DEBUG
		ValidationTimer timer{};
		constexpr float MAX_WAIT_TIME = 5000.f;

		while (vkGetEventStatus(vkDevice, m_event) && timer.GetTimeMilliseconds() < MAX_WAIT_TIME)
		{}

		VT_ENSURE(timer.GetTimeMilliseconds() < MAX_WAIT_TIME);
#else
		while (vkGetEventStatus(vkDevice, m_event))
		{}
#endif
	}

	void* VulkanEvent::GetHandleImpl() const
	{
		return m_event;
	}
}
