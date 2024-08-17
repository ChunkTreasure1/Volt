#pragma once
#include <AssetSystem/Asset.h>
#include <AssetSystem/AssetManager.h>
#include "Volt/Asset/Mesh/Mesh.h"

#include "Volt/Scene/Scene.h"
#include "Volt/Scene/Entity.h"

#include <EntitySystem/ComponentRegistry.h>

#include <Amp/WwiseAudioManager/WwiseAudioManager.h>

#include <glm/glm.hpp>

#include<unordered_map> 
#include<string>

namespace Volt
{
	struct AudioListenerComponent
	{
		bool isDefault = true;

		void OnCreate(EntityID entityID)
		{
			m_id = entityID;
			Amp::WwiseAudioManager::RegisterListener(static_cast<uint32_t>(m_id), std::to_string(static_cast<uint32_t>(m_id)).c_str(), isDefault);
		}

		static void ReflectType(TypeDesc<AudioListenerComponent>& reflect)
		{
			reflect.SetGUID("{C540C160-3AF7-4C5D-A0E0-7121F27C7DA5}"_guid);
			reflect.SetLabel("Audio Listener Component");
			reflect.AddMember(&AudioListenerComponent::isDefault, "default", "Default", "", true);
		}

		REGISTER_COMPONENT(AudioListenerComponent);

	private:
		EntityID m_id = Entity::NullID();
	};

	struct AudioSourceComponent
	{
		void OnCreate(EntityID entityID)
		{
			Amp::WwiseAudioManager::CreateAudioObject(static_cast<uint32_t>(entityID), "SpawnedObj");
			m_id = entityID;
		}

		void OnStart(Volt::Entity entity)
		{
			Amp::WwiseAudioManager::CreateAudioObject(entity.GetID(), entity.GetTag().c_str());
			m_id = entity.GetID();
		}

		bool PlayOneshotEvent(const char* aEventName, const glm::vec3& aPosition, const glm::vec3& aForward, const glm::vec3& aUp)
		{
			return Amp::WwiseAudioManager::PlayOneShotEvent(aEventName, aPosition, aForward, aUp);
		}

		bool PlayEvent(const char* aEventName, uint32_t& aPlayingID)
		{
			return Amp::WwiseAudioManager::PlayEvent(aEventName, static_cast<uint32_t>(m_id), aPlayingID);
		}

		bool StopEvent(const uint32_t& aPlayingID)
		{
			return Amp::WwiseAudioManager::StopEvent(aPlayingID);
		}

		void StopAllEvents()
		{
			Amp::WwiseAudioManager::StopAllEvents(static_cast<uint32_t>(m_id));
		}

		bool PauseEvent(const uint32_t& aPlayingID)
		{
			return Amp::WwiseAudioManager::PauseEvent(aPlayingID);
		}

		bool ResumeEvent(const uint32_t& aPlayingID)
		{
			return Amp::WwiseAudioManager::ResumeEvent(aPlayingID);
		}

		bool SetState(const char* aStateGroup, const char* aState)
		{
			return Amp::WwiseAudioManager::SetState(aStateGroup, aState);
		}

		bool SetSwitch(const char* aSwitchGroup, const char* aState)
		{
			return Amp::WwiseAudioManager::SetSwitch(aSwitchGroup, aState, static_cast<uint32_t>(m_id));
		}

		bool SetParameter(const char* aParameterName, const float& aValue)
		{
			return Amp::WwiseAudioManager::SetParameter(aParameterName, aValue, static_cast<uint32_t>(m_id));
		}

		bool SetParameter(const char* aParameterName, const float& aValue, const uint32_t& aOvertime)
		{
			return Amp::WwiseAudioManager::SetParameter(aParameterName, aValue, static_cast<uint32_t>(m_id), aOvertime);
		}

		uint32_t getID() { return static_cast<uint32_t>(m_id); }

		static void ReflectType(TypeDesc<AudioSourceComponent>& reflect)
		{
			reflect.SetGUID("{06A69F94-BB09-4A3A-AF17-C9DA7D552BFE}"_guid);
			reflect.SetLabel("Audio Source Component");
		}

		REGISTER_COMPONENT(AudioSourceComponent);

	private:
		EntityID m_id = Entity::NullID();
	};

}
