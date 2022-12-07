#pragma once
#include <string>
#include <array>
#include <unordered_map>

#include "fmod_studio.hpp"
#include "fmod.hpp"
#include "fmod_errors.h"
#include "GEM/gem.h"
#include "Volt/Log/Log.h"

#define AUDIOENGINE AudioEngine::GetInstance()

struct Event
{
	std::string Path;
	bool isLoaded;
	FMOD::Studio::EventDescription* FmodEventDesc = nullptr;
};

struct Listener
{
	int listenerID;
	FMOD_VECTOR position;
	FMOD_VECTOR velocity;
	FMOD_VECTOR forward;
	FMOD_VECTOR up;

	FMOD_3D_ATTRIBUTES attributes;
};

class AudioEngine
{
	FMOD::Studio::System* myStudioSystem = NULL;
	FMOD::System* myCoreSystem = NULL;

	FMOD::Studio::Bank* myMasterBank = NULL;
	FMOD::Studio::Bank* myMasterStringBank = NULL;

	typedef std::unordered_map<std::string, Event> EventMap;
	typedef std::unordered_map<std::string, FMOD::Studio::Bank*> BankMap;

	EventMap myEvents;
	BankMap myBanks;

	Listener testListener;

public:
	static AudioEngine& GetInstance();

	AudioEngine() = default;
	~AudioEngine() = default;

	bool Init(const std::string aFileDirectory);
	void Update();
	void Release();

	bool LoadMasterBank(const std::string& aMasterFileName, const std::string& aMasterStringFileName, FMOD_STUDIO_LOAD_BANK_FLAGS someFlags);
	bool LoadBank(const std::string& aFileName, FMOD_STUDIO_LOAD_BANK_FLAGS someFlags);
	bool UnloadBank(const std::string& aFileName);

	bool LoadEvent(const std::string aEventPath);
	FMOD::Studio::EventInstance* CreateEventInstance(const std::string aEventPath);

	bool PlayEvent(FMOD::Studio::EventInstance* aEventInstance);
	bool PlayOneShot(const std::string aEventPath);
	bool PlayOneShot(const std::string aEventPath, const std::string aEventParameter, const float aParameterValue);

	bool StopEvent(FMOD::Studio::EventInstance* aEventInstance, bool immediately);
	bool StopAll(const int aStopMode);

	bool SetEventParameter(FMOD::Studio::EventInstance* aEventInstance, const std::string aEventParameter, const float aParameterValue);
	bool SetEvent3Dattributes(FMOD::Studio::EventInstance* aEventInstance, std::vector<float> aPosition);
	bool SetEvent3Dattributes(FMOD::Studio::EventInstance* aEventInstance, gem::vec3 aPosition);

	bool InitListener(int aListenerID, std::array<float,3> aPosition, std::array<float,3> aForwardDir, std::array<float,3> aUpVector);
	bool InitListener(int aListenerID, gem::vec3 aPosition, gem::vec3 aForwardDir, gem::vec3 aUpVector);
	bool UpdateListener(int aListenerID, std::array<float, 3> aPosition, std::array<float, 3> aForwardDir, std::array<float, 3> aUp, std::array<float, 3> aVelocity);
	bool UpdateListener(int aListenerID, gem::vec3 aPosition, gem::vec3 aForwardDir, gem::vec3 aUp, gem::vec3 aVelocity = {0,0,0});

	bool SetMixerVolume(const std::string& aBusName, float aVolume);

private:
	static AudioEngine* instance;

	std::string myRootDirectory;
	bool isInitialized;
	FMOD_RESULT lastResult = FMOD_OK;

	bool ErrorCheck_Critical(FMOD_RESULT result);
	bool ErrorCheck(FMOD_RESULT result);

};

//// Audiotest
//if (!initiated)
//{
//	instance = AUDIOENGINE.CreateEventInstance("event:/Test");
//	AUDIOENGINE.SetEvent3Dattributes(instance, { 0,0,0 });
//}
//
////myRuntimeScene->GetRegistry().ForEach<Volt::CameraComponent>([&](Wire::EntityId anId, Volt::CameraComponent& aCamera)
////	{
////		camera = Volt::Entity(anId, myRuntimeScene.get());
////	});
//
//if (!initiated)
//{
//	
// 
// 
// .InitListener(0, myEditorCameraController->GetCamera()->GetPosition(), myEditorCameraController->GetCamera()->GetForward(), myEditorCameraController->GetCamera()->GetUp());
//	AUDIOENGINE.PlayEvent(instance);
//
//	initiated = true;
//}
//
//AUDIOENGINE.UpdateListener(0, myEditorCameraController->GetCamera()->GetPosition(), myEditorCameraController->GetCamera()->GetForward(), myEditorCameraController->GetCamera()->GetUp());

