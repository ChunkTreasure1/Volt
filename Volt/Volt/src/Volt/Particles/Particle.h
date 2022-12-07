#pragma once
#include "GEM/vector/vector.h"

namespace Volt{
	struct Particle{

		Volt::AssetHandle texture = Volt::Asset::Null();

		gem::vec4 startColor;
		gem::vec4 endColor;
		gem::vec4 color;

		gem::vec3 direction;
		gem::vec3 startPosition;
		gem::vec3 endPosition;
		gem::vec3 position;

		gem::vec3 gravity;

		gem::vec3 size;
		gem::vec3 startSize;
		gem::vec3 endSize;

		float velocity;
		float startVelocity;
		float endVelocity;

		float totalLifeTime;
		float lifeTime;

		float distance;
		float endDistance;

		bool dead = true;
	};
}