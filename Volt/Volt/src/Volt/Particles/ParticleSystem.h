#pragma once

#include "Volt/Asset/Asset.h"
#include "Volt/Particles/Particle.h"

#include <entt.hpp>
#include <glm/glm.hpp>

#include <shared_mutex>

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
		Vector<Particle> particles;
		AssetHandle preset;
	};

	class ParticleSystem
	{
	public:
		ParticleSystem() = default;
		void Update(entt::registry& registry, Weak<Scene> scene, float deltaTime);
		//void RenderParticles(Wire::Registry& registry);

		const auto& GetParticleStorage() const { return m_particleStorage; }

	private:
		void UpdateParticles(ParticleEmitterComponent& particleEmitterComponent, TransformComponent& transformComp, const float& deltaTime);
		void SendParticles(ParticleEmitterComponent& particleEmitterComponent, entt::entity id, glm::vec3 aEntityPos, const float& deltaTime);

		bool ParticleKillCheck(Particle& particle, const float& deltaTime);
		void ParticlePositionUpdate(Particle& particle, const float& deltaTime, const glm::vec3& entityForward);
		void ParticleSizeUpdate(Particle& particle, const float& deltaTime);
		void ParticleVelocityUpdate(Particle& particle, const float& deltaTime);
		void ParticleColorUpdate(Particle& particle, const float& deltaTime);
		void ParticleTimeUpdate(Particle& particle, float deltaTime);

		std::unordered_map<entt::entity, ParticleSystemInternalStorage> m_particleStorage;

		std::shared_mutex m_updateMutex;
	};
}
