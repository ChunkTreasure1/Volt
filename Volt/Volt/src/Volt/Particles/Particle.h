#pragma once

#include <CoreUtilities/Containers/Vector.h>

#include <glm/glm.hpp>

namespace Volt
{
	struct Particle
	{
		Volt::AssetHandle texture = Volt::Asset::Null();

		Vector<glm::vec4> colors;
		glm::vec4 color;

		glm::vec3 direction;
		glm::vec3 position;

		glm::vec3 gravity;
		glm::vec3 rotation;
		Vector<glm::vec3> sizes;
		glm::vec3 size;

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
