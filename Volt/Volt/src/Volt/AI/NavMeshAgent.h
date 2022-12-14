#pragma once
#include "Volt/Scene/Entity.h"
#include <gem/gem.h>

namespace Volt
{
	enum class AgentSteeringBehaviors : uint32_t
	{
		Seek,
		Flee,
		Arrive,
		Align,
		Pursue,
		Evade,
		Wander,
		PathFollowing,
		Separation,
		CollisionAvoidance,
	};

	class NavMeshAgent
	{
	public:
		NavMeshAgent() = default;
		~NavMeshAgent() = default;

		void StartNavigation() { myActive = true; };
		void StopNavigation() { myActive = false; };

		void SetTarget(const gem::vec3& aPosition);
		void SetSteeringBehavior(const AgentSteeringBehaviors& behavior) { myBehavior = behavior; };

		AgentSteeringBehaviors GetSteeringBehavior() const { return myBehavior; };
		gem::vec3 GetVelocity() const { return myVelocity; };
		bool GetCurrentMilestone(gem::vec3* outMilestone = nullptr) const;

	private:
		friend class NavigationSystem;

		void Update(float aTimestep, Entity aEntity);
		void MoveToTarget(float aTimestep, Entity aEntity);
		gem::vec3 GetSteeringForce(Entity aEntity);
		void DrawDebugLines();

		gem::vec3 myCurrent;
		gem::vec3 myTarget;
		std::vector<gem::vec3> myPath;

		AgentSteeringBehaviors myBehavior = AgentSteeringBehaviors::Seek;
		gem::vec3 myVelocity = 0.f;

		bool myActive = true;
	};
}