#include "amppch.h"
#include <Amp/WwiseAudioManager/WwiseAudioManager.h>

#include <Amp/WWiseEngine/WWiseEngine.h>

namespace Amp
{
	bool WwiseAudioManager::CreateAudioObject(uint32_t aObjID, const char* aObjName)
	{
		auto AKObj = myRegisteredAKObj.find(aObjID);
		if (myRegisteredAKObj.find(aObjID) != myRegisteredAKObj.end())
		{
			//TODO: DEBUG MESSAGE
			return false;
		}

		if (Amp::WWiseEngine::Get().RegisterObject(aObjName, aObjID))
		{
			AKObject aNewObj;
			aNewObj.myObjName = aObjName;
			aNewObj.myObjectID = aObjID;
			myRegisteredAKObj.insert({ aObjID,aNewObj });

			return true;
		}
		return false;
	}

	bool WwiseAudioManager::DeleteAudioObject(uint32_t aObjID)
	{
		auto AKObj = myRegisteredAKObj.find(aObjID);
		if (myRegisteredAKObj.find(aObjID) == myRegisteredAKObj.end())
		{
			//TODO: DEBUG MESSAGE
			return false;
		}

		for (auto playingID : AKObj->second.myPlayingIDs)
		{
			Amp::WWiseEngine::Get().ExecuteEventAction(AK::SoundEngine::AkActionOnEventType_Stop, playingID);
		}

		if (!Amp::WWiseEngine::Get().UnRegisterObject(AKObj->first))
		{
			//TODO: DEBUG MESSAGE
			return false;
		}

		return true;
	}

	bool WwiseAudioManager::ClearAllObjects()
	{
		for (auto AkObj : myRegisteredAKObj)
		{
			for (auto playingID : AkObj.second.myPlayingIDs)
			{
				Amp::WWiseEngine::Get().ExecuteEventAction(AK::SoundEngine::AkActionOnEventType_Stop, playingID);
			}
		}

		myRegisteredAKObj.clear();
		return WWiseEngine::Get().RemoveAllObjects();
	}

	bool WwiseAudioManager::SetObjectPosition(uint32_t aObjID, const gem::vec3& aPosition, const gem::vec3& aForward, const gem::vec3& aUp)
	{
		AkTransform transform;
		transform.SetPosition({ aPosition.x, aPosition.y, aPosition.z });
		transform.SetOrientation({ aForward.x,aForward.y,aForward.z }, { aUp.x,aUp.y,aUp.z });

		return Amp::WWiseEngine::Get().SetObjectPosition(aObjID, transform);
	}

	bool WwiseAudioManager::PlayOneShotEvent(const char* eventName)
	{
		AkTransform transform;
		transform.SetPosition({ 0, 0, 0 });
		transform.SetOrientation({0,0,1 }, { 0,1,0 });

		return Amp::WWiseEngine::Get().PlayOneShotEvent(eventName, transform);
	}

	bool WwiseAudioManager::PlayOneShotEvent(const char* eventName, const gem::vec3& aPosition, const gem::vec3& aForward, const gem::vec3& aUp)
	{
		AkTransform transform;
		transform.SetPosition({ aPosition.x, aPosition.y, aPosition.z });
		transform.SetOrientation({ aForward.x,aForward.y,aForward.z }, { aUp.x,aUp.y,aUp.z });

		return Amp::WWiseEngine::Get().PlayOneShotEvent(eventName, transform);
	}

	bool WwiseAudioManager::PlayEvent(const char* eventName, uint32_t aObjID, uint32_t& aPlayingID)
	{
		//TODO: CHECK IF IT IS REGISTERED
		if (Amp::WWiseEngine::Get().PlayEvent(eventName, aObjID, aPlayingID) != true)
		{
			return false;
		}

		if (!myRegisteredAKObj.contains(aObjID))
		{
			return false;
		}

		myRegisteredAKObj.at(aObjID).myPlayingIDs.push_back(aPlayingID);
		return true;
	}

	bool WwiseAudioManager::StopEvent(uint32_t aPlayingID)
	{
		return Amp::WWiseEngine::Get().ExecuteEventAction(AK::SoundEngine::AkActionOnEventType_Stop, aPlayingID);
	}

	bool WwiseAudioManager::PauseEvent(uint32_t aPlayingID)
	{
		return Amp::WWiseEngine::Get().ExecuteEventAction(AK::SoundEngine::AkActionOnEventType_Pause, aPlayingID);
	}

	bool WwiseAudioManager::ResumeEvent(uint32_t aPlayingID)
	{
		return Amp::WWiseEngine::Get().ExecuteEventAction(AK::SoundEngine::AkActionOnEventType_Resume, aPlayingID);
	}

	bool WwiseAudioManager::SetState(const char* aStateGroup, const char* aState)
	{
		return Amp::WWiseEngine::Get().SetState(aStateGroup, aState);
	}

	bool WwiseAudioManager::SetSwitch(const char* aSwitchGroup, const char* aState, uint64_t AkGameObjectID)
	{
		return Amp::WWiseEngine::Get().SetSwitch(aSwitchGroup, aState, AkGameObjectID);
	}

	bool WwiseAudioManager::SetParameter(const char* aParameterName, float avalue, uint64_t AkGameObjectID)
	{
		return Amp::WWiseEngine::Get().SetParameter(aParameterName, avalue, AkGameObjectID);
	}

	bool WwiseAudioManager::SetParameter(const char* aParameterName, float avalue, uint64_t AkGameObjectID, int32_t aOvertime)
	{
		return Amp::WWiseEngine::Get().SetParameter(aParameterName, avalue, AkGameObjectID, aOvertime);
	}

	std::vector<std::string> WwiseAudioManager::GetAllEventNames(std::filesystem::path aFilePath)
	{
		std::vector<std::string> strings;

		std::vector<std::string> bankNames = Amp::WWiseEngine::Get().GetBankNames();

		for (auto bankName : bankNames)
		{
			std::filesystem::path aBankFile = aFilePath / bankName;

			aBankFile.replace_extension(".txt");

			std::fstream eventFile;
			eventFile.open(aBankFile, std::ios::in);
			if (eventFile.is_open())
			{
				std::string str;

				while (std::getline(eventFile, str))
				{
					if (str.empty())
					{
						break;
					}

					if (str.find('\\') == std::string::npos) { continue; }

					size_t index = str.find_last_of('\\');

					std::string eventName = str.substr(index + 1);

					VT_CORE_INFO(eventName);
					strings.push_back(eventName);

				}

				eventFile.close();
			}
		}

		return strings;
	}

	bool WwiseAudioManager::RegisterListener(uint32_t aEntityID, const char* aEntityName, bool isDefault)
	{
		return Amp::WWiseEngine::Get().RegisterListener(aEntityID, aEntityName, isDefault);
	}

	bool WwiseAudioManager::UnregisterListener()
	{
		return Amp::WWiseEngine::Get().UnregisterListener();
	}

	bool WwiseAudioManager::SetListenerPosition(const gem::vec3& aPosition, const gem::vec3& aForward, const gem::vec3& aUp)
	{
		AkTransform transform;
		transform.SetPosition({ aPosition.x, aPosition.y, aPosition.z });
		transform.SetOrientation({ aForward.x,aForward.y,aForward.z }, { aUp.x,aUp.y,aUp.z });

		return Amp::WWiseEngine::Get().SetListenerPosition(transform);
	}

}
