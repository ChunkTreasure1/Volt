#include "rhipch.h"
#include "Event.h"

#include "VoltRHI/RHIProxy.h"

namespace Volt::RHI
{
	RefPtr<Event> Event::Create(const EventCreateInfo& createInfo)
	{
		return RHIProxy::GetInstance().CreateEvent(createInfo);
	}
}
