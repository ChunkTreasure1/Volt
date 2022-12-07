#pragma once
#include "../Components/Components.h"

namespace Volt
{
class Scene;

	class ParticleSystem {
	public:
		ParticleSystem(Scene*);
		void Update(const float& deltaTime);
		void RenderParticles();
	private:
		void SendParticles(ParticleEmitterComponent& particleEmitterComponent, gem::vec3 aEntityPos, const float& deltaTime, float aExtraLifeTime);
		bool ParticleKillCheck(Particle& particle, const float& deltaTime);
		void ParticlePositionUpdate(Particle& particle, const float& deltaTime);
		void ParticleSizeUpdate(Particle& particle, const float& deltaTime);
		void ParticleVelocityUpdate(Particle& particle, const float& deltaTime);
		void ParticleColorUpdate(Particle& particle, const float& deltaTime);

		void UpdateParticles(ParticleEmitterComponent& particleEmitterComponent, TransformComponent& transformComp, const float& deltaTime);
		Scene* myScene;

		float myTimeBtwSends =0.f;
	};
}