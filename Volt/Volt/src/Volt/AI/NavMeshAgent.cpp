#include "vtpch.h"
#include "NavMeshAgent.h"

#include "Volt/Scene/Entity.h"
#include "Volt/AI/NavigationSystem.h"
#include "Volt/Components/NavigationComponents.h"

#include <Volt/Rendering/Renderer.h>
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
		auto steeringForce = GetSteeringForce();

		if (myIsKinematic)
		{
			aEntity.SetWorldPosition(myCurrent + steeringForce * aTimestep);
		}
		else
		{
			auto acceleration = steeringForce;
			myVelocity = myVelocity + acceleration * aTimestep;
			myVelocity = gem::clamp(myVelocity, gem::vec3(-1.f) * myMaxVelocity, gem::vec3(1.f) * myMaxVelocity);

			aEntity.SetWorldPosition(myCurrent + myVelocity * aTimestep);
		}

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

	gem::vec3 NavMeshAgent::GetSteeringForce()
	{
		switch (myBehavior)
		{
		case AgentSteeringBehaviors::Seek:
			return Seek();

		case AgentSteeringBehaviors::Flee:
			return Flee();

		case AgentSteeringBehaviors::Arrive:
			return Arrive();

		case AgentSteeringBehaviors::Align:
			return Align();

		case AgentSteeringBehaviors::Pursue:
			return Pursue();

		case AgentSteeringBehaviors::Evade:
			return Evade();

		case AgentSteeringBehaviors::Wander:
			return Wander();

		case AgentSteeringBehaviors::PathFollowing:
			return PathFollowing();

		case AgentSteeringBehaviors::Separation:
			return Separation();

		case AgentSteeringBehaviors::CollisionAvoidance:
			return CollisionAvoidance();

		default:
			return Seek();
		}
	}

	void NavMeshAgent::DrawDebugLines()
	{
		Renderer::SubmitLine(myCurrent, myCurrent + myVelocity, gem::vec4(0.f, 0.f, 1.f, 1.f));
	}

	gem::vec3 NavMeshAgent::Seek()
	{
		gem::vec3 currentTarget;
		gem::vec3 desiredVelocity;
		gem::vec3 steeringForce;
		if (GetCurrentMilestone(&currentTarget))
		{
			desiredVelocity = currentTarget - myCurrent;
			desiredVelocity = gem::normalize(desiredVelocity);
			desiredVelocity *= myMaxVelocity;
			steeringForce = desiredVelocity - myVelocity;
			steeringForce /= myMaxVelocity;
			steeringForce *= myMaxForce;
		}
		return steeringForce;
	}

	gem::vec3 NavMeshAgent::Flee()
	{
		if (myIsKinematic)
		{

		}
		else
		{

		}
		return gem::vec3();
	}

	gem::vec3 NavMeshAgent::Arrive()
	{
		if (myIsKinematic)
		{

		}
		else
		{

		}
		return gem::vec3();
	}

	gem::vec3 NavMeshAgent::Align()
	{
		if (myIsKinematic)
		{

		}
		else
		{

		}
		return gem::vec3();
	}

	gem::vec3 NavMeshAgent::Pursue()
	{
		if (myIsKinematic)
		{

		}
		else
		{

		}
		return gem::vec3();
	}

	gem::vec3 NavMeshAgent::Evade()
	{
		if (myIsKinematic)
		{

		}
		else
		{

		}
		return gem::vec3();
	}

	gem::vec3 NavMeshAgent::Wander()
	{
		if (myIsKinematic)
		{

		}
		else
		{

		}
		return gem::vec3();
	}

	gem::vec3 NavMeshAgent::PathFollowing()
	{
		if (myIsKinematic)
		{

		}
		else
		{

		}
		return gem::vec3();
	}

	gem::vec3 NavMeshAgent::Separation()
	{
		if (myIsKinematic)
		{

		}
		else
		{

		}
		return gem::vec3();
	}

	gem::vec3 NavMeshAgent::CollisionAvoidance()
	{
		if (myIsKinematic)
		{

		}
		else
		{

		}
		return gem::vec3();
	}
}
