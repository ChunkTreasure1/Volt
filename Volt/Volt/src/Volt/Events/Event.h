
#pragma once

#include "Volt/Core/Base.h"

#include <cstdint>
#include <string>

#define EVENT_CLASS_TYPE(type) static Volt::EventType GetStaticType() { return Volt::EventType::##type; }\
								virtual Volt::EventType GetEventType() const override { return GetStaticType(); }\
								virtual const char* GetName() const override { return #type; }

#define EVENT_CLASS_CATEGORY(category) virtual int GetCategoryFlags() const override { return category; }


namespace Volt
{
	enum EventType
	{
		None = 0,
		WindowClose, WindowResize, WindowFocus, WindowLostFocus, WindowMoved, WindowDragDrop, ViewportResize, WindowTitlebarHittest,
		AppUpdate, AppRender, AppLog, AppImGuiUpdate,
		KeyPressed, KeyReleased, KeyTyped,
		MouseButtonPressed, MouseButtonReleased, MouseMoved, MouseScrolled, MouseMovedViewport,

		// Game
		OnScenePlay, OnSceneStop, OnSceneLoaded, OnSceneTransition, OnRespawn, OnGameStateChanged, OnPlayGame,

		// GraphKey
		OnCollisionEnter, OnCollisionExit, OnTriggerEnter, OnTriggerExit,

		// Settings
		OnRenderScaleChanged,
		OnRendererSettingsChanged
	};

	enum EventCategory
	{
		EventCategoryNone = 0,
		EventCategoryApplication = BIT(0),
		EventCategoryInput = BIT(1),
		EventCategoryKeyboard = BIT(2),
		EventCategoryMouse = BIT(3),
		EventCategoryMouseButton = BIT(4),
		EventCategoryGame = BIT(5),
		EventCategoryGraphKey = BIT(6),
		EventCategorySettings = BIT(7),

		EventCategoryAnyInput = EventCategoryInput | EventCategoryMouse | EventCategoryMouseButton
	};

	class Event
	{
	public:
		bool handled = false;

		virtual EventType GetEventType() const = 0;
		virtual const char* GetName() const = 0;
		virtual int32_t GetCategoryFlags() const = 0;
		virtual std::string ToString() const { return GetName(); }

		inline bool IsInCategory(EventCategory category) { return GetCategoryFlags() & category; }
	};

	class EventDispatcher
	{
	public:
		EventDispatcher(Event& event)
			: m_event(event)
		{
		}

		template<typename T, typename F>
		bool Dispatch(const F& func)
		{
			if (m_event.GetEventType() == T::GetStaticType())
			{
				m_event.handled = func(static_cast<T&>(m_event));
				return true;
			}

			return false;
		}

	private:
		Event& m_event;
	};

	inline std::ostream& operator <<(std::ostream& os, const Event& e)
	{
		return os << e.ToString();
	}
}
