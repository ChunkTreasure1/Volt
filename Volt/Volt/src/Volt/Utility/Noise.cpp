#include "vtpch.h"
#include "Noise.h"

#include <random>

namespace Volt
{
	static std::uniform_real_distribution<float> globalJitters(0.f, 1.f);
	static std::default_random_engine globalJitterGenerator(1337u);

	std::array<gem::vec4, 16> Noise::HBAOJitter()
	{
		constexpr float PI = 3.14159265358979323846264338f;
		const float numDir = 8.f;  // keep in sync to hlsl

		std::array<gem::vec4, 16> result{};

		for (int32_t i = 0; i < 16; i++)
		{
			float rand1 = globalJitters(globalJitterGenerator);
			float rand2 = globalJitters(globalJitterGenerator);
		
			const float angle = 2.f * PI * rand1 / numDir;
			result[i].x = cosf(angle);
			result[i].y = sinf(angle);
			result[i].z = rand2;
			result[i].w = 0.f;
		}

		return result;
	}
}

