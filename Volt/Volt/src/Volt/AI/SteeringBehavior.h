#pragma once
#include "Volt/Scene/Entity.h"
#include <gem/gem.h>

namespace Volt
{
	class SteeringBehavior
	{
	public:
		SteeringBehavior() = default;
		~SteeringBehavior() = default;

		static gem::vec3 Seek(Entity agent, gem::vec3 target);
		static gem::vec3 Flee(Entity agent, gem::vec3 target);
		static gem::vec3 Arrive(Entity agent, gem::vec3 target);
		static gem::vec3 Align(Entity agent, gem::vec3 target);
		static gem::vec3 Pursue(Entity agent, gem::vec3 target);
		static gem::vec3 Evade(Entity agent, gem::vec3 target);
		static gem::vec3 Wander(Entity agent, gem::vec3 target);
		static gem::vec3 PathFollowing(Entity agent, gem::vec3 target);
		static gem::vec3 Separation(Entity agent, gem::vec3 target);
		static gem::vec3 CollisionAvoidance(Entity agent, gem::vec3 target);
	};
}