#pragma once

#include "EventModule/Config.h"

#include <string>
#include <CoreUtilities/VoltGUID.h>

//forward declare ostream
#include <iosfwd>

#define EVENT_CLASS(eventClass, eventGUID) static VoltGUID GetStaticGUID() {return eventGUID;}\
										virtual const VoltGUID GetGUID() const override {return GetStaticGUID(); }\
										virtual const char* GetName() const override { return #eventClass; }
namespace Volt
{
	class EVENTMODULE_API Event
	{
	public:

		virtual const VoltGUID GetGUID() const = 0;
		virtual const char* GetName() const = 0;
		virtual std::string ToString() const { return GetName(); }

		inline bool IsHandled() { return m_handled; }
		inline void SetHandled(bool handled) { m_handled = handled; }
	private:
		bool m_handled = false;
	};

	class EVENTMODULE_API EventDispatcher
	{
	public:
		EventDispatcher(Event& event)
			: m_event(event)
		{
		}

		template<typename T, typename F>
		bool Dispatch(const F& func)
		{
			if (m_event.GetGUID() == T::GetStaticGUID())
			{
				m_event.SetHandled(func(static_cast<T&>(m_event)));
				return true;
			}

			return false;
		}

	private:
		Event& m_event;
	};

	EVENTMODULE_API inline std::ostream& operator <<(std::ostream& os, const Event& e);
}
