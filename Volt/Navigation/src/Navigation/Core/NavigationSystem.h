#pragma once

#include "CoreInterfaces.h"
#include "Navigation/NavMesh/VTNavMesh.h"

#include <Volt/Core/Base.h>

#include <Volt/Events/ApplicationEvent.h>

static const int MAX_POLYS = 256;

namespace Volt
{
	namespace AI
	{
		class NavigationSystem
		{
		public:
			NavigationSystem();
			~NavigationSystem() = default;

			void OnEvent(Event& event);

			void SetVTNavMesh(Ref<NavMesh> navmesh);
			const Ref<NavMesh>& GetVTNavMesh() { return myNavMesh; };

		private:
			friend class Scene;

			bool OnSceneLoadedEvent(Volt::OnSceneLoadedEvent& e);
			bool OnAppUpdateEvent(Volt::AppUpdateEvent& e);
			bool OnRuntimeStart();

			void PauseAgent(Volt::Entity entity, float deltaTime);
			void UnpauseAgent(Volt::Entity entity);
			void SyncDetourPosition(Volt::Entity entity, float deltaTime);

			void InitAgents();
			void ClearAgents();

			std::unordered_map<entt::entity, glm::vec3> myEntityIdToTargetPosMap;

			Ref<NavMesh> myNavMesh = nullptr;
			Ref<Scene> myActiveScene = nullptr;
		};
	}
}
