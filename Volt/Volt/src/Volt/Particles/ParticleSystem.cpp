#include "vtpch.h"
#include "ParticleSystem.h"

#include "gem/gem.h"
#include "../Scene/Scene.h"
#include "Volt/Scene/Entity.h"
#include "Volt/Rendering/Renderer.h"

#include "Volt/Utility/SerializationMacros.h"
#include "Volt/Utility/YAMLSerializationHelpers.h"
#include "Volt/Utility/FileSystem.h"
#include "Volt/Utility/Random.h"

#include <Volt/Core/Base.h>
#include "Volt/Asset/AssetManager.h"
#include "Volt/Asset/Asset.h"
#include "Volt/Asset/ParticlePreset.h"
#include "Volt/Rendering/Shader/ShaderRegistry.h"
#include "Volt/Rendering/Shader/Shader.h"
#include "Volt/Rendering/Texture/Texture2D.h"
#include <yaml-cpp/yaml.h>
#include <fstream>
#include <filesystem>
#include <random>

Volt::ParticleSystem::ParticleSystem(Scene* s)
{
	myScene = s;
}

void Volt::ParticleSystem::Update(const float& deltaTime)
{
	auto& registry = myScene->GetRegistry();
	registry.ForEach<ParticleEmitterComponent, TransformComponent>([&](Wire::EntityId id, ParticleEmitterComponent& particleEmitterComponent, TransformComponent& transformComp)
		{
			if (particleEmitterComponent.preset == Asset::Null())
			{
				particleEmitterComponent.particles.resize(0);
				particleEmitterComponent.numberOfAliveParticles = 0;
				return;
			}
			Ref<ParticlePreset> preset = AssetManager::GetAsset<ParticlePreset>(particleEmitterComponent.preset);
			if (preset != nullptr)
			{
				if (particleEmitterComponent.preset != particleEmitterComponent.currentPreset)
				{
					particleEmitterComponent.currentPreset = particleEmitterComponent.preset;

					for (auto& p : particleEmitterComponent.particles)
						p.dead = true;

					particleEmitterComponent.particles.resize(100000);
					particleEmitterComponent.numberOfAliveParticles = 0;
				}

				if (particleEmitterComponent.pressedPlay == true)
				{
					for (auto& p : particleEmitterComponent.particles)
						p.dead = true;

					particleEmitterComponent.particles.resize(100000);
					particleEmitterComponent.numberOfAliveParticles = 0;
					particleEmitterComponent.pressedPlay = false;
				}

				particleEmitterComponent.isLooping = preset->isLooping;
				Volt::Entity ent = { id, myScene };
				

				if (transformComp.visible)
				{
					if (myTimeBtwSends <= 0)
					{
						while (myTimeBtwSends < 0)
						{
							if (particleEmitterComponent.isLooping)
							{
								particleEmitterComponent.emittionTimer = 1.f;
								SendParticles(particleEmitterComponent, ent.GetPosition(), deltaTime, -myTimeBtwSends);
							}
							else
							{
								if (particleEmitterComponent.emittionTimer >= 0)
								{
									SendParticles(particleEmitterComponent, ent.GetPosition(), deltaTime, -myTimeBtwSends);
								}
							}
							myTimeBtwSends += 1 / preset->intensity;
						}
						particleEmitterComponent.emittionTimer -= deltaTime;
						myTimeBtwSends = 1 / preset->intensity;
					}

					myTimeBtwSends -= deltaTime;

					auto& p_vec = particleEmitterComponent.particles;
					for (int index = 0; index < particleEmitterComponent.numberOfAliveParticles; index++)
					{
						// TODO: add updating stuff
						auto& p = p_vec[index];
						ParticlePositionUpdate(p, deltaTime);
						ParticleSizeUpdate(p, deltaTime);
						ParticleVelocityUpdate(p, deltaTime);
						ParticleColorUpdate(p, deltaTime);
						if (ParticleKillCheck(p, deltaTime))
						{
							std::swap(p, p_vec[particleEmitterComponent.numberOfAliveParticles - 1]);
							// -- ref
							particleEmitterComponent.numberOfAliveParticles--;
						}
					}
				}
			}
		}
	);
}

void Volt::ParticleSystem::RenderParticles()
{
	VT_PROFILE_FUNCTION();

	auto& registry = myScene->GetRegistry();
	registry.ForEach<ParticleEmitterComponent, TransformComponent>([&](Wire::EntityId id, ParticleEmitterComponent& particleEmitterComponent, const TransformComponent& transformComp)
		{
			auto e = AssetManager::GetAsset<ParticlePreset>(particleEmitterComponent.preset);
			if (transformComp.visible)
			{
				for (int i = 0; i < particleEmitterComponent.numberOfAliveParticles; i++)
				{
					auto& p = particleEmitterComponent.particles[i];
					Renderer::SubmitBillboard(AssetManager::GetAsset<Volt::Texture2D>(e->texture), p.position, p.size, id, p.color);
				}
			}
			if (particleEmitterComponent.preset != Asset::Null())
			{
				Ref<ParticlePreset> preset = AssetManager::GetAsset<ParticlePreset>(particleEmitterComponent.preset);
				if (preset && preset->IsValid())
				{
					Ref<Volt::Shader> shader = ShaderRegistry::Get(preset->shader);
					if (shader != nullptr && shader->IsValid())
						Renderer::DispatchBillboardsWithShader(shader);
				}
			}
		});
}

void Volt::ParticleSystem::SendParticles(ParticleEmitterComponent& particleEmitterComponent, gem::vec3 aEntityPos, const float& deltaTime, float aExtraLifeTime)
{
	auto e = AssetManager::GetAsset<ParticlePreset>(particleEmitterComponent.preset);

	if (particleEmitterComponent.particles.size() < particleEmitterComponent.numberOfAliveParticles) return;

	auto& p = particleEmitterComponent.particles[particleEmitterComponent.numberOfAliveParticles];

	// TODO: make different starting patterns <func>
	gem::vec3 dir;
	float radius = e->sphereRadius;
	if (e->shape == 0)
	{
		if (e->sphereSpawnOnEdge)
		{
			dir = gem::normalize(gem::vec3{ Volt::Random::Float(-radius, radius),Volt::Random::Float(-radius, radius),Volt::Random::Float(-radius, radius) });
			p.position = aEntityPos + dir * radius;
		}
		else
		{
			dir = gem::normalize(gem::vec3{ Volt::Random::Float(-radius, radius),Volt::Random::Float(-radius, radius),Volt::Random::Float(-radius, radius) });
			p.position = aEntityPos + dir * Volt::Random::Float(0, radius);
		}
	}

	p.dead = false;
	p.direction = dir;
	// temp	value  <--------->
	p.lifeTime = Volt::Random::Float(e->minLifeTime, e->maxLifeTime);
	p.totalLifeTime = p.lifeTime + aExtraLifeTime;

	p.startColor = e->startColor;
	p.endColor = e->endColor;
	p.color = e->startColor;

	p.size = e->startSize;
	p.startSize = p.size;
	p.endSize = e->endSize;


	p.velocity = e->startVelocity;
	p.startVelocity = p.velocity;
	p.endVelocity = e->endVelocity;
	p.gravity = e->gravity;


	particleEmitterComponent.numberOfAliveParticles++;
}

bool Volt::ParticleSystem::ParticleKillCheck(Particle& particle, const float& deltaTime)
{
	if (particle.lifeTime <= 0)
	{
		particle.dead = true;
	}
	else
	{
		particle.lifeTime -= deltaTime;
	}
	/*if (particle.distance > particle.endDistance && particle.endDistance > 0)
		particle.dead = true;*/
	return particle.dead;
}

void Volt::ParticleSystem::ParticlePositionUpdate(Particle& particle, const float& deltaTime)
{
	particle.position += particle.velocity * particle.direction * deltaTime;
	particle.distance += particle.velocity * deltaTime;
	particle.direction += particle.gravity * deltaTime;
}

void Volt::ParticleSystem::ParticleSizeUpdate(Particle& particle, const float& deltaTime)
{
	float sizePersentage = (particle.totalLifeTime - particle.lifeTime) / particle.totalLifeTime;
	gem::vec3 newSize = gem::lerp(particle.size, particle.endSize, sizePersentage);

	particle.size = newSize;
}

void Volt::ParticleSystem::ParticleVelocityUpdate(Particle& particle, const float& deltaTime)
{
	float velocityPersentage = (particle.totalLifeTime - particle.lifeTime) / particle.totalLifeTime;
	float newVelocity = gem::lerp(particle.velocity, particle.endVelocity, velocityPersentage);

	particle.velocity = newVelocity;
}

void Volt::ParticleSystem::ParticleColorUpdate(Particle& particle, const float& deltaTime)
{
	float colorPersentage = (particle.totalLifeTime - particle.lifeTime) / particle.totalLifeTime;
	float x = gem::lerp(particle.color.x, particle.endColor.x, colorPersentage);
	float y = gem::lerp(particle.color.y, particle.endColor.y, colorPersentage);
	float z = gem::lerp(particle.color.z, particle.endColor.z, colorPersentage);
	float w = gem::lerp(particle.color.w, particle.endColor.w, colorPersentage);

	particle.color = gem::vec4{ x,y,z,w };
}

void Volt::ParticleSystem::UpdateParticles(ParticleEmitterComponent& particleEmitterComponent, TransformComponent& transformComp, const float& deltaTime)
{
}
