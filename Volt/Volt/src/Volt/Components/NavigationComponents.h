#pragma once

#include "Volt/Asset/Asset.h"

#include <Wire/Serialization.h>
#include <gem/gem.h>

#include <optional>

namespace Volt
{
	SERIALIZE_ENUM((enum class AgentSteeringBehavior : uint32_t
	{
		Seek = 0,
		Flee,
		Arrive,
		Align,
		Pursue,
		Evade,
		Wander,
		PathFollowing,
		Separation,
		CollisionAvoidance,
	}), AgentSteeringBehavior);

	SERIALIZE_COMPONENT((struct AgentComponent
	{
		PROPERTY(Name = Max Velocity) float maxVelocity = 500.f;
		PROPERTY(Name = Max Force) float maxForce = 500.f;
		PROPERTY(Name = Steering Behavior, SpecialType = Enum) AgentSteeringBehavior steering = AgentSteeringBehavior::Seek;
		PROPERTY(Name = Kinematic) bool kinematic = true;

		gem::vec3 target;

		inline void StartNavigation() { myActive = true; };
		inline void StopNavigation() { myActive = false; };
		inline std::optional<gem::vec3> GetCurrentMilestone() const { if (myPath.empty()) { return std::optional<gem::vec3>(); } else { return myPath.back(); } }

	private:
		friend class NavigationSystem;
		friend class SteeringBehavior;

		std::vector<gem::vec3> myPath;
		gem::vec3 myVelocity;
		bool myActive = true;

	public:
		CREATE_COMPONENT_GUID("{F29BA549-DD7D-407E-8024-6E281C4ED2AC}"_guid);
	}), AgentComponent);
}
