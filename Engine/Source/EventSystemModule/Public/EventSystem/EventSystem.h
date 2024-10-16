#pragma once

#include "EventSystem/Event.h"

#include <SubSystem/SubSystem.h>

#include <CoreUtilities/Containers/Map.h>
#include <CoreUtilities/VoltGUID.h>

namespace Volt
{
	class EventListener;
	class EVENTMODULE_API EventSystem : public SubSystem
	{
	public:
		EventSystem();
		~EventSystem();

		typedef std::function<bool(Event& e)> EventListenerDelegate;
		typedef std::function<bool()> EventDispatchPredicate;

		static void RegisterListener(VoltGUID eventGUID, EventListenerDelegate delegate, EventDispatchPredicate predicate, EventListener* listener);
		static void UnregisterListeners(EventListener* listener);

		template<IsEvent T>
		static void DispatchEvent(T& e)
		{
			VT_ENSURE(s_instance);
			s_instance->DispatchEventInternal(T::GetStaticGUID(), e);
		}

		VT_DECLARE_SUBSYSTEM("{53104069-97D1-459F-B307-7E6DB62676BF}"_guid)

	private:
		inline static EventSystem* s_instance = nullptr;
	
		struct ListenerInfo
		{
			EventListener* listener;
			EventListenerDelegate delegate;
			EventDispatchPredicate predicate;
		};

		void DispatchEventInternal(VoltGUID eventGUID, Event& e);

		vt::map<VoltGUID, Vector<ListenerInfo>> m_registeredListeners;
	};
}
