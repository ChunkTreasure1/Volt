#pragma once
#include "Volt/Asset/Asset.h"
#include "GEM/gem.h"

namespace Volt
{
	class ParticlePreset : public Asset
	{
	public:
		Volt::AssetHandle texture;
		std::string shader = "EntityGizmo";
		
		float distance = 0.f;
		float emittionTime = 1.f;

		uint32_t shape;

		float sphereRadius;
		bool sphereSpawnOnEdge = false;

		float coneInnerRadius;
		float coneOuterRadius;
		bool coneSpawnOnEdge = false;

		gem::vec3 startSize = 1.f;
		gem::vec3 endSize = 1.f;
		
		float startVelocity = 0.f;
		float endVelocity = 0.f;
		
		float intensity = 0.f;
		float minLifeTime = 1.f;
		float maxLifeTime = 1.f;

		float spread = 0.f;
		
		gem::vec3 direction = {1};
		gem::vec3 gravity = {0};

		gem::vec4 startColor = {1};
		gem::vec4 endColor = {1};
		
		bool isLooping = true;

		int poolSize = 0;

		static AssetType GetStaticType() { return AssetType::ParticlePreset; }
		virtual AssetType GetType() override { return GetStaticType(); }
	};
}