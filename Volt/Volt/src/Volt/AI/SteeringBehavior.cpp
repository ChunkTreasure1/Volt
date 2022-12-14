#include "vtpch.h"
#include "SteeringBehavior.h"
#include "Volt/Components/NavigationComponents.h"

namespace Volt
{
	gem::vec3 SteeringBehavior::Seek(Entity agent, gem::vec3 target)
	{
		gem::vec3 currentTarget;
		gem::vec3 desiredVelocity;
		gem::vec3 steeringForce;

		auto& agentComp = agent.GetComponent<AgentComponent>();

		desiredVelocity = target - agent.GetWorldPosition();
		desiredVelocity = gem::normalize(desiredVelocity);
		desiredVelocity *= agentComp.maxVelocity;
		steeringForce = desiredVelocity - agentComp.agent.GetVelocity();
		steeringForce /= agentComp.maxVelocity;
		steeringForce *= agentComp.maxForce;

		return steeringForce;
	}

	gem::vec3 SteeringBehavior::Flee(Entity agent, gem::vec3 target)
	{
		gem::vec3 currentTarget;
		gem::vec3 desiredVelocity;
		gem::vec3 steeringForce;

		auto& agentComp = agent.GetComponent<AgentComponent>();

		desiredVelocity = agent.GetWorldPosition() - target;
		desiredVelocity = gem::normalize(desiredVelocity);
		desiredVelocity *= agentComp.maxVelocity;
		steeringForce = desiredVelocity - agentComp.agent.GetVelocity();
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

	gem::vec3 SteeringBehavior::Wander(Entity agent, gem::vec3 target)
	{
		return gem::vec3();
	}

	gem::vec3 SteeringBehavior::PathFollowing(Entity agent, gem::vec3 target)
	{
		return gem::vec3();
	}

	gem::vec3 SteeringBehavior::Separation(Entity agent, gem::vec3 target)
	{
		return gem::vec3();
	}

	gem::vec3 SteeringBehavior::CollisionAvoidance(Entity agent, gem::vec3 target)
	{
		return gem::vec3();
	}
}