#include "inputpch.h"
#include "Events/KeyboardEvents.h"

namespace Volt
{
	std::string KeyPressedEvent::ToString() const
	{
		std::stringstream ss;
		ss << "KeyPressedEvent: " << Volt::ToString(m_keyCode) << "(" << m_repeatCount << " repeats)";
		return ss.str();
	}

	std::string KeyReleasedEvent::ToString() const
	{
		std::stringstream ss;
		ss << "KeyReleasedEvent: " << Volt::ToString(m_keyCode);
		return ss.str();
	}

	std::string KeyTypedEvent::ToString() const
	{
		std::stringstream ss;
		ss << "KeyTypedEvent: " << Volt::ToString(m_keyCode);
		return ss.str();
	}
}
