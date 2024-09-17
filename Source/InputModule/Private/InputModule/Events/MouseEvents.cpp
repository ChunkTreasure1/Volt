#include "inputpch.h"

#include "Events/MouseEvents.h"

namespace Volt
{
	std::string MouseMovedEvent::ToString() const
	{
		std::stringstream ss;
		ss << "MouseMovedEvent: " << m_mouseX << ", " << m_mouseY;
		return ss.str();
	}

	std::string MouseButtonPressedEvent::ToString() const
	{
		std::stringstream ss;
		ss << "MouseButtonPressedEvent: " << Volt::ToString(GetMouseButton());
		return ss.str();
	}

	std::string MouseButtonReleasedEvent::ToString() const
	{
		std::stringstream ss;
		ss << "MouseButtonReleasedEvent: " << Volt::ToString(GetMouseButton());
		return ss.str();
	}
}
