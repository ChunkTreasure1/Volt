#pragma once

#include "Event.h"

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

		EVENT_CLASS_TYPE(OnRenderScaleChanged);
		EVENT_CLASS_CATEGORY(EventCategorySettings);

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

		EVENT_CLASS_TYPE(OnRendererSettingsChanged);
		EVENT_CLASS_CATEGORY(EventCategorySettings);

	private:
		SceneRendererSettings mySettings{};
	};
}
