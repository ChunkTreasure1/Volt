#pragma once

#include <Volt/Core/UUID.h>

#include <vector>
#include <functional>

namespace GraphKey
{
	class EventSystem
	{
	public:
		void RegisterListener(UUID64 nodeId, UUID64 eventId, std::function<void()>&& eventFunc);
		void UnregisterListener(UUID64 nodeId, UUID64 eventId);

		void Dispatch(const UUID64& e);

	private:
		std::unordered_map<UUID64, std::unordered_map<UUID64, std::function<void()>>> myEvents;
	};
}
