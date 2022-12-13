#include "amppch.h"
#include <cassert>

#include "AudioEngine.h"
#include "fmod_errors.h"

#include <Volt/Core/Profiling.h>

const float DISTANCEFACTOR = 100.0f;

namespace Amp 
{
	bool AudioEngine::Init(const std::string aFileDirectory)
	{
		rootDirectory = aFileDirectory;

		if (!rootDirectory.empty() && rootDirectory.back() != '\\')
		{
			rootDirectory.append("\\");
		}

		ErrorCheck_Critical(FMOD::Studio::System::create(&studioSystem));

		ErrorCheck_Critical(studioSystem->getCoreSystem(&coreSystem));

		ErrorCheck_Critical(coreSystem->setSoftwareFormat(0, FMOD_SPEAKERMODE_STEREO, 0));

		ErrorCheck_Critical(studioSystem->initialize(1024, FMOD_STUDIO_INIT_NORMAL | FMOD_STUDIO_INIT_LIVEUPDATE, FMOD_INIT_NORMAL, nullptr));

		coreSystem->set3DNumListeners(1);
		coreSystem->set3DSettings(1.f, DISTANCEFACTOR, 1.f);

		isInitialized = true;

		return true;
	}

	void AudioEngine::Release()
	{
		studioSystem->unloadAll();
		studioSystem->release();
		coreSystem->release();
	}

	bool AudioEngine::LoadMasterBank(const std::string& aMasterFileName, const std::string& aMasterStringFileName, FMOD_STUDIO_LOAD_BANK_FLAGS)
	{
		if (studioSystem == nullptr) { return false; }

		bool isOK = false;

		isOK = ErrorCheck(studioSystem->loadBankFile((rootDirectory + aMasterStringFileName).c_str(), FMOD_STUDIO_LOAD_BANK_NORMAL, &masterStringBank));

		isOK = ErrorCheck(studioSystem->loadBankFile((rootDirectory + aMasterFileName).c_str(), FMOD_STUDIO_LOAD_BANK_NORMAL, &masterBank));

		if (isOK)
		{
			banks[aMasterFileName] = masterBank;

			//load all the files in the bank
			FMOD::Studio::EventDescription* eventList[512]{};

			int eventCount = 0;
			masterBank->getEventCount(&eventCount);
			if (eventCount > 0)
			{
				FMOD_RESULT result = masterBank->getEventList(&*eventList, 512, &eventCount);
				for (auto ptr : eventList)
				{
					char path[512];
					int size = 0;
					result = ptr->getPath(path, 512, &size);
					events[path] = { path, false, nullptr };
				}
			}
		}

		return isOK;
	}

	bool AudioEngine::UnloadBank(const std::string& aFileName)
	{
		auto bank = banks.find(aFileName);
		if (banks.find(aFileName) == banks.end())
		{
			return false;
		}

		return ErrorCheck(bank->second->unload());
	}

	bool AudioEngine::LoadBank(const std::string& aFileName, FMOD_STUDIO_LOAD_BANK_FLAGS someFlags)
	{
		if (studioSystem == nullptr) { return false; }

		if (banks.find(aFileName) != banks.end())
			return true;

		FMOD::Studio::Bank* audioBank = nullptr;
		bool isOK = ErrorCheck(studioSystem->loadBankFile((rootDirectory + aFileName).c_str(), someFlags, &audioBank));

		if (isOK)
		{
			banks[aFileName] = audioBank;

			//load all the files in the bank
			//TODO: Fill the EventData with information
			FMOD::Studio::EventDescription* eventList[512]{};

			int eventCount = 0;
			audioBank->getEventCount(&eventCount);
			if (eventCount > 0)
			{
				FMOD_RESULT result = audioBank->getEventList(&*eventList, 512, &eventCount);
				for (auto ptr : eventList)
				{
					char path[512];
					int size = 0;
					result = ptr->getPath(path, 512, &size);
					std::string p(path);
					events[path] = { path, false, nullptr };
				}
			}
		}

		return isOK;
	}

	void AudioEngine::Update()
	{
		VT_PROFILE_FUNCTION();

		coreSystem->update();
		studioSystem->update();
	}

	bool AudioEngine::LoadEvent(const std::string aEventPath)
	{
		if (auto It = events.find(aEventPath); It != events.end())
		{
			if (!It->second.isLoaded)
			{
				FMOD::Studio::EventDescription* eventDesc = nullptr;
				ErrorCheck(studioSystem->getEvent(aEventPath.c_str(), &eventDesc));
				if (eventDesc)
				{
					It->second.FmodEventDesc = eventDesc;
					It->second.isLoaded = true;
					return true;
				}
			}
		}
		return false;
	}

	EventInstance& AudioEngine::CreateEventInstance(const std::string aEventPath)
	{
		auto foundEvent = events.find(aEventPath);
		if (foundEvent == events.end() || !foundEvent->second.isLoaded)
		{
			LoadEvent(aEventPath);
		}

		FMOD::Studio::EventInstance* eventInstance;
		if (!ErrorCheck(foundEvent->second.FmodEventDesc->createInstance(&eventInstance)))
		{
			std::string errorMessage = "Failed to load in audioclip with event path: " + aEventPath;
			VT_CORE_ERROR(errorMessage);
		}

		foundEvent->second.instanceIDpool++;
		int aNewID = foundEvent->second.instanceIDpool;
		foundEvent->second.instances.insert({ aNewID, std::make_unique<EventInstance>()});
		EventInstance& eventInstanceHandle = *foundEvent->second.instances.end()->second;

		eventInstanceHandle.ID = aNewID;
		eventInstanceHandle.instance = eventInstance;

		return eventInstanceHandle;
	}

	bool AudioEngine::RemoveEvent(EventInstance& aEventHandle)
	{
		//TODO Remove Event
		return false;
	}

	bool AudioEngine::PlayEvent(FMOD::Studio::EventInstance* aEventInstance)
	{
		return ErrorCheck(aEventInstance->start());
	}

	bool AudioEngine::PlayOneShot(const std::string aEventPath)
	{
		auto foundEvent = events.find(aEventPath);
		if (foundEvent == events.end() || !foundEvent->second.isLoaded)
		{
			LoadEvent(aEventPath);
			foundEvent = events.find(aEventPath);
			if (foundEvent == events.end() || !foundEvent->second.isLoaded)
				return false;
		}

		FMOD::Studio::EventInstance* eventInstance;
		lastResult = foundEvent->second.FmodEventDesc->createInstance(&eventInstance);
		if (eventInstance)
		{
			lastResult = eventInstance->start();
			lastResult = eventInstance->release();
		}

		return lastResult == FMOD_OK;
	}

	bool AudioEngine::PlayOneShot(const std::string aEventPath, const std::string aEventParameter, const float aParameterValue)
	{
		auto foundEvent = events.find(aEventPath);
		if (foundEvent == events.end() || !foundEvent->second.isLoaded)
		{
			LoadEvent(aEventPath);
			foundEvent = events.find(aEventPath);
			if (foundEvent == events.end() || !foundEvent->second.isLoaded)
				return false;
		}

		FMOD::Studio::EventInstance* eventInstance;
		lastResult = foundEvent->second.FmodEventDesc->createInstance(&eventInstance);
		if (eventInstance)
		{
			lastResult = eventInstance->start();
			lastResult = eventInstance->setParameterByName(aEventParameter.c_str(), aParameterValue);
			lastResult = eventInstance->release();
		}

		return lastResult == FMOD_OK;
	}

	bool AudioEngine::StopEvent(FMOD::Studio::EventInstance* aEventInstance, bool immediately)
	{
		const FMOD_STUDIO_STOP_MODE stopMode = immediately ? FMOD_STUDIO_STOP_IMMEDIATE : FMOD_STUDIO_STOP_ALLOWFADEOUT;
		return ErrorCheck(aEventInstance->stop(stopMode));
	}

	bool AudioEngine::StopAll(const int aStopMode)
	{
		FMOD::Studio::Bus* aMasterBus = nullptr;
		if (!studioSystem->getBus("bus:/", &aMasterBus))
		{
			return aMasterBus->stopAllEvents(static_cast<FMOD_STUDIO_STOP_MODE>(aStopMode));
		}

		return false;
	}

	bool AudioEngine::SetEvent3Dattributes(FMOD::Studio::EventInstance* aEventInstance, EventData aEventData)
	{
		VT_PROFILE_FUNCTION();

		FMOD_3D_ATTRIBUTES new3Dattributes = { { 0 } };

		new3Dattributes.position.x = aEventData.position.x;
		new3Dattributes.position.y = aEventData.position.y;
		new3Dattributes.position.z = aEventData.position.z;

		new3Dattributes.up.x = aEventData.up.x;
		new3Dattributes.up.y = aEventData.up.y;
		new3Dattributes.up.z = aEventData.up.z;

		new3Dattributes.forward.x = 0;
		new3Dattributes.forward.y = 0;
		new3Dattributes.forward.z = 1.0f;

		return ErrorCheck(aEventInstance->set3DAttributes(&new3Dattributes));
	}

	bool AudioEngine::ErrorCheck(FMOD_RESULT result)
	{
		lastResult = result;
		if (result != FMOD_OK)
		{
			return false;
		}
		return true;
	}

	bool AudioEngine::ErrorCheck_Critical(FMOD_RESULT result)
	{
		lastResult = result;
		_ASSERTE(result == FMOD_OK && "CRITICAL FMOD ERROR");
		return true;
	}

	bool AudioEngine::InitListener(ListenerData aListenerData)
	{
		mainListener.position.x = aListenerData.position[0];
		mainListener.position.y = aListenerData.position[1];
		mainListener.position.z = aListenerData.position[2];

		mainListener.forward.x = aListenerData.forward[0];
		mainListener.forward.y = aListenerData.forward[1];
		mainListener.forward.z = aListenerData.forward[2];

		mainListener.up.x = aListenerData.up[0];
		mainListener.up.y = aListenerData.up[1];
		mainListener.up.z = aListenerData.up[2];

		mainListener.listenerID = aListenerData.ID;
		mainListener.prevPosition = mainListener.position;
		mainListener.attributes.position = mainListener.position;
		mainListener.attributes.up = mainListener.up;
		mainListener.attributes.forward = mainListener.forward;

		return ErrorCheck(studioSystem->setListenerAttributes(mainListener.listenerID, &mainListener.attributes));
	}

	bool AudioEngine::UpdateListener(ListenerData aListenerData)
	{
		VT_PROFILE_FUNCTION();

		mainListener.position.x = aListenerData.position[0];
		mainListener.position.y = aListenerData.position[1];
		mainListener.position.z = aListenerData.position[2];

		mainListener.forward.x = aListenerData.forward[0];
		mainListener.forward.y = aListenerData.forward[1];
		mainListener.forward.z = aListenerData.forward[2];

		mainListener.up.x = aListenerData.up[0];
		mainListener.up.y = aListenerData.up[1];
		mainListener.up.z = aListenerData.up[2];

		mainListener.velocity.x = (mainListener.position.x - mainListener.prevPosition.x) * aListenerData.aDeltaTime;
		mainListener.velocity.y = (mainListener.position.y - mainListener.prevPosition.y) * aListenerData.aDeltaTime;
		mainListener.velocity.z = (mainListener.position.z - mainListener.prevPosition.z) * aListenerData.aDeltaTime;

		mainListener.prevPosition = mainListener.position;

		mainListener.attributes.position = mainListener.position;
		mainListener.attributes.forward = mainListener.forward;
		mainListener.attributes.up = mainListener.up;
		mainListener.attributes.velocity = mainListener.velocity;

		if (!ErrorCheck(coreSystem->set3DListenerAttributes(0, &mainListener.position, &mainListener.velocity, &mainListener.forward, &mainListener.up)))
		{
			return false;
		}

		if (!ErrorCheck(studioSystem->setListenerAttributes(0, &mainListener.attributes)))
		{
			return false;
		}

		return true;
	}

	bool AudioEngine::SetMixerVolume(const std::string& aBusName, float aVolume)
	{
		FMOD::Studio::Bus* aBus = nullptr;
		if (!studioSystem->getBus(aBusName.c_str(), &aBus))
		{
			return aBus->setVolume(aVolume);
		}

		return false;
	}
}






