#include "vtpch.h"
#include "SceneManager.h"

namespace Volt
{
	void SceneManager::Shutdown()
	{
		myActiveScene = nullptr;
	}
}