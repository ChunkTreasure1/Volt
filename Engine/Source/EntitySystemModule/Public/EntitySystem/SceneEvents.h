#pragma once

#include <EventSystem/Event.h>

namespace Volt
{
	class EntityScene;

	class OnSceneRuntimeStartEvent : public Event
	{
	public:
		OnSceneRuntimeStartEvent(EntityScene& scene);
		EntityScene& GetScene();

		EVENT_CLASS(OnSceneRuntimeStartEvent, "{AC1AC4C9-7C21-4BF9-BD9F-8D84C50F3F1C}"_guid);
	private:
		EntityScene& m_scene;
	};

	class OnSceneRuntimeEndEvent : public Event
	{
	public:
		OnSceneRuntimeEndEvent(EntityScene& scene);
		EntityScene& GetScene();

		EVENT_CLASS(OnSceneRuntimeEndEvent, "{AC1AC4C9-7C21-4BF9-BD9F-8D84C50F3F1C}"_guid);
	
	private:
		EntityScene& m_scene;
	};
}
