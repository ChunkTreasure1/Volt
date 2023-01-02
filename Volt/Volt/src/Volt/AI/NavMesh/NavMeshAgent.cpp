#include "vtpch.h"
#include "NavMeshAgent.h"

#include "Volt/Scene/Entity.h"
#include "Volt/Components/Components.h"
#include "Volt/AI/Pathfind/ThetaStar.h"
#include "Volt/AI/NavMesh2/NavMesh2.h"

#include <Volt/Core/Profiling.h>

namespace Volt
{
	NavMeshAgent::NavMeshAgent()
	{

	}

	NavMeshAgent::~NavMeshAgent()
	{

	}

	void NavMeshAgent::StartNavigation()
	{
		myActive = true;
	}

	void NavMeshAgent::StopNavigation()
	{
		myActive = false;
	}

	void NavMeshAgent::SetTarget(gem::vec3 aPosition)
	{
		VT_PROFILE_FUNCTION();

		//if (AStar::IsValidPosition(myAgentPosition))
		{
			if (NavigationsSystem::Get().GetNavMesh2())
			{
				auto newPath = NavigationsSystem::Get().GetNavMesh2()->FindPath(myAgentPosition, aPosition);
				if (!newPath.empty())
				{
					myTarget = aPosition;
					myPath2 = NavigationsSystem::Get().GetNavMesh2()->FindPath(myAgentPosition, aPosition);
				}
			}
			//if (bUseThetaStar)
			//{
			//	myPath = FindPath(myAgentPosition, myTarget.value());
			//}
			//else
			//{
			//	myPath = std::stack<gem::vec3>();
			//	myPath.push(aPosition);
			//}
		}
	}

	std::optional<gem::vec3> NavMeshAgent::GetCurrentTarget() const
	{
		std::optional<gem::vec3> targetPos;
		if (!myPath2.empty())
		{
			targetPos = myPath2.back();
		}
		return targetPos;
	}

	bool NavMeshAgent::HasArrived() const
	{
		return !GetCurrentTarget().has_value();
	}

	void NavMeshAgent::Update(float aTimestep, Entity aEntity)
	{
		if (myActive)
		{
			myAgentPosition = aEntity.GetPosition();
			MoveToTarget(aTimestep, aEntity);
		}
	}

	void NavMeshAgent::MoveToTarget(float aTimestep, Entity aEntity)
	{
		if (!myTarget.has_value() || HasArrived()) { return; }

		gem::vec3 targetPos = GetCurrentTarget().value();
		auto worldPos = aEntity.GetPosition();

		while (gem::distance(worldPos, targetPos) < 10.f && !myPath2.empty())
		{
			myPath2.pop_back();
			if (HasArrived()) { return; }
			targetPos = GetCurrentTarget().value();
		}

		auto moveDirection = gem::normalize(targetPos - worldPos);
		aEntity.SetPosition(worldPos + moveDirection * mySpeed * aTimestep);
	}

	std::stack<gem::vec3> NavMeshAgent::FindPath(const gem::vec3& start, const gem::vec3& target)
	{
		auto res = ThetaStar::FindPath(start, target);
		return res.PathPositions;
	}
}
