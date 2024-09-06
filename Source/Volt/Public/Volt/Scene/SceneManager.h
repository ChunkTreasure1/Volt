#pragma once

#include "Volt/Core/Base.h"

namespace Volt
{
	class Scene;
	class SceneManager
	{
	public:
		static void Shutdown();
		inline static void SetActiveScene(Weak<Scene> scene) { m_activeScene = scene; }
		inline static Weak<Scene> GetActiveScene() { return m_activeScene; }
		static bool IsPlaying();

	private:
		SceneManager() = delete;
		inline static Weak<Scene> m_activeScene;
	};
}
