#pragma once
#include "AudioEngine.h"
#include "AudioDefinitions.h"
#include "Volt/Scene/Entity.h"

#define AUDIOMANAGER AudioManager::GetInstance()

struct EventData
{
	int myId;
	std::string myName;
	FMOD::Studio::EventInstance* myEventInstance;
	Volt::Entity myParent = {};
	float myWaitingTime = 0;
	bool myWaiting = false;
};

struct ListenerData
{
	int myListenerId;
	Volt::Entity myParent;
	Volt::Entity myParentsParent;
};

class AudioManager
{
	public:
	static AudioManager& GetInstance();
	AudioManager(AudioManager& aManager) = delete;
	AudioManager& operator=(const AudioManager& aManager) = delete;

	void Init(std::string aFileDirectory, std::string aMasterBank, std::string aMasterStringsBank, FMOD_STUDIO_LOAD_BANK_FLAGS aLoadBankFlags = 0);
	void InitListener(int aListenerID, Volt::Entity aParent);
	void ResetListener();

	void Update(float aDeltaTime);
	void UpdateAudioEngine();
	int Play2D(EventName anEventName, bool aShouldLoop);
	int Play2DAfter(EventName anEventName, bool aShouldLoop, float aTime);
	int Play3D(EventName anEventName, bool aShouldLoop, bool aShouldFollow, Volt::Entity aParent);
	int Play3DAfter(EventName anEventName, bool aShouldLoop, bool aShouldFollow, Volt::Entity aParent, float aTime);

	bool PlayOnce(EventName anEventName);

	void StopAll(int aStopMode = 0);
	void StopById(int anId, bool aImmediately = true);
	void StopByEventName(std::string anEventName, bool aImmediately = true);

	bool SetMasterVolume(float aVolPerct);
	bool SetMixerVolume(std::string aMixerPath, float aVolPerct);

	private:
	AudioManager() : myAudioEngine(AudioEngine::GetInstance()) {};
	~AudioManager();
	static AudioManager* myInstance;
	AudioEngine& myAudioEngine;
	std::vector<EventData> myEventInstances;

	inline static int ourIdCounter = 0;

	bool myHasListener = false;
	std::shared_ptr<ListenerData> myListenerData;
};
