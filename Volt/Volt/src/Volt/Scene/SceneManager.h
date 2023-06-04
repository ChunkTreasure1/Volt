#pragma once

#include "Volt/Core/Base.h"

namespace Volt
{
	class Scene;
	class SceneManager
	{
	public:
		static void Shutdown();
		inline static void SetActiveScene(Weak<Scene> scene) { myActiveScene = scene; }
		inline static Weak<Scene> GetActiveScene() { return myActiveScene; }

	private:
		SceneManager() = delete;
		inline static Weak<Scene> myActiveScene;
	};
}
