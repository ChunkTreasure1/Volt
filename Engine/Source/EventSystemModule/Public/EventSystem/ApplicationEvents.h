#pragma once

#include "EventSystem/Event.h"

namespace Volt
{
	class AppUpdateEvent : public Event
	{
	public:
		AppUpdateEvent(float timestep)
			: m_timestep(timestep)
		{
		}

		inline const float& GetTimestep() { return m_timestep; }

		EVENT_CLASS(AppUpdateEvent, "{07AB89BC-1ACE-4FFD-B2C3-ED6DA5A24029}"_guid);

	private:
		float m_timestep;
	};

	class AppPostFrameUpdateEvent : public Event
	{
	public:
		AppPostFrameUpdateEvent(float timestep)
			: m_timestep(timestep)
		{ }

		inline const float& GetTimestep() { return m_timestep; }

		EVENT_CLASS(AppPostFrameUpdateEvent, "{FD1AD6FA-5428-4FBD-BDF9-7D1BFF009D85}"_guid);
	private:
		float m_timestep;
	};

	class AppImGuiUpdateEvent : public Event
	{
	public:
		AppImGuiUpdateEvent() = default;

		EVENT_CLASS(AppImGuiUpdateEvent, "{AAFC6679-2596-4706-99B1-E8F27A339C55}"_guid);
	};

	class AppPreRenderEvent : public Event
	{
	public:
		AppPreRenderEvent() = default;

		EVENT_CLASS(AppPreRenderEvent, "{F1179BDD-7FF6-4CD1-9238-A1BC31E7A062}"_guid);
	};

	class AppRenderEvent : public Event
	{
	public:
		AppRenderEvent() = default;

		EVENT_CLASS(AppRenderEvent, "{B38DBA07-ACAA-4334-8751-2570560D4490}"_guid);
	};
}
