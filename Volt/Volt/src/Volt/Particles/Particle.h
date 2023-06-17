#pragma once
#include "GEM/vector/vector.h"

namespace Volt
{
	struct Particle
	{
		Volt::AssetHandle texture = Volt::Asset::Null();

		std::vector<gem::vec4> colors;
		gem::vec4 color;

		gem::vec3 direction;
		gem::vec3 position;

		gem::vec3 gravity;
		gem::vec3 rotation;
		std::vector<gem::vec3> sizes;
		gem::vec3 size;

		float velocity;
		float startVelocity;
		float endVelocity;

		float totalLifeTime;
		float lifeTime;

		float distance;
		float randomValue = 0.f;
		float timeSinceSpawn = 0.f;

		bool dead = true;
	};
}
