#include "vtpch.h"
#include "Volt/Components/AudioComponents.h"

#include "Volt/Scene/SceneManager.h"

namespace Volt
{
	void AudioListenerComponent::OnCreate(AudioEntity entity)
	{
		if (!SceneManager::IsPlaying())
		{
			return;
		}

		auto& component = entity.GetComponent<AudioListenerComponent>();

		component.m_id = entity.GetID();
		Amp::WwiseAudioManager::RegisterListener(static_cast<uint32_t>(entity.GetID()), std::to_string(static_cast<uint32_t>(entity.GetID())).c_str(), component.isDefault);
	}

	void AudioSourceComponent::OnCreate(AudioEntity entity)
	{
		if (!SceneManager::IsPlaying())
		{
			return;
		}

		auto& component = entity.GetComponent<AudioSourceComponent>();

		component.m_id = entity.GetID();
		Amp::WwiseAudioManager::CreateAudioObject(static_cast<uint32_t>(entity.GetID()), "SpawnedObj");
	}
}
