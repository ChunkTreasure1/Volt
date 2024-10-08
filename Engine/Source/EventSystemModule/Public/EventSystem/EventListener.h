#pragma once

#include "EventSystem/Event.h"

#include <functional>

namespace Volt
{
	class EVENTMODULE_API EventListener
	{
	public:
		EventListener() = default;
		virtual ~EventListener();

		VT_INLINE bool AreEventsBlocked() const { return m_eventsBlocked; }

	protected:
		void UnregisterListeners();

		void BlockAllEvents();
		void UnblockAllEvents();

		template<IsEvent T, typename F>
		void RegisterListener(const F& func)
		{
			RegisterListenerInternal(T::GetStaticGUID(), [func](Event& event)
			{
				return func(static_cast<T&>(event));
			}, nullptr);
		}

		template<IsEvent T, typename F, typename P>
		void RegisterListener(const F& func, const P& predicate)
		{
			RegisterListenerInternal(T::GetStaticGUID(), [func](Event& event)
			{
				return func(static_cast<T&>(event));
			}, predicate);
		}

	private:
		void RegisterListenerInternal(VoltGUID eventGUID, std::function<bool(Event&)> delegate, std::function<bool()> predicate);
	
		bool m_eventsBlocked = false;
	};
}
