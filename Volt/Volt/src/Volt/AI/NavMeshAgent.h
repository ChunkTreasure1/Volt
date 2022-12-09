#pragma once
#include "Volt/Scene/Entity.h"
#include <gem/gem.h>

namespace Volt
{

	enum AgentSteeringBehaviors : uint32_t
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
		void SetKinematic(const bool& value) { myIsKinematic = value; };
		void SetMaxVelocity(const float& aMaxVelocity) { myMaxVelocity = aMaxVelocity; };
		void SetMaxForce(const float& aMaxForce) { myMaxForce = aMaxForce; };

		AgentSteeringBehaviors GetSteeringBehavior() const { return myBehavior; };
		bool GetKinematic() const { return myIsKinematic; };
		float GetMaxVelocity() const { return myMaxVelocity; };
		float GetMaxForce() const { return myMaxForce; };
		bool GetCurrentMilestone(gem::vec3* outMilestone = nullptr) const;

	private:
		friend class NavigationSystem;

		void Update(float aTimestep, Entity aEntity);
		void MoveToTarget(float aTimestep, Entity aEntity);
		gem::vec3 GetSteeringForce();
		void DrawDebugLines();

		gem::vec3 Seek();
		gem::vec3 Flee();
		gem::vec3 Arrive();
		gem::vec3 Align();
		gem::vec3 Pursue();
		gem::vec3 Evade();
		gem::vec3 Wander();
		gem::vec3 PathFollowing();
		gem::vec3 Separation();
		gem::vec3 CollisionAvoidance();

		gem::vec3 myCurrent;
		gem::vec3 myTarget;
		std::vector<gem::vec3> myPath;

		AgentSteeringBehaviors myBehavior = AgentSteeringBehaviors::Seek;
		bool myIsKinematic = true;
		gem::vec3 myVelocity = 0.f;
		float myMaxVelocity = 500.f;
		float myMaxForce = 500.f;

		bool myActive = true;
	};
}