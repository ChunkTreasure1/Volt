#include "amppch.h"
#include <cassert>

#include "AudioEngine.h"
#include "fmod_errors.h"

#include <Volt/Core/Profiling.h>

bool AudioEngine::Init(const std::string aFileDirectory)
{
	myRootDirectory = aFileDirectory;

	if (!myRootDirectory.empty() && myRootDirectory.back() != '\\')
	{
		myRootDirectory.append("\\");
	}

	ErrorCheck_Critical(FMOD::Studio::System::create(&myStudioSystem));

	ErrorCheck_Critical(myStudioSystem->getCoreSystem(&myCoreSystem));

	ErrorCheck_Critical(myCoreSystem->setSoftwareFormat(0, FMOD_SPEAKERMODE_STEREO, 0));

	ErrorCheck_Critical(myStudioSystem->initialize(1024, FMOD_STUDIO_INIT_NORMAL | FMOD_STUDIO_INIT_LIVEUPDATE, FMOD_INIT_NORMAL, nullptr));

	myCoreSystem->set3DNumListeners(1);
	myCoreSystem->set3DSettings(1.f, 100.f, 1.f);

	isInitialized = true;

	return true;
}

void AudioEngine::Release()
{
	myStudioSystem->unloadAll();
	myStudioSystem->release();
	myCoreSystem->release();
}

bool AudioEngine::LoadMasterBank(const std::string& aMasterFileName, const std::string& aMasterStringFileName, FMOD_STUDIO_LOAD_BANK_FLAGS)
{
	if (myStudioSystem == nullptr) { return false; }

	bool isOK = false;

	isOK = ErrorCheck(myStudioSystem->loadBankFile((myRootDirectory + aMasterStringFileName).c_str(), FMOD_STUDIO_LOAD_BANK_NORMAL, &myMasterStringBank));
	
	isOK = ErrorCheck(myStudioSystem->loadBankFile((myRootDirectory + aMasterFileName).c_str(), FMOD_STUDIO_LOAD_BANK_NORMAL, &myMasterBank));

	if (isOK)
	{
		myBanks[aMasterFileName] = myMasterBank;

		//load all the files in the bank
		FMOD::Studio::EventDescription* eventList[512]{};

		int eventCount = 0;
		myMasterBank->getEventCount(&eventCount);
		if (eventCount > 0)
		{
			FMOD_RESULT result = myMasterBank->getEventList(&*eventList, 512, &eventCount);
			for (auto ptr : eventList)
			{
				char path[512];
				int size = 0;
				result = ptr->getPath(path, 512, &size);
				std::string p(path);
				myEvents[path] = { path, false, nullptr };
			}
		}
	}

	return isOK;
}

bool AudioEngine::UnloadBank(const std::string& aFileName)
{
	auto bank = myBanks.find(aFileName);
	if (myBanks.find(aFileName) == myBanks.end())
	{
		return false;
	}

	return ErrorCheck(bank->second->unload());
}

bool AudioEngine::LoadBank(const std::string& aFileName, FMOD_STUDIO_LOAD_BANK_FLAGS someFlags)
{
	if (myStudioSystem == nullptr) { return false; }

	if (myBanks.find(aFileName) != myBanks.end())
		return true;

	FMOD::Studio::Bank* audioBank = nullptr;
	bool isOK = ErrorCheck(myStudioSystem->loadBankFile((myRootDirectory + aFileName).c_str(), someFlags, &audioBank));

	if (isOK)
	{
		myBanks[aFileName] = audioBank;

		//load all the files in the bank
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
				myEvents[path] = { path, false, nullptr };
			}
		}
	}

	return isOK;
}

void AudioEngine::Update()
{
	VT_PROFILE_FUNCTION();

	myStudioSystem->update();
}

bool AudioEngine::LoadEvent(const std::string aEventPath)
{
	if (auto It = myEvents.find(aEventPath); It != myEvents.end())
	{
		if (!It->second.isLoaded)
		{
			FMOD::Studio::EventDescription* eventDesc = nullptr;
			ErrorCheck(myStudioSystem->getEvent(aEventPath.c_str(), &eventDesc));
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

FMOD::Studio::EventInstance* AudioEngine::CreateEventInstance(const std::string aEventPath)
{
	auto foundEvent = myEvents.find(aEventPath);
	if (foundEvent == myEvents.end() || !foundEvent->second.isLoaded)
	{
		LoadEvent(aEventPath);
	}

	FMOD::Studio::EventInstance* eventInstance;
	if (!ErrorCheck(foundEvent->second.FmodEventDesc->createInstance(&eventInstance)))
	{
		std::string errorMessage = "Failed to load in audioclip with event path: " + aEventPath;
		VT_CORE_ERROR(errorMessage);
	}

	return eventInstance;
}

bool AudioEngine::PlayEvent(FMOD::Studio::EventInstance* aEventInstance)
{
	return ErrorCheck(aEventInstance->start());
}

bool AudioEngine::PlayOneShot(const std::string aEventPath)
{
	auto foundEvent = myEvents.find(aEventPath);
	if (foundEvent == myEvents.end() || !foundEvent->second.isLoaded)
	{
		LoadEvent(aEventPath);
		foundEvent = myEvents.find(aEventPath);
		if (foundEvent == myEvents.end() || !foundEvent->second.isLoaded)
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
	auto foundEvent = myEvents.find(aEventPath);
	if (foundEvent == myEvents.end() || !foundEvent->second.isLoaded)
	{
		LoadEvent(aEventPath);
		foundEvent = myEvents.find(aEventPath);
		if (foundEvent == myEvents.end() || !foundEvent->second.isLoaded)
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
	if (!myStudioSystem->getBus("bus:/", &aMasterBus))
	{
		return aMasterBus->stopAllEvents(static_cast<FMOD_STUDIO_STOP_MODE>(aStopMode));
	}

	return false;
}

bool AudioEngine::SetEvent3Dattributes(FMOD::Studio::EventInstance* aEventInstance, std::vector<float> aPosition)
{
	VT_PROFILE_FUNCTION();

	FMOD_3D_ATTRIBUTES new3Dattributes = { { 0 } };

	new3Dattributes.position.x = aPosition[0];
	new3Dattributes.position.y = aPosition[1];
	new3Dattributes.position.z = aPosition[2];

	new3Dattributes.up.x = 0;
	new3Dattributes.up.y = 1.0f;
	new3Dattributes.up.z = 0;

	new3Dattributes.forward.x = 0;
	new3Dattributes.forward.y = 0;
	new3Dattributes.forward.z = 1.0f;

	return ErrorCheck(aEventInstance->set3DAttributes(&new3Dattributes));
}

bool AudioEngine::SetEvent3Dattributes(FMOD::Studio::EventInstance* aEventInstance, gem::vec3 aPosition)
{
	std::vector<float> position;

	position.emplace_back(aPosition.x);
	position.emplace_back(aPosition.y);
	position.emplace_back(aPosition.z);

	return SetEvent3Dattributes(aEventInstance, position);
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

bool AudioEngine::InitListener(int, std::array<float, 3> aPosition, std::array<float, 3> aForwardDir, std::array<float, 3> aUpVector)
{
	testListener.position.x = aPosition[0] * 0.1f;
	testListener.position.y = aPosition[1] * 0.1f;
	testListener.position.z = aPosition[2] * 0.1f;

	testListener.up.x = aUpVector[0];
	testListener.up.y = aUpVector[1];
	testListener.up.z = aUpVector[2];

	testListener.forward.x = aForwardDir[0];
	testListener.forward.y = aForwardDir[1];
	testListener.forward.z = aForwardDir[2];

	testListener.attributes.position = testListener.position;
	testListener.attributes.up = testListener.up;
	testListener.attributes.forward = testListener.forward;

	return ErrorCheck(myStudioSystem->setListenerAttributes(0, &testListener.attributes));
}

bool AudioEngine::InitListener(int aListenerID, gem::vec3 aPosition, gem::vec3 aForwardDir, gem::vec3 aUpVector)
{
	std::array<float, 3> position;
	std::array<float, 3> forward;
	std::array<float, 3> up;

	position[0] = aPosition.x;
	position[1] = aPosition.y;
	position[2] = aPosition.z;

	forward[0] = aForwardDir.x;
	forward[1] = aForwardDir.y;
	forward[2] = aForwardDir.z;

	up[0] = aUpVector.x;
	up[1] = aUpVector.y;
	up[2] = aUpVector.z;

	return (InitListener(aListenerID, position, forward, up));
}

bool AudioEngine::UpdateListener(int, std::array<float, 3> aPosition, std::array<float, 3> aForwardDir, std::array<float, 3> aUpDir, std::array<float, 3> aVelocity)
{
	VT_PROFILE_FUNCTION();

	testListener.position.x = aPosition[0];
	testListener.position.y = aPosition[1];
	testListener.position.z = aPosition[2];

	testListener.forward.x = aForwardDir[0];
	testListener.forward.y = aForwardDir[1];
	testListener.forward.z = aForwardDir[2];

	testListener.up.x = aUpDir[0];
	testListener.up.y = aUpDir[1];
	testListener.up.z = aUpDir[2];

	testListener.velocity.x = aVelocity[0];
	testListener.velocity.y = aVelocity[1];
	testListener.velocity.z = aVelocity[2];

	testListener.attributes.position = testListener.position;
	testListener.attributes.forward = testListener.forward;
	testListener.attributes.up = testListener.up;
	testListener.attributes.velocity = testListener.velocity;

	ErrorCheck(myCoreSystem->set3DListenerAttributes(0, &testListener.position, &testListener.velocity, &testListener.forward, &testListener.up));
	return ErrorCheck(myStudioSystem->setListenerAttributes(0, &testListener.attributes));
}

bool AudioEngine::UpdateListener(int aListenerID, gem::vec3 aPosition, gem::vec3 aForwardDir, gem::vec3 aUp, gem::vec3 aVelocity)
{
	std::array<float, 3> position;
	std::array<float, 3> forward;
	std::array<float, 3> up;
	std::array<float, 3> velocity;

	position[0] = aPosition.x;
	position[1] = aPosition.y;
	position[2] = aPosition.z;

	forward[0] = aForwardDir.x;
	forward[1] = aForwardDir.y;
	forward[2] = aForwardDir.z;

	up[0] = aUp.x;
	up[1] = aUp.y;
	up[2] = aUp.z;

	velocity[0] = aVelocity.x;
	velocity[1] = aVelocity.y;
	velocity[2] = aVelocity.z;

	return (UpdateListener(aListenerID, position, forward, up, velocity));
}

bool AudioEngine::SetMixerVolume(const std::string& aBusName, float aVolume)
{
	FMOD::Studio::Bus* aBus = nullptr;
	if (!myStudioSystem->getBus(aBusName.c_str(), &aBus))
	{
		return aBus->setVolume(aVolume);
	}

	return false;
}




