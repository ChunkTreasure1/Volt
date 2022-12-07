#pragma once

#include "Event.h"
#include <sstream>

namespace Volt
{
	class KeyEvent : public Event
	{
	public:
		inline int32_t GetKeyCode() const { return m_keyCode; }

		EVENT_CLASS_CATEGORY(EventCategoryKeyboard | EventCategoryInput);
	protected:
		KeyEvent(int32_t keyCode)
			: m_keyCode(keyCode)
		{
		}

		int m_keyCode;
	};

	class KeyPressedEvent : public KeyEvent
	{
	public:
		KeyPressedEvent(int32_t keyCode, int32_t repeatCount)
			: KeyEvent(keyCode), m_repeatCount(repeatCount)
		{
		}

		inline int GetRepeatCount() const { return m_repeatCount; }

		std::string ToString() const override
		{
			std::stringstream ss;
			ss << "KeyPressedEvent: " << m_keyCode << "(" << m_repeatCount << " repeats)";
			return ss.str();
		}

		EVENT_CLASS_TYPE(KeyPressed);
	private:
		int32_t m_repeatCount;
	};

	class KeyReleasedEvent : public KeyEvent
	{
	public:
		KeyReleasedEvent(int32_t keyCode)
			: KeyEvent(keyCode)
		{
		}

		std::string ToString() const override
		{
			std::stringstream ss;
			ss << "KeyReleasedEvent: " << m_keyCode;
			return ss.str();
		}

		EVENT_CLASS_TYPE(KeyReleased);
	};

	class KeyTypedEvent : public KeyEvent
	{
	public:
		KeyTypedEvent(int32_t keyCode)
			: KeyEvent(keyCode)
		{
		}

		std::string ToString() const override
		{
			std::stringstream ss;
			ss << "KeyTypedEvent: " << m_keyCode;
			return ss.str();
		}

		EVENT_CLASS_TYPE(KeyTyped);
	};
}