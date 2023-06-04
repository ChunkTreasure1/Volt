#pragma once

#include <Volt/Core/UUID.h>

#include <vector>
#include <functional>

namespace GraphKey
{
	class EventSystem
	{
	public:
		void RegisterListener(Volt::UUID nodeId, Volt::UUID eventId, std::function<void()>&& eventFunc);
		void UnregisterListener(Volt::UUID nodeId, Volt::UUID eventId);

		void Dispatch(const Volt::UUID& e);

	private:
		std::unordered_map<Volt::UUID, std::unordered_map<Volt::UUID, std::function<void()>>> myEvents;
	};
}