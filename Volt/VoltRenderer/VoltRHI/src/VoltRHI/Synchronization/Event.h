#pragma once

#include "VoltRHI/Core/RHIInterface.h"

namespace Volt::RHI
{
	enum class EventStatus : uint8_t
	{
		Signaled = 0,
		Unsignaled
	};

	struct EventCreateInfo
	{
		bool deviceOnly = false;
	};

	class Event : public RHIInterface
	{
	public:
		static Ref<Event> Create(const EventCreateInfo& createInfo);

		virtual void Reset() = 0;
		virtual EventStatus GetStatus() const = 0;
		virtual void WaitUntilSignaled() const = 0;

	protected:
		Event() = default;
	};
}