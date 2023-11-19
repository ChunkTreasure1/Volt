#include "gkpch.h"
#include "EventSystem.h"

namespace GraphKey
{
	void EventSystem::RegisterListener(UUID64 nodeId, UUID64 eventType, std::function<void()>&& eventFunc)
	{
		auto& typeEvents = myEvents[eventType];

		if (!typeEvents.contains(nodeId))
		{
			typeEvents.emplace(nodeId, std::move(eventFunc));
		}
	}

	void EventSystem::UnregisterListener(UUID64 nodeId, UUID64 eventType)
	{
		auto& typeEvents = myEvents[eventType];
		if (typeEvents.contains(nodeId))
		{
			typeEvents.erase(nodeId);
		}
	}

	void EventSystem::Dispatch(const UUID64& e)
	{
		auto& typeEvents = myEvents[e];
		for (const auto& [nodeId, func] : typeEvents)
		{
			func();
		}
	}
}
