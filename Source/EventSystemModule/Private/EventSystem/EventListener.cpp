#include "eventpch.h"
#include "EventListener.h"

#include "EventSystem/EventSystem.h"

namespace Volt
{
	EventListener::~EventListener()
	{
		EventSystem::UnregisterListeners(this);
	}

	void EventListener::UnregisterListeners()
	{
		EventSystem::UnregisterListeners(this);
	}

	void EventListener::BlockAllEvents()
	{
		m_eventsBlocked = true;
	}

	void EventListener::UnblockAllEvents()
	{
		m_eventsBlocked = false;
	}

	void EventListener::RegisterListenerInternal(VoltGUID eventGUID, std::function<bool(Event&)> delegate, std::function<bool()> predicate)
	{
		EventSystem::RegisterListener(eventGUID, delegate, predicate, this);
	}
}
