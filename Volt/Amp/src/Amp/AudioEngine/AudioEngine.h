#pragma once
#include <string>
#include <array>
#include <unordered_map>

#include "fmod_studio.hpp"
#include "fmod.hpp"
#include "fmod_errors.h"

#include "GEM/gem.h"

struct Event
{
	std::string Path;
	bool isLoaded;
	FMOD::Studio::EventDescription* FmodEventDesc = nullptr;

	typedef std::unordered_map<int, FMOD::Studio::EventInstance*> InstanceMap;
	InstanceMap instances;

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
	FMOD::Studio::System* studioSystem = NULL;
	FMOD::System* coreSystem = NULL;

	FMOD::Studio::Bank* masterBank = NULL;
	FMOD::Studio::Bank* masterStringBank = NULL;

	typedef std::unordered_map<std::string, Event> EventMap;
	typedef std::unordered_map<std::string, FMOD::Studio::Bank*> BankMap;

	EventMap events;
	BankMap banks;

	Listener mainListener;

public:
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
	std::string rootDirectory;
	bool isInitialized;
	FMOD_RESULT lastResult = FMOD_OK;

	bool ErrorCheck_Critical(FMOD_RESULT result);
	bool ErrorCheck(FMOD_RESULT result);

};
