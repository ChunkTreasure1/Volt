#include "espch.h"
#include "SceneEvents.h"

namespace Volt
{
	OnSceneRuntimeStartEvent::OnSceneRuntimeStartEvent(EntityScene& scene)
		: m_scene(scene)
	{
	}

	EntityScene& OnSceneRuntimeStartEvent::GetScene()
	{
		return m_scene;
	}

	OnSceneRuntimeEndEvent::OnSceneRuntimeEndEvent(EntityScene& scene)
		: m_scene(scene)
	{
	}
	
	EntityScene& OnSceneRuntimeEndEvent::GetScene()
	{
		return m_scene;
	}
}
