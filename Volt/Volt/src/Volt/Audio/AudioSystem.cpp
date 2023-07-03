#include "vtpch.h"
#include "AudioSystem.h"

#include <glm/glm.hpp>
#include "../Scene/Scene.h"
#include "Volt/Scene/Entity.h"

#include <Volt/Components/Components.h>
#include <Volt/Components/AudioComponents.h>

#include <Volt/Rendering/DebugRenderer.h>
#include <Volt/Physics/Physics.h>
#include <Volt/Physics/PhysicsScene.h>

#include <Amp/WwiseAudioManager/WwiseAudioManager.h>

void Volt::AudioSystem::RuntimeStart(Wire::Registry& registry, Scene* scene)
{
	registry.ForEach<AudioSourceComponent>([&](Wire::EntityId id, AudioSourceComponent& audioSourceComp)
	{
		Volt::Entity entity({ id, scene });
		audioSourceComp.OnStart(entity);
	}
	);

	//TODO: Check for multiple AudioListners with default
	registry.ForEach<AudioListenerComponent>([&](Wire::EntityId id, AudioListenerComponent& audioListenerComp)
	{
		Volt::Entity entity({ id, scene });
		Amp::WwiseAudioManager::RegisterListener(entity.GetId(), entity.GetTag().c_str(), audioListenerComp.isDefault);	
	}
	);
}

void Volt::AudioSystem::RuntimeStop(Wire::Registry& registry, Scene* scene)
{
	registry.ForEach<AudioListenerComponent>([&](Wire::EntityId id, AudioListenerComponent&)
	{
		Volt::Entity entity({ id, scene });
		Amp::WwiseAudioManager::UnregisterListener();
	}
	);

	Amp::WwiseAudioManager::ClearAllObjects();
}

void Volt::AudioSystem::Update(Wire::Registry& registry, Scene* scene, const float& aDeltaTime)
{
	VT_PROFILE_FUNCTION();

	fixedUpdateTimer += aDeltaTime;
	UpdateAudioListeners(registry, scene, aDeltaTime);
	//UpdateAudioOcclusion(registry, scene);
	UpdateAudioSources(registry, scene, aDeltaTime);
}

void Volt::AudioSystem::OnEvent(Wire::Registry&, Volt::Event&)
{
}

void Volt::AudioSystem::UpdateAudioSources(Wire::Registry& registry, Scene* scene, const float&)
{
	registry.ForEach<AudioSourceComponent>([&](Wire::EntityId id, AudioSourceComponent& audioSourceComp)
	{
		Volt::Entity entity({ id, scene });
		Amp::WwiseAudioManager::SetObjectPosition(audioSourceComp.getID(), entity.GetPosition(), entity.GetForward(), entity.GetUp());
	}
	);
}

void Volt::AudioSystem::UpdateAudioListeners(Wire::Registry& registry, Scene* scene, const float&)
{
	registry.ForEach<AudioListenerComponent>([&](Wire::EntityId id, AudioListenerComponent&)
	{
		Volt::Entity entity({ id, scene });
		Amp::WwiseAudioManager::SetListenerPosition(entity.GetPosition(), entity.GetForward(), entity.GetUp());
	}
	);
}

//void Volt::AudioSystem::UpdateAudioOcclusion(Wire::Registry& registry, Scene* scene)
//{
//	if (fixedUpdateTimer < 0.25f) { return; }
//	fixedUpdateTimer = 0.f;
//
//	registry.ForEach<AudioListenerComponent>([&](Wire::EntityId id, AudioListenerComponent& audioListenerComp)
//	{
//		Volt::Entity entity({ id, scene });
//		if (audioListenerComp.isActive)
//		{
//			std::vector<Volt::Entity> audioSources;
//			registry.ForEach<AudioSourceComponent>([&](Wire::EntityId id, AudioSourceComponent& audioSourceComp)
//			{
//				Volt::Entity entity({ id, scene });
//				audioSources.push_back(entity);
//			}
//			);
//
//			bool debug = audioListenerComp.isDebug;
//
//			for (auto& AS : audioSources)
//			{
//				Volt::Entity targetEntity({ audioListenerComp.target, scene });
//				glm::vec3 targetPos = targetEntity.GetPosition();
//				targetPos = { targetPos.x ,targetPos.y + 50.f, targetPos.z };
//				glm::vec3 direction = targetPos - entity.GetPosition();
//				float distToObj = audioListenerComp.cameraDistance;
//
//				glm::vec3 listnerPos = { (entity.GetPosition() + (direction * distToObj)) };
//				glm::vec3 sourcePos = AS.GetPosition();
//
//				glm::vec3 listLeft = CalculatePoint(listnerPos, sourcePos, 100.f, true);
//				glm::vec3 listRight = CalculatePoint(listnerPos, sourcePos, 100.f, false);
//
//				glm::vec3 sourceLeft = CalculatePoint(sourcePos, listnerPos, 100.f, true);
//				glm::vec3 sourceRight = CalculatePoint(sourcePos, listnerPos, 100.f, false);
//
//				AudioSourceComponent& ASC = AS.GetComponent<AudioSourceComponent>();
//
//				float mostDist = 0;
//
//				for (auto& inst : ASC.instances3DMap)
//				{
//					float maxDist;
//					float minDist;
//					inst.second.EventDesc->getMinMaxDistance(&minDist, &maxDist);
//					if (mostDist < maxDist)
//					{
//						mostDist = maxDist;
//					}
//				}
//
//				float distance = glm::distance(sourcePos, listnerPos);
//
//				if (distance < mostDist)
//				{
//					RaycastHit hit;
//
//					int hitAmount = 0;
//					hitAmount += CastRay(listnerPos, sourcePos, debug);
//					hitAmount += CastRay(listnerPos, sourceLeft, debug);
//					hitAmount += CastRay(listnerPos, sourceRight, debug);
//					hitAmount += CastRay({ listnerPos.x,listnerPos.y + 100.f ,listnerPos.z }, sourcePos, debug);
//
//					hitAmount += CastRay(sourcePos, listLeft, debug);
//					hitAmount += CastRay(sourcePos, listRight, debug);
//
//
//					if (hitAmount > 0)
//					{
//						ASC.SetOcclusion(1.f / hitAmount);
//					}
//					else
//					{
//						ASC.SetOcclusion(1.f);
//					}
//				}
//			}
//		}
//	}
//	);
//
//}
//
//glm::vec3 Volt::AudioSystem::CalculatePoint(const glm::vec3& posA, const glm::vec3& posB, float soundWidth, bool isPositive)
//{
//	//Based on https://awaismunir.net/universal/tangents/3rd-third-vertext-calculate-right-angled-triangle.gif formula
//	float x;
//	float z;
//	glm::vec3 a = { posA.x,0.f, posA.z };
//	glm::vec3 b = { posB.x,0.f, posB.z };
//	float distance = glm::distance(a, b);
//	float mn = (soundWidth / distance);
//	if (isPositive)
//	{
//		x = a.x + (mn * (a.z - b.z));
//		z = a.z - (mn * (a.x - b.x));
//	}
//	else
//	{
//		x = a.x - (mn * (a.z - b.z));
//		z = a.z + (mn * (a.x - b.x));
//	}
//
//	return { x, posA.y, z };
//}

int Volt::AudioSystem::CastRay(const glm::vec3& posA, const glm::vec3& posB, bool debug)
{
	RaycastHit hit;
	if (Physics::GetScene()->Linecast(posA, posB, &hit, 1))
	{
		if (debug) DebugRenderer::DrawLine(posA, posB, { 1,0,0,1 });
		return 1;
	}
	else
	{
		if (debug) DebugRenderer::DrawLine(posA, posB, { 0,0,1,1 });
		return 0;
	}

}

