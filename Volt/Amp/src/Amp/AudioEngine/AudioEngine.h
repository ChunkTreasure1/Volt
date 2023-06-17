#pragma once
#include <string>
#include <array>
#include <unordered_map>

#include "fmod_studio.hpp"
#include "fmod.hpp"
#include "fmod_errors.h"

#include <glm/glm.hpp>

namespace Amp
{
	struct InitInsturct
	{
		std::filesystem::path aFileDirectory;
		std::filesystem::path aMasterbank;
		std::filesystem::path aMasterStringsBank;
		FMOD_STUDIO_LOAD_BANK_FLAGS aLoadBankFlags;
	};

	struct EventInstance
	{
		int ID = -1;
		std::string EventName = "";
		FMOD::Studio::EventDescription* EventDesc = nullptr;
		FMOD::Studio::EventInstance* instance = nullptr;
		FMOD_VECTOR position{ 0,0,0 };
		FMOD_VECTOR prevPosition{ 0,0,0 };
		FMOD_VECTOR velocity{ 0,0,0 };
		FMOD_VECTOR forward{ 0,0,0 };
		FMOD_VECTOR up{ 0,0,0 };
		//FMOD_STUDIO_STOP_MODE stopMode = FMOD_STUDIO_STOP_IMMEDIATE; // Might need later?
	};

	struct Event
	{
		std::string Path = "";
		bool isLoaded = false;
		FMOD::Studio::EventDescription* FmodEventDesc = nullptr;

		int instanceIDpool = 0;
		typedef std::unordered_map<int, EventInstance> InstanceMap;
		InstanceMap instances;
	};

	struct EventData
	{
		float aDeltaTime = 0;
		bool is3D = false;
		FMOD_VECTOR position{ 0,0,0 };
		FMOD_VECTOR forward{ 0,0,0 };
		FMOD_VECTOR up{ 0,0,0 };
		bool changeParameter = false;
		std::string aParameterPath = "";
		float aParameterValue;
	};

	struct Listener
	{
		int listenerID = -1;
		FMOD_VECTOR position{ 0,0,0 };
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
		glm::vec3 position;
		glm::vec3 forward;
		glm::vec3 up;
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
		void Update(float aDeltaTime);
		void Release();

		bool LoadMasterBank(const std::string& aMasterFileName, const std::string& aMasterStringFileName, FMOD_STUDIO_LOAD_BANK_FLAGS someFlags);
		bool LoadBank(const std::string& aFileName, FMOD_STUDIO_LOAD_BANK_FLAGS someFlags);
		bool UnloadBank(const std::string& aFileName);

		bool LoadEvent(const std::string& aEventPath);
		FMOD::Studio::EventDescription* FindEvent(const std::string& aEventPath);
		std::vector<std::string> GetAllEventNames();

		EventInstance& CreateEventInstance(const std::string& aEventPath);
		bool RemoveEvent(EventInstance& aEventHandle);
		void ReleaseAll();

		bool PlayEvent(FMOD::Studio::EventInstance* aEventInstance);

		bool PlayOneShot(const std::string& aEventPath, EventData& a3DData);

		bool StopEvent(FMOD::Studio::EventInstance* aEventInstance, bool immediately);
		bool StopAll(const int aStopMode);

		bool PauseEvent(FMOD::Studio::EventInstance* aEventInstance);
		bool UnpauseEvent(FMOD::Studio::EventInstance* aEventInstance);

		bool SetEventParameter(FMOD::Studio::EventInstance* aEventInstance, const std::string& aEventParameter, const float aParameterValue);

		void Update3DEvents(float aDeltaTime);
		void SetEvent3Dattributes(EventInstance& aEventInstance, float aDeltaTime);
		bool SetEvent3Dattributes(FMOD::Studio::EventInstance* aEventInstance, EventData& aEventData);

		bool InitListener(ListenerData aListenerData);
		bool UpdateListener(ListenerData& aListenerData);

		bool SetMixerVolume(const std::string& aBusName, float aVolume);




	private:
		std::string rootDirectory;
		bool isInitialized;
		FMOD_RESULT lastResult = FMOD_OK;

		bool ErrorCheck_Critical(FMOD_RESULT result);
		bool ErrorCheck(FMOD_RESULT result);

	};


#pragma region DEPRICATED
	//bool CreateGeometry(GeometryData aNewGeometry);
	//void ReleaseGeometry();
	/*typedef std::unordered_map<int, GeometryInstance> GeometryMap;
	GeometryMap geometry;
	int geometryID = -1;*/

	/*struct GeometryData
	{
		FMOD_VECTOR position;
		FMOD_VECTOR scale;
		FMOD_VECTOR forward;
		FMOD_VECTOR up;
		std::vector<FMOD_VECTOR> verticies;

		int maxPolygons;
		int maxVertices;
	};

	struct GeometryInstance
	{
		int ID = -1;
		FMOD::Geometry* geometryObj;
		GeometryData data;
	};*/


#pragma endregion


}


