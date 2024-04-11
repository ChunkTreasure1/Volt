#include "rhipch.h"
#include "Event.h"

#include "VoltRHI/RHIProxy.h"

namespace Volt::RHI
{
	Ref<Event> Event::Create(const EventCreateInfo& createInfo)
	{
		return RHIProxy::GetInstance().CreateEvent(createInfo);
	}
}
