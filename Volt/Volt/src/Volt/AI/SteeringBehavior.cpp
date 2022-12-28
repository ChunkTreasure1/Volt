#include "vtpch.h"
#include "SteeringBehavior.h"
#include "Volt/AI/NavigationSystem.h"
#include "Volt/Components/NavigationComponents.h"
#include "Volt/Utility/Random.h"

namespace Volt
{
	gem::vec3 SteeringBehavior::Seek(Entity agent, gem::vec3 target)
	{
		gem::vec3 currentTarget;
		gem::vec3 desiredVelocity;
		gem::vec3 steeringForce;

		const auto& agentComp = agent.GetComponent<AgentComponent>();

		desiredVelocity = target - agent.GetWorldPosition();
		desiredVelocity = gem::normalize(desiredVelocity);
		desiredVelocity *= agentComp.maxVelocity;
		steeringForce = desiredVelocity - agentComp.myVelocity;
		steeringForce /= agentComp.maxVelocity;
		steeringForce *= agentComp.maxForce;

		return steeringForce;
	}

	gem::vec3 SteeringBehavior::Flee(Entity agent, gem::vec3 target)
	{
		gem::vec3 currentTarget;
		gem::vec3 desiredVelocity;
		gem::vec3 steeringForce;

		const auto& agentComp = agent.GetComponent<AgentComponent>();

		desiredVelocity = agent.GetWorldPosition() - target;
		desiredVelocity = gem::normalize(desiredVelocity);
		desiredVelocity *= agentComp.maxVelocity;
		steeringForce = desiredVelocity - agentComp.myVelocity;
		steeringForce /= agentComp.maxVelocity;
		steeringForce *= agentComp.maxForce;

		return steeringForce;
	}

	gem::vec3 SteeringBehavior::Arrive(Entity agent, gem::vec3 target)
	{
		return gem::vec3();
	}

	gem::vec3 SteeringBehavior::Align(Entity agent, gem::vec3 target)
	{
		return gem::vec3();
	}

	gem::vec3 SteeringBehavior::Pursue(Entity agent, gem::vec3 target)
	{
		return gem::vec3();
	}

	gem::vec3 SteeringBehavior::Evade(Entity agent, gem::vec3 target)
	{
		return gem::vec3();
	}

	gem::vec3 SteeringBehavior::Wander(Entity agent, gem::vec3 direction, float wanderRadius, float wanderOffset)
	{
		gem::vec3 steeringForce;

		if (direction != gem::vec3(0.f)) 
		{
			auto angle = Random::Float(0.f, 360.f);

			gem::vec3 wanderTarget;
			auto center = agent.GetWorldPosition() + gem::normalize(direction) * wanderOffset;
			wanderTarget.x = center.x + wanderRadius * gem::sin(angle);
			wanderTarget.z = center.z + wanderRadius * gem::cos(angle);

			steeringForce += Seek(agent, wanderTarget);
			agent.GetComponent<AgentComponent>().target = wanderTarget; // Maybe don't want to set this
		}

		return steeringForce;
	}

	gem::vec3 SteeringBehavior::PathFollowing(Entity agent, gem::vec3 target)
	{
		return gem::vec3();
	}

	gem::vec3 SteeringBehavior::Separation(Entity agent, float radius)
	{
		gem::vec3 steeringForce;
		float decayCoefficient = 1000.f;

		auto agentPositions = NavigationSystem::Get().myAgentPositions;

		for (const auto& entry : agentPositions)
		{
			if (entry.first == agent.GetId()) { continue; }
			const auto& entryPos = entry.second;
			if (auto distance = gem::distance(entryPos, agent.GetWorldPosition()); distance < radius)
			{
				auto strength = agent.GetComponent<AgentComponent>().maxForce * (radius - distance) / radius;
				steeringForce = gem::normalize(Flee(agent, entryPos)) * strength;
			}
		}

		return steeringForce;
	}

	gem::vec3 SteeringBehavior::CollisionAvoidance(Entity agent, gem::vec3 target)
	{
		return gem::vec3();
	}
}