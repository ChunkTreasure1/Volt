#include "vtpch.h"
#include "NavMeshAgent.h"

#include "Volt/Scene/Entity.h"
#include "Volt/AI/NavigationSystem.h"
#include "Volt/Components/NavigationComponents.h"

#include <Volt/Core/Profiling.h>

namespace Volt
{
	void NavMeshAgent::SetTarget(const gem::vec3& aPosition)
	{
		auto navmesh = NavigationSystem::Get().GetNavMesh();
		if (gem::distance(myTarget, aPosition) > 10.f && navmesh) // Ignore small movements
		{
			myTarget = aPosition;
			auto path = navmesh->GetNavMeshData().findPath(VTtoPF(myCurrent), VTtoPF(myTarget));
			myPath.resize(path.size());
			for (const auto& p : path)
			{
				myPath.emplace_back(PFtoVT(p));
			}
		}
	};

	bool NavMeshAgent::GetCurrentMilestone(gem::vec3* outMilestone) const
	{
		if (!myPath.empty())
		{
			*outMilestone = myPath.back();
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
		}
	}

	void NavMeshAgent::MoveToTarget(float aTimestep, Entity aEntity)
	{
		switch (myBehavior)
		{
		case AgentSteeringBehaviors::Seek:
			Seek();
			break;

		case AgentSteeringBehaviors::Flee:
			Flee();
			break;

		case AgentSteeringBehaviors::Arrive:
			Arrive();
			break;

		case AgentSteeringBehaviors::Align:
			Align();
			break;

		case AgentSteeringBehaviors::Pursue:
			Pursue();
			break;

		case AgentSteeringBehaviors::Evade:
			Evade();
			break;

		case AgentSteeringBehaviors::Wander:
			Wander();
			break;

		case AgentSteeringBehaviors::PathFollowing:
			PathFollowing();
			break;

		case AgentSteeringBehaviors::Separation:
			Separation();
			break;

		case AgentSteeringBehaviors::CollisionAvoidance:
			CollisionAvoidance();
			break;

		default:
			break;
		}
	}

	void NavMeshAgent::Seek()
	{
		if (myIsKinematic)
		{

		}
		else
		{

		}
	}

	void NavMeshAgent::Flee()
	{
		if (myIsKinematic)
		{

		}
		else
		{

		}
	}

	void NavMeshAgent::Arrive()
	{
		if (myIsKinematic)
		{

		}
		else
		{

		}
	}

	void NavMeshAgent::Align()
	{
		if (myIsKinematic)
		{

		}
		else
		{

		}
	}

	void NavMeshAgent::Pursue()
	{
		if (myIsKinematic)
		{

		}
		else
		{

		}
	}

	void NavMeshAgent::Evade()
	{
		if (myIsKinematic)
		{

		}
		else
		{

		}
	}

	void NavMeshAgent::Wander()
	{
		if (myIsKinematic)
		{

		}
		else
		{

		}
	}

	void NavMeshAgent::PathFollowing()
	{
		if (myIsKinematic)
		{

		}
		else
		{

		}
	}

	void NavMeshAgent::Separation()
	{
		if (myIsKinematic)
		{

		}
		else
		{

		}
	}

	void NavMeshAgent::CollisionAvoidance()
	{
		if (myIsKinematic)
		{

		}
		else
		{

		}
	}
}
