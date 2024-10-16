#include "eventpch.h"
#include "EventSystem.h"

#include "EventSystem/EventListener.h"

namespace Volt
{
	VT_REGISTER_SUBSYSTEM(EventSystem, PreEngine, -1);

	EventSystem::EventSystem()
	{
		VT_ENSURE(s_instance == nullptr);
		s_instance = this;
	}

	EventSystem::~EventSystem()
	{
		s_instance = nullptr;
	}

	void EventSystem::RegisterListener(VoltGUID eventGUID, EventListenerDelegate delegate, EventDispatchPredicate predicate, EventListener* listener)
	{
		VT_ENSURE(s_instance);
		s_instance->m_registeredListeners[eventGUID].emplace_back(listener, delegate, predicate);
	}

	void EventSystem::UnregisterListeners(EventListener* listener)
	{
		VT_ENSURE(s_instance);
		for (auto& [guid, delegates] : s_instance->m_registeredListeners)
		{
			auto it = std::find_if(delegates.begin(), delegates.end(), [&](const ListenerInfo& info)
			{
				return info.listener == listener;
			});

			if (it != delegates.end())
			{
				delegates.erase(it);
			}
		}
	}

	void EventSystem::DispatchEventInternal(VoltGUID eventGUID, Event& e)
	{
		VT_PROFILE_SCOPE(std::format("Dispatch {}", e.GetName()).c_str());

		for (auto& info : m_registeredListeners[eventGUID])
		{
			if (info.listener->AreEventsBlocked())
			{
				continue;
			}

			if (info.predicate)
			{
				if (!info.predicate())
				{
					continue;
				}
			}

			// If the event gets handled, we skip the rest of the listeners
			if (info.delegate(e))
			{
				break;
			}
		}
	}
}
