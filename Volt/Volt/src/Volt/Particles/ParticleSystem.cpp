#include "vtpch.h"
#include "ParticleSystem.h"

#include <glm/glm.hpp>
#include "Volt/Scene/Entity.h"
#include "Volt/Scene/Scene.h"

#include "Volt/Utility/YAMLSerializationHelpers.h"
#include "Volt/Utility/FileSystem.h"
#include "Volt/Utility/Random.h"

#include "Volt/Components/RenderingComponents.h"
#include "Volt/Components/CoreComponents.h"

#include <Volt/Core/Base.h>
#include "Volt/Asset/AssetManager.h"
#include "Volt/Asset/Asset.h"
#include "Volt/Asset/ParticlePreset.h"
#include "Volt/Rendering/Texture/Texture2D.h"
#include <yaml-cpp/yaml.h>
#include <fstream>
#include <filesystem>
#include <random>

#include "Volt/Core/Application.h"

#include <JobSystem/JobSystem.h>

void Volt::ParticleSystem::Update(entt::registry& registry, Weak<Scene> scene, const float deltaTime)
{
	VT_PROFILE_FUNCTION();

	std::set<entt::entity> emittersAliveThisFrame;
	
	auto view = registry.view<ParticleEmitterComponent, TransformComponent>();
	view.each([&](const entt::entity id, ParticleEmitterComponent& particleEmitterComponent, TransformComponent& transformComp) 
	{
		if (particleEmitterComponent.preset == Asset::Null())
		{
			m_particleStorage[id].particles.resize(0);
			m_particleStorage[id].numberOfAliveParticles = 0;
			return;
		}
		Ref<ParticlePreset> preset = AssetManager::GetAsset<ParticlePreset>(particleEmitterComponent.preset);
		if (preset != nullptr)
		{
			if (particleEmitterComponent.preset != particleEmitterComponent.currentPreset)
			{
				particleEmitterComponent.currentPreset = particleEmitterComponent.preset;

				for (auto& p : m_particleStorage[id].particles)
					p.dead = true;

				m_particleStorage[id].particles.resize(10000);
				m_particleStorage[id].numberOfAliveParticles = 0;
				m_particleStorage[id].preset = particleEmitterComponent.currentPreset;
				particleEmitterComponent.emissionTimer = preset->emittionTime;
				particleEmitterComponent.internalTimer = 0;
			}
			emittersAliveThisFrame.insert(id);
			particleEmitterComponent.isLooping = preset->isLooping;
			Volt::Entity ent = { id, scene };
			if (transformComp.visible)
			{
				if (particleEmitterComponent.isLooping || particleEmitterComponent.emissionTimer > 0)
				{
					particleEmitterComponent.internalTimer += deltaTime;
					particleEmitterComponent.emissionTimer -= deltaTime;
					if (preset->isBurst)
					{
						particleEmitterComponent.burstTimer -= deltaTime;
						if (particleEmitterComponent.burstTimer < -preset->burstLength)
						{
							particleEmitterComponent.burstTimer = preset->burstInterval;
						}
					}
					if (!preset->isBurst || particleEmitterComponent.burstTimer < 0)
					{
						SendParticles(particleEmitterComponent, id, ent.GetPosition(), 1 / preset->intensity);
					}
				}
			}
		}
	});

	Vector<entt::entity> emittersToRemove{};
	Vector<std::future<void>> futures{};
	std::mutex emittersToRemoveMutex;

	for (auto& [id, particleStorage] : m_particleStorage)
	{
		futures.emplace_back(JobSystem::SubmitTask([&]() 
		{
			VT_PROFILE_SCOPE("Update particle system");

			Entity entity{ id, scene };
			if (!entity)
			{
				std::scoped_lock lock{ emittersToRemoveMutex };
				emittersToRemove.emplace_back(id);
				return;
			}

			const auto forward = entity.GetForward();

			Vector<Particle>& p_vec = particleStorage.particles;
			for (int index = 0; index < particleStorage.numberOfAliveParticles; index++)
			{
				Particle& p = p_vec[index];

				if (ParticleKillCheck(p, deltaTime))
				{
					std::swap(p, p_vec[particleStorage.numberOfAliveParticles - 1]);
					particleStorage.numberOfAliveParticles--;
					continue;
				}

				ParticlePositionUpdate(p, deltaTime, forward);
				ParticleSizeUpdate(p, deltaTime);
				ParticleVelocityUpdate(p, deltaTime);
				ParticleColorUpdate(p, deltaTime);
				ParticleTimeUpdate(p, deltaTime);
			}
			if (particleStorage.numberOfAliveParticles == 0)
			{
				if (!emittersAliveThisFrame.contains(id))
				{
					std::scoped_lock lock{ emittersToRemoveMutex };
					emittersToRemove.emplace_back(id);
				}
			}
		}));
	}

	for (auto& future : futures)
	{
		future.wait();
	}

	for (const auto& emitter : emittersToRemove)
	{
		m_particleStorage.erase(emitter);
	}
}

void Volt::ParticleSystem::SendParticles(ParticleEmitterComponent& particleEmitterComponent, entt::entity id, glm::vec3 aEntityPos, const float& intencity)
{
	auto e = AssetManager::GetAsset<ParticlePreset>(particleEmitterComponent.preset);
	while (particleEmitterComponent.internalTimer > 0)
	{
		if (m_particleStorage[id].particles.size() <= m_particleStorage[id].numberOfAliveParticles || e->colors.empty())
			return;

		auto& p = m_particleStorage[id].particles[m_particleStorage[id].numberOfAliveParticles];

		glm::vec3 dir = { 0.f, 1.f, 0.f };
		float radius = e->sphereRadius;
		if (e->shape == 0)
		{
			// #mmax: make different starting patterns <func>
			if (e->sphereSpawnOnEdge)
			{
				dir = glm::normalize(glm::vec3{ Volt::Random::Float(-radius, radius),Volt::Random::Float(-radius, radius),Volt::Random::Float(-radius, radius) });
				p.position = aEntityPos + dir * radius;
			}
			else
			{
				dir = glm::normalize(glm::vec3{ Volt::Random::Float(-radius, radius),Volt::Random::Float(-radius, radius),Volt::Random::Float(-radius, radius) });
				p.position = aEntityPos + dir * Volt::Random::Float(0, radius);
			}
		}

		p.dead = false;
		p.direction = dir;

		p.lifeTime = Volt::Random::Float(e->minLifeTime, (e->minLifeTime >= e->maxLifeTime) ? e->minLifeTime : e->maxLifeTime);
		p.totalLifeTime = p.lifeTime;

		p.colors = e->colors;
		p.color = e->colors[0];
		p.texture = e->texture;

		p.sizes = e->sizes;
		p.size = e->sizes[0];
		p.rotation = { Volt::Random::Float(0, 6.28318531f),Volt::Random::Float(0, 6.28318531f),Volt::Random::Float(0, 6.28318531f) };

		p.velocity = e->startVelocity;
		p.startVelocity = p.velocity;
		p.endVelocity = e->endVelocity;
		p.gravity = e->gravity;

		p.randomValue = Random::Float(0.f, 1.f);
		p.timeSinceSpawn = 0.f;

		m_particleStorage[id].numberOfAliveParticles++;
		particleEmitterComponent.internalTimer -= intencity;
	}
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
	return particle.dead;
}

void Volt::ParticleSystem::ParticlePositionUpdate(Particle& particle, const float& deltaTime, const glm::vec3& entityForward)
{
	const glm::vec3 dir = glm::normalize(entityForward + particle.direction);

	particle.position += particle.velocity * dir * deltaTime;
	particle.distance += particle.velocity * deltaTime;
	particle.direction += particle.gravity * deltaTime;
}

void Volt::ParticleSystem::ParticleSizeUpdate(Particle& particle, const float&)
{
	float sizePersentage = (particle.totalLifeTime - particle.lifeTime) / particle.totalLifeTime;
	if (sizePersentage > 1)
		sizePersentage = 1;
	float scaledTime = sizePersentage * ((float)particle.sizes.size() - 1);
	//glm::vec4 colorArray[3] = { particle.startColor, particle.middleColor, particle.endColor };
	glm::vec4 oldSize = glm::vec4{ particle.sizes[(int)scaledTime], 0.f };
	auto newScaled = (int)(scaledTime + 1.0f);
	glm::vec4 newSize = glm::vec4{ particle.sizes[(newScaled >= particle.sizes.size()) ? particle.sizes.size() - 1 : newScaled], 1.f };
	float newT = scaledTime - (int)(scaledTime);

	float x = glm::mix(oldSize.x, newSize.x, newT);
	float y = glm::mix(oldSize.y, newSize.y, newT);
	float z = glm::mix(oldSize.z, newSize.z, newT);

	particle.size = glm::vec3{ x,y,z };

	//float w = glm::mix(oldColor.w, newColor.w, newT);


	/*float sizePersentage = (particle.totalLifeTime - particle.lifeTime) / particle.totalLifeTime;
	glm::vec3 newSize = glm::mix(particle.startSize, particle.endSize, sizePersentage);

	particle.size = newSize;*/
}

void Volt::ParticleSystem::ParticleVelocityUpdate(Particle& particle, const float&)
{
	float velocityPersentage = (particle.totalLifeTime - particle.lifeTime) / particle.totalLifeTime;
	float newVelocity = glm::mix(particle.startVelocity, particle.endVelocity, velocityPersentage);

	particle.velocity = newVelocity;
}


void Volt::ParticleSystem::ParticleColorUpdate(Particle& particle, const float&)
{
	float colorPersentage = (particle.totalLifeTime - particle.lifeTime) / particle.totalLifeTime;
	if (colorPersentage > 1)
		colorPersentage = 1;
	float scaledTime = colorPersentage * ((float)particle.colors.size() - 1);
	//glm::vec4 colorArray[3] = { particle.startColor, particle.middleColor, particle.endColor };
	glm::vec4 oldColor = particle.colors[(int)scaledTime];
	auto newScaled = (int)(scaledTime + 1.0f);
	glm::vec4 newColor = particle.colors[(newScaled >= particle.colors.size()) ? particle.colors.size() - 1 : newScaled];
	float newT = scaledTime - (int)(scaledTime);

	float x = glm::mix(oldColor.x, newColor.x, newT);
	float y = glm::mix(oldColor.y, newColor.y, newT);
	float z = glm::mix(oldColor.z, newColor.z, newT);
	float w = glm::mix(oldColor.w, newColor.w, newT);

	particle.color = glm::vec4{ x,y,z,w };
}

void Volt::ParticleSystem::ParticleTimeUpdate(Particle& particle, float deltaTime)
{
	particle.timeSinceSpawn += deltaTime;
}

void Volt::ParticleSystem::UpdateParticles(ParticleEmitterComponent&, TransformComponent&, const float&)
{
}
