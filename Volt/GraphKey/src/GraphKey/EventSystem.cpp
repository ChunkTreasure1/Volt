#include "gkpch.h"
#include "EventSystem.h"

namespace GraphKey
{
	void EventSystem::RegisterListener(Volt::UUID nodeId, Volt::UUID eventType, std::function<void()>&& eventFunc)
	{
		auto& typeEvents = myEvents[eventType];

		if (!typeEvents.contains(nodeId))
		{
			typeEvents.emplace(nodeId, std::move(eventFunc));
		}
	}

	void EventSystem::UnregisterListener(Volt::UUID nodeId, Volt::UUID eventType)
	{
		auto& typeEvents = myEvents[eventType];
		if (typeEvents.contains(nodeId))
		{
			typeEvents.erase(nodeId);
		}
	}

	void EventSystem::Dispatch(const Volt::UUID& e)
	{
		auto& typeEvents = myEvents[e];
		for (const auto& [nodeId, func] : typeEvents)
		{
			func();
		}
	}
}