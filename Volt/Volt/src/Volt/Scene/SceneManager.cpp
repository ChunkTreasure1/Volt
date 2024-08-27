#include "vtpch.h"
#include "SceneManager.h"

#include "Volt/Scene/Scene.h"

namespace Volt
{
	void SceneManager::Shutdown()
	{
		m_activeScene.Reset();
	}

	bool SceneManager::IsPlaying()
	{
		if (m_activeScene)
		{
			return m_activeScene->IsPlaying();
		}

		return false;
	}
}
