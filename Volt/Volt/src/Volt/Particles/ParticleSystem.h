#pragma once

#include <GEM/gem.h>
#include <Volt/Asset/Asset.h>

namespace Wire
{
	class Registry;
	typedef uint32_t EntityId;
}

namespace Volt
{
	struct ParticleEmitterComponent;
	struct TransformComponent;
	struct Particle;
	class Scene;
	class ParticlePreset;

	struct ParticleSystemInternalStorage
	{
		int numberOfAliveParticles = 0;
		std::vector<Particle> particles;
		AssetHandle preset;
	};

	class ParticleSystem
	{
	public:
		ParticleSystem() = default;
		void Update(Wire::Registry& registry, Scene* scene, float deltaTime);
		//void RenderParticles(Wire::Registry& registry);

		const auto& GetParticleStorage() const { return m_particleStorage; }

	private:
		void UpdateParticles(ParticleEmitterComponent& particleEmitterComponent, TransformComponent& transformComp, const float& deltaTime);
		void SendParticles(ParticleEmitterComponent& particleEmitterComponent, Wire::EntityId id, gem::vec3 aEntityPos, const float& deltaTime);

		bool ParticleKillCheck(Particle& particle, const float& deltaTime);
		void ParticlePositionUpdate(Particle& particle, Wire::EntityId id, Scene* scene, const float& deltaTime);
		void ParticleSizeUpdate(Particle& particle, const float& deltaTime);
		void ParticleVelocityUpdate(Particle& particle, const float& deltaTime);
		void ParticleColorUpdate(Particle& particle, const float& deltaTime);
		void ParticleTimeUpdate(Particle& particle, float deltaTime);

		std::unordered_map<Wire::EntityId, ParticleSystemInternalStorage> m_particleStorage;
	};
}
