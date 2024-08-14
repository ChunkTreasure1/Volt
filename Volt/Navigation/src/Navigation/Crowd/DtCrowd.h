#pragma once

#include "Navigation/NavMesh/DtNavMesh.h"

#include <Volt/Scene/Entity.h>
#include <Volt/Components/NavigationComponents.h>

#include <DetourCrowd.h>

namespace Volt
{
	namespace AI
	{
		class DtCrowd
		{
		public:
			DtCrowd(Ref<DtNavMesh> navmesh);

			const dtCrowdAgent* GetAgent(Volt::Entity entity);
			const dtCrowdAgent* GetAgent(EntityID entityId);
			const std::unordered_map<EntityID, uint32_t>& GetAgentMap() const { return myEntityToAgentMap; };

			void SetAgentPosition(Volt::Entity entity, glm::vec3 position);
			void SetAgentTarget(Volt::Entity entity, glm::vec3 target);
			void ResetAgentTarget(Volt::Entity entity);

			void UpdateAgentParams(Volt::Entity entity);

			void AddAgent(Volt::Entity entity);
			void RemoveAgent(Volt::Entity entity);
			void ClearAgents();

			void SetMaxAgents(uint32_t count) { myMaxAgents = count; };
			void SetMaxAgentRadius(float radius) { myMaxAgentRadius = radius; };

			Ref<dtCrowd>& GetDTCrowd() { return myCrowd; };

		private:
			std::unordered_map<EntityID, uint32_t> myEntityToAgentMap;

			uint32_t myMaxAgents = 100;
			float myMaxAgentRadius = 60.f;

			Ref<DtNavMesh> myNavMesh = nullptr;
			Ref<dtCrowd> myCrowd = nullptr;
		};
	}
}
