#pragma once

#include "InputModule/Config.h"

#include <EventSystem/Event.h>

namespace Volt
{
	class INPUTMODULE_API MouseMovedEvent : public Event
	{
	public:
		MouseMovedEvent(float x, float y)
			: m_mouseX(x), m_mouseY(y)
		{
		}

		//Getting
		inline float GetX() const { return m_mouseX; }
		inline float GetY() const { return m_mouseY; }

		std::string ToString() const override;

		EVENT_CLASS(MouseMovedEvent, "{7D80A1B6-FEE2-42AA-9004-AFB99EEB7E30}"_guid)

	private:
		float m_mouseX;
		float m_mouseY;
	};

	class INPUTMODULE_API MouseScrolledEvent : public Event
	{
	public:
		MouseScrolledEvent(float xOffset, float yOffset)
			: m_xOffset(xOffset), m_yOffset(yOffset)
		{
		}

		inline const float GetXOffset() const { return m_xOffset; }
		inline const float GetYOffset() const { return m_yOffset; }

		EVENT_CLASS(MouseScrolledEvent, "{128BF64B-5D6C-487F-9E75-DCAD110F220F}"_guid)
	private:
		float m_xOffset;
		float m_yOffset;
	};

	class INPUTMODULE_API MouseButtonEvent : public Event
	{
	public:
		//Getting
		inline int GetMouseButton() const { return m_button; }

		EVENT_CLASS(MouseButtonEvent, "{6A5764D2-B47E-41BF-9CA9-609AFC3610F2}"_guid)
	protected:
		MouseButtonEvent(int button)
			: m_button(button)
		{
		}

		int m_button;
	};

	class INPUTMODULE_API MouseButtonPressedEvent : public MouseButtonEvent
	{
	public:
		MouseButtonPressedEvent(int button)
			: MouseButtonEvent(button)
		{
		}

		std::string ToString() const override;

		EVENT_CLASS(MouseButtonEvent, "{86C684FF-C169-437E-BDAC-1AF7C721C8CA}"_guid)
	};

	class INPUTMODULE_API MouseButtonReleasedEvent : public MouseButtonEvent
	{
	public:
		MouseButtonReleasedEvent(int button)
			: MouseButtonEvent(button)
		{
		}

		std::string ToString() const override;

		EVENT_CLASS(MouseButtonEvent, "{39C7FC4A-705C-4431-A278-811DF01773EB}"_guid)

	};
}
