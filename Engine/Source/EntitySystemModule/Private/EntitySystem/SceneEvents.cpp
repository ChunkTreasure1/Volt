#include "espch.h"
#include "SceneEvents.h"

namespace Volt
{
	OnSceneRuntimeStartEvent::OnSceneRuntimeStartEvent(Weak<EntityScene> scene)
		: m_scene(scene)
	{
	}

	Weak<EntityScene> OnSceneRuntimeStartEvent::GetScene()
	{
		return m_scene;
	}

	OnSceneRuntimeEndEvent::OnSceneRuntimeEndEvent(Weak<EntityScene> scene)
		: m_scene(scene)
	{
	}
	
	Weak<EntityScene> OnSceneRuntimeEndEvent::GetScene()
	{
		return m_scene;
	}
}
