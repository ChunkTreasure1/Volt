#include "windowpch.h"

#include "WindowEvents.h"

namespace Volt
{
	std::string Volt::WindowResizeEvent::ToString() const
	{
		std::stringstream ss;
		ss << "WindowResizeEvent: " << m_width << ", " << m_height << std::endl;
		return ss.str();
	}

	std::string ViewportResizeEvent::ToString() const
	{
		std::stringstream ss;
		ss << "ViewportResizeEvent: " << m_width << ", " << m_height << std::endl;
		return ss.str();
	}
}
