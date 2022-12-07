#pragma once
#include "ObserverEvent.h"
#include "Postmaster.h"

namespace Volt{
	class Observer{
		friend class Postmaster;
		std::vector<eEvent> mySubscriptions;

	public:
		Observer(){};
		~Observer(){
			for(eEvent e : mySubscriptions)
				Postmaster::UnSubscribe(this, e);
		}

		virtual void OnEvent(const eEvent& aEvent, const EventMessage& aMessage) = 0;
	};
}