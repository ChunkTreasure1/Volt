#pragma once
#include "Volt/Asset/Asset.h"
#include "gem/gem.h"

namespace Volt
{
	class ParticlePreset : public Asset
	{
	public:
		enum class eType : int
		{
			MESH = 0,
			PARTICLE
		} type = eType::PARTICLE;

		Volt::AssetHandle mesh = 0;
		Volt::AssetHandle material = 0;
		Volt::AssetHandle texture = 0;

		float distance = 1.f;
		float emittionTime = 1.f;

		uint32_t shape;

		float sphereRadius = 2;
		bool sphereSpawnOnEdge = false;

		float coneInnerRadius;
		float coneOuterRadius;
		bool coneSpawnOnEdge = false;

		std::vector<gem::vec3> sizes;
		std::vector<gem::vec4> colors;

		float startVelocity = 0.f;
		float endVelocity = 0.f;

		float intensity = 1.f;
		float minLifeTime = 1.f;
		float maxLifeTime = 1.f;

		gem::vec3 direction = { 1 };
		gem::vec3 gravity = { 0 };

		bool isLooping = true;

		bool isBurst = false;
		float burstInterval = 0;
		float burstLength = 0;

		int poolSize = 0;

		static AssetType GetStaticType() { return AssetType::ParticlePreset; }
		AssetType GetType() override { return GetStaticType(); }
	};
}
