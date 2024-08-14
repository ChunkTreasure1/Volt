#pragma once

#include "InputModule/Config.h"

#include <EventModule/Event.h>

namespace Volt
{
	class INPUTMODULE_API KeyEvent : public Event
	{
	public:
		inline int32_t GetKeyCode() const { return m_keyCode; }

		EVENT_CLASS(KeyEvent, "{F57124D9-554B-4A8F-9788-79641498BF1C}"_guid);
	protected:
		KeyEvent(int32_t keyCode)
			: m_keyCode(keyCode)
		{
		}

		int m_keyCode;
	};

	class INPUTMODULE_API KeyPressedEvent : public KeyEvent
	{
	public:
		KeyPressedEvent(int32_t keyCode, int32_t repeatCount)
			: KeyEvent(keyCode), m_repeatCount(repeatCount)
		{
		}

		inline int GetRepeatCount() const { return m_repeatCount; }

		std::string ToString() const override;

		EVENT_CLASS(KeyPressedEvent, "{35E9A5ED-54B1-46F0-B5A3-3851BC78FC93}"_guid);
	private:
		int32_t m_repeatCount;
	};

	class INPUTMODULE_API KeyReleasedEvent : public KeyEvent
	{
	public:
		KeyReleasedEvent(int32_t keyCode)
			: KeyEvent(keyCode)
		{
		}

		std::string ToString() const override;

		EVENT_CLASS(KeyReleasedEvent, "{683AC3F1-9CEC-4BEA-9944-1740CD80E7A4}"_guid);
	};

	class INPUTMODULE_API KeyTypedEvent : public KeyEvent
	{
	public:
		KeyTypedEvent(int32_t keyCode)
			: KeyEvent(keyCode)
		{
		}

		std::string ToString() const override;

		EVENT_CLASS(KeyTypedEvent, "{E01DA431-6A5A-427D-8A99-2D1A3D4868AB}"_guid);
	};
}
