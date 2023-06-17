#pragma once

#include <gem/gem.h>
#include <array>

namespace Volt
{
	class Noise
	{
	public:

		enum NoiseType
		{
			Perlin,
			Simplex
		};

		static std::array<gem::vec4, 16> HBAOJitter();
		static const gem::vec2 GetTAAJitter(uint64_t frameIndex, const gem::vec2ui& viewportSize);

		static void Initialize();

		static float GetNoise(const float x, const float y, const float z);
		static float GetNoise(const float x, const float y, const float z, const float aTime);
		static float GetRandomSeed();

		static void SetFrequency(const float frequency);
		static void SetSeed(const int seed);
		static void SetNoiseType(NoiseType type);

	private:
		Noise() = delete;
	};
}
