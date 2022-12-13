#pragma once
#include <string>
#include <array>
#include <unordered_map>

#include "fmod_studio.hpp"
#include "fmod.hpp"
#include "fmod_errors.h"

#include "GEM/gem.h"

namespace Amp
{
	struct InitInsturct
	{
		std::string aFileDirectory;
		std::string aMasterbank;
		std::string aMasterStringsBank;
		FMOD_STUDIO_LOAD_BANK_FLAGS aLoadBankFlags;
	};

	struct EventInstance
	{
		int ID = -1;
		FMOD::Studio::EventInstance* instance = nullptr;
		FMOD_VECTOR position{0,0,0};
		FMOD_VECTOR prevPosition{ 0,0,0 };
		FMOD_VECTOR velocity{ 0,0,0 };
		FMOD_VECTOR forward{ 0,0,0 };
		FMOD_VECTOR up{ 0,0,0 };
	};

	struct Event
	{
		std::string Path = "";
		bool isLoaded = false;
		FMOD::Studio::EventDescription* FmodEventDesc = nullptr;

		int instanceIDpool = 0;
		typedef std::unordered_map<int, std::unique_ptr<EventInstance>> InstanceMap;
		InstanceMap instances;
	};

	struct EventData
	{
		FMOD_VECTOR position;
		FMOD_VECTOR forward;
		FMOD_VECTOR up;
	};

	struct Listener
	{
		int listenerID = -1;
		FMOD_VECTOR position{0,0,0};
		FMOD_VECTOR prevPosition{ 0,0,0 };
		FMOD_VECTOR velocity{ 0,0,0 };
		FMOD_VECTOR forward{ 0,0,0 };
		FMOD_VECTOR up{ 0,0,0 };

		FMOD_3D_ATTRIBUTES attributes{};
	};

	struct ListenerData
	{
		int ID = -1;
		float aDeltaTime = 0;
		std::array<float, 3> position;
		std::array<float, 3> forward;
		std::array<float, 3> up;
	};

	class AudioEngine
	{
		FMOD::Studio::System* studioSystem = NULL;
		FMOD::System* coreSystem = NULL;

		FMOD::Studio::Bank* masterBank = NULL;
		FMOD::Studio::Bank* masterStringBank = NULL;

		typedef std::unordered_map<std::string, Event> EventMap;
		typedef std::unordered_map<std::string, FMOD::Studio::Bank*> BankMap;
		typedef std::unordered_map<int, Listener> ListenerMap;

		EventMap events;
		BankMap banks;
		ListenerMap listeners;

		//TODO: Make to map
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

		EventInstance& CreateEventInstance(const std::string aEventPath);
		bool RemoveEvent(EventInstance& aEventHandle);

		bool PlayEvent(FMOD::Studio::EventInstance* aEventInstance);
		bool PlayOneShot(const std::string aEventPath);
		//TODO 3D OneShot
		bool PlayOneShot(const std::string aEventPath, const std::string aEventParameter, const float aParameterValue);
		//TODO 3D OneShot with parameters

		bool StopEvent(FMOD::Studio::EventInstance* aEventInstance, bool immediately);
		bool StopAll(const int aStopMode);

		bool SetEventParameter(FMOD::Studio::EventInstance* aEventInstance, const std::string aEventParameter, const float aParameterValue);

		bool SetEvent3Dattributes(FMOD::Studio::EventInstance* aEventInstance, EventData aEventData);

		bool InitListener(ListenerData aListenerData);
		bool UpdateListener(ListenerData aListenerData);

		bool SetMixerVolume(const std::string& aBusName, float aVolume);

	private:
		std::string rootDirectory;
		bool isInitialized;
		FMOD_RESULT lastResult = FMOD_OK;

		bool ErrorCheck_Critical(FMOD_RESULT result);
		bool ErrorCheck(FMOD_RESULT result);

	};
}


