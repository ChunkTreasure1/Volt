#include "vtpch.h"
#include "NavMeshAgent.h"

#include "Volt/Scene/Entity.h"
#include "Volt/AI/NavigationSystem.h"
#include "Volt/Components/NavigationComponents.h"

#include "Volt/AI/SteeringBehavior.h"

#include "Volt/Rendering/Renderer.h"
#include "Volt/Core/Profiling.h"

namespace Volt
{
	void NavMeshAgent::SetTarget(const gem::vec3& aPosition)
	{
		auto navmesh = NavigationSystem::Get().GetNavMesh();
		if (gem::distance(myTarget, aPosition) > 10.f && navmesh) // Ignore small movements
		{
			myTarget = aPosition;
			auto path = navmesh->GetNavMeshData().findPath(VTtoPF(myCurrent), VTtoPF(myTarget));
			if (!path.empty()) { myPath.clear(); }
			for (const auto& p : path)
			{
				myPath.emplace_back(PFtoVT(p));
			}
			myPath.pop_back();
		}
	};

	bool NavMeshAgent::GetCurrentMilestone(gem::vec3* outMilestone) const
	{
		if (!myPath.empty())
		{
			if (outMilestone)
			{
				*outMilestone = myPath.back();
			}
			return true;
		}
		else
		{
			return false;
		}
	}

	void NavMeshAgent::Update(float aTimestep, Entity aEntity)
	{
		myCurrent = aEntity.GetWorldPosition();
		if (myActive && GetCurrentMilestone())
		{
			MoveToTarget(aTimestep, aEntity);
			DrawDebugLines();
		}
	}

	void NavMeshAgent::MoveToTarget(float aTimestep, Entity aEntity)
	{
		auto& agentComp = aEntity.GetComponent<AgentComponent>();
		auto steeringForce = GetSteeringForce(aEntity);

		if (agentComp.kinematic)
		{
			aEntity.SetWorldPosition(myCurrent + steeringForce * aTimestep);
		}
		else
		{
			auto acceleration = steeringForce;
			myVelocity = myVelocity + acceleration * aTimestep;
			myVelocity = gem::clamp(myVelocity, gem::vec3(-1.f) * agentComp.maxVelocity, gem::vec3(1.f) * agentComp.maxVelocity);

			aEntity.SetWorldPosition(myCurrent + myVelocity * aTimestep);
		}

		// #SAMUEL_TODO: this needs to be better implemented
		gem::vec3 milestone;
		if (GetCurrentMilestone(&milestone))
		{
			float dist = gem::distance(myCurrent, milestone);
			if (dist < 10.f) // continue to next milestone if close enough to current milestone
			{
				myPath.pop_back();
			}
		}
	}

	gem::vec3 NavMeshAgent::GetSteeringForce(Entity aEntity)
{
		gem::vec3 target;
		if (GetCurrentMilestone(&target))
		{
			switch (myBehavior)
			{
			case AgentSteeringBehaviors::Seek:
				return SteeringBehavior::Seek(aEntity, target);

			case AgentSteeringBehaviors::Flee:
				return SteeringBehavior::Flee(aEntity, target);

			case AgentSteeringBehaviors::Arrive:
				return SteeringBehavior::Arrive(aEntity, target);

			case AgentSteeringBehaviors::Align:
				return SteeringBehavior::Align(aEntity, target);

			case AgentSteeringBehaviors::Pursue:
				return SteeringBehavior::Pursue(aEntity, target);

			case AgentSteeringBehaviors::Evade:
				return SteeringBehavior::Evade(aEntity, target);

			case AgentSteeringBehaviors::Wander:
				return SteeringBehavior::Wander(aEntity, target);

			case AgentSteeringBehaviors::PathFollowing:
				return SteeringBehavior::PathFollowing(aEntity, target);

			case AgentSteeringBehaviors::Separation:
				return SteeringBehavior::Separation(aEntity, target);

			case AgentSteeringBehaviors::CollisionAvoidance:
				return SteeringBehavior::CollisionAvoidance(aEntity, target);

			default:
				return SteeringBehavior::Seek(aEntity, target);
			}
		}
	}

	void NavMeshAgent::DrawDebugLines()
	{
		Renderer::SubmitLine(myCurrent, myCurrent + myVelocity, gem::vec4(0.f, 0.f, 1.f, 1.f));
	}
}
