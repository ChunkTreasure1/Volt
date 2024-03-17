#include "vtpch.h"
#include "SceneManager.h"

namespace Volt
{
	void SceneManager::Shutdown()
	{
		m_activeScene.Reset();
	}
}
