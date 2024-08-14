#include "rhipch.h"
#include "Event.h"

#include "RHIModule/RHIProxy.h"

namespace Volt::RHI
{
	RefPtr<Event> Event::Create(const EventCreateInfo& createInfo)
	{
		return RHIProxy::GetInstance().CreateEvent(createInfo);
	}
}
