#pragma once

#include "EventModule/Event.h"

#include <AssetSystem/Asset.h>

namespace Volt
{
	class OnScenePlayEvent : public Event
	{
	public:
		OnScenePlayEvent() = default;

		EVENT_CLASS(OnScenePlayEvent, "{DDB25ECF-2095-4A95-AF48-C3101EA6756D}"_guid);
	};

	class OnSceneStopEvent : public Event
	{
	public:
		OnSceneStopEvent() = default;

		EVENT_CLASS(OnSceneStopEvent, "{BA8D9CF7-7AF9-465E-ACF7-58205BF1C21D}"_guid);
	};

	class Scene;
	class OnSceneLoadedEvent : public Event
	{
	public:
		OnSceneLoadedEvent(Ref<Volt::Scene> aScene)
			: myScene(aScene)
		{
		}

		inline Ref<Volt::Scene> GetScene() const { return myScene; }

		EVENT_CLASS(OnSceneLoadedEvent, "{D8C1545C-373E-4D0D-A546-9D382ED3AE3C}"_guid);


	private:
		Ref<Volt::Scene> myScene;
	};

	class OnSceneTransitionEvent : public Event
	{
	public:
		OnSceneTransitionEvent(Volt::AssetHandle aHandle)
			: myHandle(aHandle)
		{
		}

		inline Volt::AssetHandle GetHandle() const { return myHandle; }

		EVENT_CLASS(OnSceneLoadedEvent, "{95250536-3211-4AD7-A2F1-4ECB6CC860B7}"_guid);


	private:
		Volt::AssetHandle myHandle;
	};
}
