#include "vtpch.h"
#include "AudioComponents.h"

#include "Volt/Scene/SceneManager.h"

namespace Volt
{
	void AudioListenerComponent::OnCreate(AudioListenerComponent& component, entt::entity id)
	{
		if (!SceneManager::IsPlaying())
		{
			return;
		}

		Entity entity{ id, SceneManager::GetActiveScene() };

		component.m_id = entity.GetID();
		Amp::WwiseAudioManager::RegisterListener(static_cast<uint32_t>(entity.GetID()), std::to_string(static_cast<uint32_t>(entity.GetID())).c_str(), component.isDefault);
	}

	void AudioSourceComponent::OnCreate(AudioSourceComponent& component, entt::entity id)
	{
		if (!SceneManager::IsPlaying())
		{
			return;
		}

		Entity entity{ id, SceneManager::GetActiveScene() };

		component.m_id = entity.GetID();
		Amp::WwiseAudioManager::CreateAudioObject(static_cast<uint32_t>(entity.GetID()), "SpawnedObj");
	}
}
