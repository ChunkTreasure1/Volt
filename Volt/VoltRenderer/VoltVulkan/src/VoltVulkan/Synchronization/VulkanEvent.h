#pragma once

#include "VoltVulkan/Core.h"
#include <VoltRHI/Synchronization/Event.h>

struct VkEvent_T;

namespace Volt::RHI
{
	class VulkanEvent : public Event
	{
	public:
		VulkanEvent(const EventCreateInfo& createInfo);
		~VulkanEvent() override;

		void Reset() override;
		EventStatus GetStatus() const override;
		void WaitUntilSignaled() const override;

	protected:
		void* GetHandleImpl() const override;

	private:
		VkEvent_T* m_event = nullptr;
	};
}
