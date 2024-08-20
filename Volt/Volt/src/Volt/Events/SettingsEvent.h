#pragma once

#include <EventSystem/Event.h>

#include "Volt/Rendering/SceneRendererSettings.h"

namespace Volt
{
	class OnRenderScaleChangedEvent : public Event
	{
	public:
		OnRenderScaleChangedEvent(float renderScale)
			: myRenderScale(renderScale)
		{ }

		inline const float GetRenderScale() const { return myRenderScale; }

		EVENT_CLASS(OnRenderScaleChangedEvent, "{B6C2F606-CE8D-41A9-B3BC-42DDBF2D40FE}"_guid);
	private:
		float myRenderScale = 0.f;
	};

	class OnRendererSettingsChangedEvent : public Event
	{
	public:
		OnRendererSettingsChangedEvent(const SceneRendererSettings& settings)
			: mySettings(settings)
		{
		}

		inline const SceneRendererSettings& GetSettings() const { return mySettings; }

		EVENT_CLASS(OnRenderScaleChangedEvent, "{921BC1D9-A876-4355-91BD-93B1CB24E140}"_guid);
	private:
		SceneRendererSettings mySettings{};
	};
}
