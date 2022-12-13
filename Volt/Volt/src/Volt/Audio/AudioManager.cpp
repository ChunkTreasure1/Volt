#include "vtpch.h"
#include "AudioManager.h"
#include "AudioDefinitions.h"
#include "Volt/Components/Components.h"

AudioManager* AudioManager::myInstance = nullptr;

AudioManager::~AudioManager()
{
	delete myInstance;
}

AudioManager& AudioManager::GetInstance()
{
	if (myInstance == nullptr)
	{
		myInstance = new AudioManager();
	}

	return *myInstance;
}

void AudioManager::Init(std::string aFileDirectory, std::string aMasterBank, std::string aMasterStringsBank, FMOD_STUDIO_LOAD_BANK_FLAGS aLoadBankFlags)
{
	myAudioEngine.Init(aFileDirectory);
	myAudioEngine.LoadMasterBank(aMasterBank, aMasterStringsBank, aLoadBankFlags);
}

void AudioManager::Update(float aDeltaTime)
{
	if (myHasListener)
	{
		// IMPORTANT! Optimized specifically for being parented to >>player camera<<.
		myAudioEngine.UpdateListener(myListenerData->myListenerId, myListenerData->myParentsParent.GetLocalPosition(), myListenerData->myParent.GetLocalForward(), myListenerData->myParent.GetLocalUp());
	}

	for (EventData& audioEvent : myEventInstances)
	{
		if (!audioEvent.myParent.IsNull() && myListenerData)
		{
			myAudioEngine.SetEvent3Dattributes(audioEvent.myEventInstance, audioEvent.myParent.GetPosition());
		}

		if (audioEvent.myWaiting)
		{
			if (audioEvent.myWaitingTime > 0)
			{
				audioEvent.myWaitingTime -= aDeltaTime;
			}
			else
			{
				audioEvent.myWaiting = false;
				myAudioEngine.PlayEvent(audioEvent.myEventInstance);
			}
		}
	}
}

void AudioManager::UpdateAudioEngine()
{
	myAudioEngine.Update();
}

void AudioManager::InitListener(int aListenerID, Volt::Entity aParent)
{
	myListenerData = std::make_shared<ListenerData>();
	myListenerData->myListenerId = aListenerID;
	myListenerData->myParent = aParent;

	auto& relComp = myListenerData->myParent.GetComponent<Volt::RelationshipComponent>();
	myListenerData->myParentsParent = { relComp.Parent, myListenerData->myParent.GetScene() };

	myHasListener = true;

	AUDIOENGINE.InitListener(myListenerData->myListenerId, myListenerData->myParentsParent.GetLocalPosition() + myListenerData->myParent.GetLocalPosition(), myListenerData->myParent.GetLocalForward(), myListenerData->myParent.GetLocalUp());
}

void AudioManager::ResetListener()
{
	myHasListener = false;
}

int AudioManager::Play2D(EventName anEventName, bool aShouldLoop)
{
	if (aShouldLoop)
	{
		anEventName += "_LOOP";
	}

	FMOD::Studio::EventInstance* eventInstance = myAudioEngine.CreateEventInstance(anEventName);

	EventData eventData;
	eventData.myId = ourIdCounter;
	eventData.myName = anEventName;
	ourIdCounter++;
	eventData.myEventInstance = eventInstance;


	myEventInstances.emplace_back(eventData);

	myAudioEngine.PlayEvent(eventInstance);

	return eventData.myId;
}

int AudioManager::Play2DAfter(EventName anEventName, bool aShouldLoop, float aTime)
{
	if (aShouldLoop)
	{
		anEventName += "_LOOP";
	}

	FMOD::Studio::EventInstance* eventInstance = myAudioEngine.CreateEventInstance(anEventName);

	EventData eventData;
	eventData.myId = ourIdCounter;
	eventData.myName = anEventName;
	ourIdCounter++;
	eventData.myEventInstance = eventInstance;
	eventData.myWaitingTime = aTime;
	eventData.myWaiting = true;

	myEventInstances.emplace_back(eventData);

	return eventData.myId;
}

int AudioManager::Play3D(EventName anEventName, bool aShouldLoop, bool aShouldFollow, Volt::Entity aParent)
{
	if (myListenerData == nullptr) { return 0; }
	if (aShouldLoop)
	{
		anEventName += "_LOOP";
	}

	FMOD::Studio::EventInstance* eventInstance = myAudioEngine.CreateEventInstance(anEventName);

	myAudioEngine.SetEvent3Dattributes(eventInstance, aParent.GetPosition());
	EventData eventData;
	eventData.myId = ourIdCounter;
	eventData.myName = anEventName;
	ourIdCounter++;
	eventData.myEventInstance = eventInstance;
	
	if (aShouldFollow)
	{
		eventData.myParent = aParent;
	}

	myEventInstances.emplace_back(eventData);

	myAudioEngine.PlayEvent(eventInstance);

	return eventData.myId;
}

int AudioManager::Play3DAfter(EventName anEventName, bool aShouldLoop, bool aShouldFollow, Volt::Entity aParent, float aTime)
{
	if (myListenerData == nullptr) { return 0; }
	if (aShouldLoop)
	{
		anEventName += "_LOOP";
	}

	FMOD::Studio::EventInstance* eventInstance = myAudioEngine.CreateEventInstance(anEventName);
	myAudioEngine.SetEvent3Dattributes(eventInstance, aParent.GetPosition());

	EventData eventData;
	eventData.myId = ourIdCounter;
	eventData.myName = anEventName;
	ourIdCounter++;
	eventData.myEventInstance = eventInstance;
	eventData.myWaitingTime = aTime;
	eventData.myWaiting = true;

	if (aShouldFollow)
	{
		eventData.myParent = aParent;
	}

	myEventInstances.emplace_back(eventData);

	return eventData.myId;
}

bool AudioManager::PlayOnce(EventName anEventName)
{
	return myAudioEngine.PlayOneShot(anEventName);
}

void AudioManager::StopAll(int)
{
	myEventInstances.clear();
	myAudioEngine.StopAll(0);
}

void AudioManager::StopById(int anId, bool aImmediately)
{
	for (int i = 0; i < myEventInstances.size(); i++)
	{
		if (myEventInstances[i].myId == anId)
		{
			myAudioEngine.StopEvent(myEventInstances[i].myEventInstance, aImmediately);
			myEventInstances.erase(myEventInstances.begin() + i);
			return;
		}
	}
}

void AudioManager::StopByEventName(std::string anEventName, bool aImmediately)
{
	std::string loopingNameVersion = anEventName + "_LOOP";

	for (int i = myEventInstances.size() - 1; i >= 0; i--)
	{
		if (myEventInstances[i].myName == anEventName)
		{
			myAudioEngine.StopEvent(myEventInstances[i].myEventInstance, aImmediately);
			myEventInstances.erase(myEventInstances.begin() + i);
			continue;
		}

		if (myEventInstances[i].myName == loopingNameVersion)
		{
			myAudioEngine.StopEvent(myEventInstances[i].myEventInstance, aImmediately);
			myEventInstances.erase(myEventInstances.begin() + i);
			continue;
		}
	}
}

bool AudioManager::SetMasterVolume(float aVolPerct)
{
	return myAudioEngine.SetMixerVolume("bus:/", aVolPerct);
}

bool AudioManager::SetMixerVolume(std::string aMixerPath, float aVolPerct)
{
	return myAudioEngine.SetMixerVolume(aMixerPath, aVolPerct);
}


