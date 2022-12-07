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

		void SetSteeringBehavior(const AgentSteeringBehaviors& behavior) { myBehavior = behavior; };
		void SetKinematic(const bool& value) { myIsKinematic = value; };
		void SetTarget(const gem::vec3& aPosition);
		void SetSpeed(const float& aSpeed) { mySpeed = aSpeed; };

		float GetSpeed() const { return mySpeed; };
		bool GetCurrentMilestone(gem::vec3* outMilestone = nullptr) const;

	private:
		friend class NavigationSystem;

		void Update(float aTimestep, Entity aEntity);
		void MoveToTarget(float aTimestep, Entity aEntity);

		void Seek();
		void Flee();
		void Arrive();
		void Align();
		void Pursue();
		void Evade();
		void Wander();
		void PathFollowing();
		void Separation();
		void CollisionAvoidance();

		gem::vec3 myCurrent;
		gem::vec3 myTarget;
		std::vector<gem::vec3> myPath;
		float mySpeed = 250.f;

		AgentSteeringBehaviors myBehavior = AgentSteeringBehaviors::Seek;
		bool myIsKinematic = true;
		
		bool myActive = true;
	};
}