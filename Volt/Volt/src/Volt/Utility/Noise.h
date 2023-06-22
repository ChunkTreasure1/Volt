#pragma once

#include <glm/glm.hpp>
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

		static std::array<glm::vec4, 16> HBAOJitter();
		static const glm::vec2 GetTAAJitter(uint64_t frameIndex, const glm::uvec2& viewportSize);

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
