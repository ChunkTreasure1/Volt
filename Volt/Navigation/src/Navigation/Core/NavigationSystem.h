#pragma once

#include "CoreInterfaces.h"
#include "Navigation/NavMesh/VTNavMesh.h"

#include <Volt/Core/Base.h>

#include <Volt/Public/Events/ApplicationEvents.h>
#include <Volt/Public/Events/SceneEvents.h>

#include <EventSystem/EventListener.h>

static const int MAX_POLYS = 256;

namespace Volt
{
	namespace AI
	{
		class NavigationSystem : public EventListener
		{
		public:
			NavigationSystem();
			~NavigationSystem() = default;

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

			std::unordered_map<EntityID, glm::vec3> myEntityIdToTargetPosMap;

			Ref<NavMesh> myNavMesh = nullptr;
			Ref<Scene> myActiveScene = nullptr;
		};
	}
}
