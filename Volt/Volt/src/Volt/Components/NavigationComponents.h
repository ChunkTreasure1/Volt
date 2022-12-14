#pragma once

#include "Volt/Asset/Asset.h"
#include "Volt/AI/NavMeshAgent.h"

#include <Wire/Serialization.h>
#include <gem/gem.h>

namespace Volt
{
	SERIALIZE_COMPONENT((struct AgentComponent
	{
		PROPERTY(Name = Kinematic) bool kinematic = true;
		PROPERTY(Name = Max Velocity) float maxVelocity = 500.f;
		PROPERTY(Name = Max Force) float maxForce = 500.f;
		NavMeshAgent agent;

	//private:
	//	friend class NavigationSystem;

	//	gem::vec3 myVelocity = 0.f;
	//	bool myActive = true;

		CREATE_COMPONENT_GUID("{F29BA549-DD7D-407E-8024-6E281C4ED2AC}"_guid);
	}), AgentComponent);
}
