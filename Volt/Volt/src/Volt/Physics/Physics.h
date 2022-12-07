#pragma once

#include "Volt/Physics/PhysicsSettings.h"
#include "Volt/Scene/Entity.h"

namespace Volt
{
	class Scene;

	class PhysicsActor;
	class PhysicsScene;
	class Physics
	{
	public:
		static void Initialize();
		static void Shutdown();

		static void LoadSettings();
		static void SaveSettings();

		static void LoadLayers();
		static void SaveLayers();

		static Ref<PhysicsScene> GetScene() { return myScene; }

		static void CreateScene(Scene* scene);
		static void DestroyScene();

		static void CreateActors(Scene* scene);
		static Ref<PhysicsActor> CreateActor(Entity entity);

		static const PhysicsSettings& GetSettings() { return mySettings; }

	private:
		Physics() = delete;

		inline static PhysicsSettings mySettings;
		inline static Ref<PhysicsScene> myScene;
	};
}