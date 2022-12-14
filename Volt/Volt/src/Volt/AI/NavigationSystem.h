#pragma once
#include "Volt/Core/Base.h"
#include "Volt/AI/NavMesh.h"

namespace Volt
{
	class Scene;
	class Entity;
	class AgentComponent;

	class NavigationSystem
	{
	public:
		NavigationSystem(Ref<Scene>& aScene);
		~NavigationSystem() = default;

		const Ref<NavMesh>& GetNavMesh() { return myNavMesh; };
		void OnRuntimeUpdate(float ts);
		void OnSceneLoad(bool isRuntime);

		static NavigationSystem& Get() { return *myInstance; };

	private:
		void UpdateAgent(const uint32_t& id, AgentComponent& comp, const float& ts);
		gem::vec3 GetSteeringForce(const Entity& aEntity) const;

		Ref<NavMesh> myNavMesh;

		Ref<Scene>& myCurrentScene;
		std::filesystem::path myCurrentScenePath = "";
		inline static NavigationSystem* myInstance;
	};
}