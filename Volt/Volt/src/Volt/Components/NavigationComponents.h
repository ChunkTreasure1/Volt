#pragma once

#include "Volt/Asset/Asset.h"
#include "Volt/AI/SteeringBehavior.h"

#include <Wire/Serialization.h>
#include <gem/gem.h>

#include <optional>

namespace Volt
{
	// Make sure to set steering force in scripts using functions from SteeringBehavior class or agents won't move.
	// Example Usage:
	// force = SteeringBehavior::Seek(agent, target);
	// force += SteeringBehavior::Flee(agent, target);
	// agent.steeringForce = force;

	SERIALIZE_COMPONENT((struct AgentComponent
	{
		PROPERTY(Name = Max Velocity) float maxVelocity = 500.f;
		PROPERTY(Name = Max Force) float maxForce = 500.f;
		PROPERTY(Name = Kinematic) bool kinematic = true;

		gem::vec3 steeringForce = gem::vec3(0.f);
		gem::vec3 target = gem::vec3(0.f);

		inline void StartNavigation() { myActive = true; };
		inline void StopNavigation() { myActive = false; };
		inline std::optional<gem::vec3> GetCurrentMilestone() const { if (myPath.empty()) { return std::optional<gem::vec3>(); } else { return myPath.back(); } }

	private:
		friend class NavigationSystem;
		friend class SteeringBehavior;

		std::vector<gem::vec3> myPath;
		gem::vec3 myVelocity = gem::vec3(0.f);
		bool myActive = true;

	public:
		CREATE_COMPONENT_GUID("{F29BA549-DD7D-407E-8024-6E281C4ED2AC}"_guid);
	}), AgentComponent);
}
