#pragma once
#include "Volt/Asset/Asset.h"
#include "Volt/Scene/Scene.h"
#include "Volt/Scene/Entity.h"

#include "Volt/Components/Components.h"
#include "Volt/Asset/AssetManager.h"
#include "Volt/Asset/Mesh/Mesh.h"

#include <Wire/Serialization.h>
#include <gem/gem.h>

#include <Volt/Events/ApplicationEvent.h>

#include<unordered_map> 
#include<string>

#include <Volt/Log/Log.h>
#include <Amp/WwiseAudioManager/WwiseAudioManager.h>


namespace Volt
{
	SERIALIZE_COMPONENT((struct AudioListenerComponent
	{
		PROPERTY(Name = Default) bool isDefault = true;

	public:
		void OnCreate(Wire::EntityId entityID)
		{
			myID = entityID;
			Amp::WwiseAudioManager::RegisterListener(myID, std::to_string(myID).c_str(), isDefault);
		}

		CREATE_COMPONENT_GUID("{C540C160-3AF7-4C5D-A0E0-7121F27C7DA5}"_guid);
	private:
		uint32_t myID;

	}), AudioListenerComponent);

	SERIALIZE_COMPONENT((struct AudioSourceComponent
	{
		//FUNC

	public:
		void OnCreate(Wire::EntityId entityID)
		{
			myID = entityID;
			Amp::WwiseAudioManager::CreateAudioObject(myID, "SpawnedObj");
		}

		void OnStart(Volt::Entity entity)
		{
			myID = entity.GetId();
			Amp::WwiseAudioManager::CreateAudioObject(myID, entity.GetTag().c_str());
		}

		bool PlayOneshotEvent(const char* aEventName, const gem::vec3& aPosition, const gem::vec3& aForward, const gem::vec3& aUp)
		{
			return Amp::WwiseAudioManager::PlayOneShotEvent(aEventName, aPosition, aForward, aUp);
		}

		bool PlayEvent(const char* aEventName, uint32_t& aPlayingID)
		{
			return Amp::WwiseAudioManager::PlayEvent(aEventName, myID, aPlayingID);
		}

		bool StopEvent(const uint32_t& aPlayingID)
		{
			return Amp::WwiseAudioManager::StopEvent(aPlayingID);
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
			return Amp::WwiseAudioManager::SetSwitch(aSwitchGroup, aState, myID);
		}

		bool SetParameter(const char* aParameterName, const float& aValue)
		{
			return Amp::WwiseAudioManager::SetParameter(aParameterName, aValue, myID);
		}

		bool SetParameter(const char* aParameterName, const float& aValue, const uint32_t& aOvertime)
		{
			return Amp::WwiseAudioManager::SetParameter(aParameterName, aValue, myID, aOvertime);
		}

		uint32_t getID() { return myID; }

		//VARIABLES
	public:
		CREATE_COMPONENT_GUID("{06A69F94-BB09-4A3A-AF17-C9DA7D552BFE}"_guid);

	private:
		uint32_t myID;

	}), AudioSourceComponent);

}
