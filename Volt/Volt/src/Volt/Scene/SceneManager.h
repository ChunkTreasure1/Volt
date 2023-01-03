#pragma once

namespace Volt
{
	class Scene;
	class SceneManager
	{
	public:
		static void Shutdown();
		inline static void SetActiveScene(Ref<Scene> scene) { myActiveScene = scene; }
		inline static Ref<Scene> GetActiveScene() { return myActiveScene; }

	private:
		SceneManager() = delete;
		inline static Ref<Scene> myActiveScene;
	};
}