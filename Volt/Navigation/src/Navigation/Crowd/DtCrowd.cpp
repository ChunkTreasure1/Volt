#include "nvpch.h"
#include "DtCrowd.h"

#include "Navigation/Core/CoreInterfaces.h"

#include <Volt/Log/Log.h>

#include <DetourCommon.h>

namespace Volt
{
	namespace AI
	{
		inline static void calcVel(float* vel, const float* pos, const float* tgt, const float speed)
		{
			dtVsub(vel, tgt, pos);
			vel[1] = 0.0;
			dtVnormalize(vel);
			dtVscale(vel, vel, speed);
		}

		DtCrowd::DtCrowd(Ref<DtNavMesh> navmesh)
			: myNavMesh(navmesh)
		{
			dtStatus status;

			myCrowd = CreateRef<dtCrowd>();

			status = myCrowd->init(myMaxAgents, myMaxAgentRadius, myNavMesh->GetNavMesh().get());
			if (dtStatusFailed(status))
			{
				VT_CORE_ERROR("Could not init DtCrowd");
			}

			// Make polygons with 'disabled' flag invalid.
			myCrowd->getEditableFilter(0)->setExcludeFlags(POLYFLAGS_DISABLED);

			// Setup local avoidance params to different qualities.
			dtObstacleAvoidanceParams params;
			// Use mostly default settings, copy from dtCrowd.
			memcpy(&params, myCrowd->getObstacleAvoidanceParams(0), sizeof(dtObstacleAvoidanceParams));

			// Low (11)
			params.velBias = 0.5f;
			params.adaptiveDivs = 5;
			params.adaptiveRings = 2;
			params.adaptiveDepth = 1;
			myCrowd->setObstacleAvoidanceParams(0, &params);

			// Medium (22)
			params.velBias = 0.5f;
			params.adaptiveDivs = 5;
			params.adaptiveRings = 2;
			params.adaptiveDepth = 2;
			myCrowd->setObstacleAvoidanceParams(1, &params);

			// Good (45)
			params.velBias = 0.5f;
			params.adaptiveDivs = 7;
			params.adaptiveRings = 2;
			params.adaptiveDepth = 3;
			myCrowd->setObstacleAvoidanceParams(2, &params);

			// High (66)
			params.velBias = 0.5f;
			params.adaptiveDivs = 7;
			params.adaptiveRings = 3;
			params.adaptiveDepth = 3;

			myCrowd->setObstacleAvoidanceParams(3, &params);
		}

		const dtCrowdAgent* DtCrowd::GetAgent(Volt::Entity entity)
		{
			if (!myCrowd || !entity.HasComponent<Volt::NavAgentComponent>() || !myEntityToAgentMap.contains(entity.GetId()))
			{
				VT_CORE_WARN("Could not get agent from entity: {0}", entity.GetId());
				return nullptr;
			}

			return myCrowd->getAgent(myEntityToAgentMap.at(entity.GetId()));
		}

		const dtCrowdAgent* DtCrowd::GetAgent(Wire::EntityId entityId)
		{
			if (!myCrowd || !myEntityToAgentMap.contains(entityId))
			{
				VT_CORE_WARN("Could not get agent from entity: {0}", entityId);
				return nullptr;
			}

			return myCrowd->getAgent(myEntityToAgentMap.at(entityId));
		}

		void DtCrowd::SetAgentPosition(Volt::Entity entity, glm::vec3 position)
		{
			auto entityId = entity.GetId();

			if (!myCrowd || !myEntityToAgentMap.contains(entityId))
			{
				VT_CORE_WARN("Could not set agent position for entity: {0}", entityId);
				return;
			}

			auto agentIndex = myEntityToAgentMap.at(entityId);

			dtCrowdAgent* ag = myCrowd->getEditableAgent(agentIndex);
			if (ag && ag->active)
			{
				ag->npos[0] = position.x;
				ag->npos[1] = position.y;
				ag->npos[2] = position.z;

				entity.SetPosition(position);
			}
		}

		void DtCrowd::SetAgentTarget(Volt::Entity entity, glm::vec3 target)
		{
			auto entityId = entity.GetId();

			if (!myCrowd || !myEntityToAgentMap.contains(entity.GetId()))
			{
				VT_CORE_WARN("Could not set agent target for entity: {0}", entityId);
				return;
			}

			auto agentIndex = myEntityToAgentMap.at(entityId);

			const dtCrowdAgent* ag = myCrowd->getAgent(agentIndex);
			const dtQueryFilter* filter = myCrowd->getFilter(0);
			const float* halfExtents = myCrowd->getQueryExtents();

			if (ag && ag->active)
			{
				dtPolyRef targetRef;
				glm::vec3 targetPos;

				myNavMesh->GetNavMeshQuery()->findNearestPoly((const float*)&target, halfExtents, filter, &targetRef, (float*)&targetPos);
				myCrowd->requestMoveTarget(agentIndex, targetRef, (const float*)&targetPos);
			}
		}

		void DtCrowd::ResetAgentTarget(Volt::Entity entity)
		{
			auto entityId = entity.GetId();

			if (!myCrowd || !myEntityToAgentMap.contains(entity.GetId()))
			{
				VT_CORE_WARN("Could not set agent target for entity: {0}", entityId);
				return;
			}

			auto agentIndex = myEntityToAgentMap.at(entityId);

			const dtCrowdAgent* ag = myCrowd->getAgent(agentIndex);

			if (ag && ag->active)
			{
				myCrowd->resetMoveTarget(agentIndex);
			}
		}

		dtCrowdAgentParams GetAgentParams(Volt::Entity entity)
		{
			dtCrowdAgentParams ap;
			memset(&ap, 0, sizeof(ap));
			if (!entity.HasComponent<Volt::NavAgentComponent>())
			{
				return ap;
			}

			auto& agentComponent = entity.GetComponent<Volt::NavAgentComponent>();

			ap.radius = agentComponent.radius;
			ap.height = agentComponent.height;
			ap.maxAcceleration = agentComponent.acceleration;
			ap.maxSpeed = agentComponent.maxSpeed;
			ap.collisionQueryRange = ap.radius * 12.0f;
			ap.pathOptimizationRange = ap.radius * 30.0f;
			ap.updateFlags = 0;
			//ap.updateFlags |= DT_CROWD_ANTICIPATE_TURNS;
			ap.updateFlags |= DT_CROWD_OPTIMIZE_VIS;
			ap.updateFlags |= DT_CROWD_OPTIMIZE_TOPO;
			ap.updateFlags |= DT_CROWD_OBSTACLE_AVOIDANCE;
			ap.updateFlags |= DT_CROWD_SEPARATION;
			ap.obstacleAvoidanceType = (unsigned char)agentComponent.obstacleAvoidanceQuality;
			ap.separationWeight = agentComponent.separationWeight;

			return ap;
		}

		void DtCrowd::UpdateAgentParams(Volt::Entity entity)
		{
			if (!myCrowd)
			{
				VT_CORE_WARN("Could not update agent parameters for entity: {0}", entity.GetId());
				return;
			}

			if (!myEntityToAgentMap.contains(entity.GetId()))
			{
				AddAgent(entity);
			}

			auto ap = GetAgentParams(entity);
			myCrowd->updateAgentParameters(myEntityToAgentMap.at(entity.GetId()), &ap);
		}

		void DtCrowd::AddAgent(Volt::Entity entity)
		{
			if (!myCrowd || !entity.HasComponent<Volt::NavAgentComponent>() || myEntityToAgentMap.contains(entity.GetId()))
			{
				VT_CORE_WARN("Could not add agent for entity: {0}", entity.GetId());
				return;
			}

			auto position = entity.GetPosition();

			auto ap = GetAgentParams(entity);

			myEntityToAgentMap[entity.GetId()] = (uint32_t)myCrowd->addAgent((const float*)&position, &ap);
		}

		void DtCrowd::RemoveAgent(Volt::Entity entity)
		{
			if (!myCrowd && !myEntityToAgentMap.contains(entity.GetId())) return;

			myCrowd->removeAgent((int)myEntityToAgentMap.at(entity.GetId()));
			myEntityToAgentMap.erase(entity.GetId());
		}

		void DtCrowd::ClearAgents()
		{
			if (!myCrowd) return;

			for (int i = 0; i < myCrowd->getAgentCount(); ++i)
			{
				myCrowd->removeAgent(i);
			}

			myEntityToAgentMap.clear();
		}
	}
}
