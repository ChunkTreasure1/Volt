#pragma once
#include "Volt/Asset/Asset.h"
#include "Volt/Asset/AssetManager.h"
#include "Volt/Asset/Mesh/Mesh.h"

#include "Volt/Scene/Scene.h"
#include "Volt/Scene/Entity.h"
#include "Volt/Scene/Serialization/ComponentReflection.h"
#include "Volt/Scene/Serialization/ComponentRegistry.h"

#include "Volt/Log/Log.h"
#include "Volt/Events/ApplicationEvent.h"

#include <Amp/WwiseAudioManager/WwiseAudioManager.h>

#include <entt.hpp>
#include <glm/glm.hpp>

#include<unordered_map> 
#include<string>

namespace Volt
{
	struct AudioListenerComponent
	{
		bool isDefault = true;

		void OnCreate(entt::entity entityID)
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
		entt::entity m_id = entt::null;
	};

	struct AudioSourceComponent
	{
		void OnCreate(entt::entity entityID)
		{
			m_id = entityID;
			Amp::WwiseAudioManager::CreateAudioObject(static_cast<uint32_t>(m_id), "SpawnedObj");
		}

		void OnStart(Volt::Entity entity)
		{
			m_id = entity.GetID();
			Amp::WwiseAudioManager::CreateAudioObject(static_cast<uint32_t>(m_id), entity.GetTag().c_str());
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
		entt::entity m_id = entt::null;
	};

}
