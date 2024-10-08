#include "vtpch.h"
#include "Volt/Utility/Noise.h"
#include "Volt/Utility/FastNoise.h"

#include <CoreUtilities/Random.h>

#include <random>

namespace Volt
{
	static FastNoiseLite s_noise;
	static std::uniform_real_distribution<float> s_globalJitters(0.f, 1.f);
	static std::default_random_engine s_globalJitterGenerator(1337u);

	inline static float s_haltonX[8];
	inline static float s_haltonY[8];

	std::array<glm::vec4, 16> Noise::HBAOJitter()
	{
		constexpr float PI = 3.14159265358979323846264338f;
		const float numDir = 8.f;  // keep in sync to hlsl

		std::array<glm::vec4, 16> result{};

		for (int32_t i = 0; i < 16; i++)
		{
			float rand1 = s_globalJitters(s_globalJitterGenerator);
			float rand2 = s_globalJitters(s_globalJitterGenerator);

			const float angle = 2.f * PI * rand1 / numDir;
			result[i].x = cosf(angle);
			result[i].y = sinf(angle);
			result[i].z = rand2;
			result[i].w = 0.f;
		}

		return result;
	}

	const glm::vec2 Noise::GetTAAJitter(uint64_t frameIndex, const glm::uvec2& viewportSize)
	{
		return { s_haltonX[frameIndex % 8] / static_cast<float>(viewportSize.x), s_haltonY[frameIndex % 8] / static_cast<float>(viewportSize.y) };
	}

	void Noise::Initialize()
	{
		auto halton = [](size_t index, size_t base)
		{
			float f = 1.0f;
			float r = 0.0f;

			while (index > 0)
			{
				f /= base;
				r += f * (index % base);
				index /= base;
			}

			return r;
		};

		for (size_t i = 0; i < 8; ++i)
		{
			s_haltonX[i] = halton(i + 1, 2) * 2.0f - 1.0f;
			s_haltonY[i] = halton(i + 1, 3) * 2.0f - 1.0f;
		}
	}

	float Noise::GetNoise(const float x, const float y, const float z)
	{
		return s_noise.GetNoise(x, y, z);
	}

	float Noise::GetNoise(const float x, const float y, const float z, const float aTime)
	{
		return s_noise.GetNoise(x, y, z);
	}

	float Noise::GetRandomSeed()
	{
		return Random::Float(1, 10000);
	}

	void Noise::SetFrequency(const float frequency)
	{
		s_noise.SetFrequency(frequency);
	}

	void Noise::SetSeed(const int seed)
	{
		s_noise.SetSeed(seed);
	}

	void Noise::SetNoiseType(NoiseType type)
	{
		switch (type)
		{
			case Volt::Noise::Perlin:
				s_noise.SetNoiseType(FastNoiseLite::NoiseType_Perlin);
				break;
			case Volt::Noise::Simplex:
				s_noise.SetNoiseType(FastNoiseLite::NoiseType_OpenSimplex2);
				break;
			default:
				break;
		}

		//Add more types if needed
	}

}

