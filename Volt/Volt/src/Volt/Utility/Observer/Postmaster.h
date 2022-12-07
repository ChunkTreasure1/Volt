#pragma once
#include <unordered_map>
#include <vector>
#include "ObserverEvent.h"

namespace Volt{
	class Observer;
	class Postmaster{
		static inline std::unordered_map<eEvent, std::vector<Observer*>> mySubscriberMap;

	public:
		Postmaster() = default;
		~Postmaster() = default;

		static void Invoke(const eEvent& aEvent, const EventMessage& aMessage);
		static bool Subscribe(Observer* ptrObserver, const eEvent& aEvent);
		static bool UnSubscribe(Observer* ptrObserver, const eEvent& aEvent);
	};
}