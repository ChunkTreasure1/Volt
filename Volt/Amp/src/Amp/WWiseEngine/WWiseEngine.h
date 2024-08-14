#pragma once
#include <map>

#include "Amp/WWiseEngine/SoundEngine/Win32/AkFilePackageLowLevelIOBlocking.h"
#include "SoundEngine/Common/AkJobWorkerMgr.h"

#include <CoreUtilities/Containers/Vector.h>

#include "AK/SoundEngine/Common/AkMemoryMgr.h"		// Memory Manager
#include <AK/SoundEngine/Common/AkModule.h>			// Default memory and stream managers
#include <AK/SoundEngine/Common/IAkStreamMgr.h>		// Streaming Manager
#include <AK/SoundEngine/Common/AkSoundEngine.h>    // Sound engine
#include <AK/MusicEngine/Common/AkMusicEngine.h>	// Music Engine
#include <AK/SoundEngine/Common/AkStreamMgrModule.h>	// AkStreamMgrModule


namespace Amp
{
	class WWiseEngine
	{
	public:
		static AkResourceMonitorDataSummary ResourceDataSummary;
		static WWiseEngine& Get() { if (!myInstance) { myInstance = new WWiseEngine(); } return *myInstance; };

		~WWiseEngine();

		//ENGINE CONTROL
		bool InitWWise(std::filesystem::path aPath);
		void TermWwise();
		void Update();

		//BANK CONTROL
		bool LoadBank(const char* aBankFile);
		bool UnloadBank(const char* aBankFile);
		bool UnloadAllBanks();
		Vector<std::string> GetBankNames();


		//LISTENER CONTROL
		bool RegisterListener(uint32_t aEntityID, const char* aEntityName, bool isDefault);
		bool UnregisterListener();
		bool SetListenerPosition(const AkTransform& aAKTranform);

		//OBJECT CONTROL
		bool RegisterObject(const char* eventName, UINT64 AKGameObjectID);
		bool UnRegisterObject(UINT64 AKGameObjectID);

		bool RemoveAllObjects();

		bool SetObjectPosition(const UINT64& AKGameObjectID, const AkTransform& aAKTranform);

		//EVENT CONTROL
		bool PlayOneShotEvent(const char* eventName, const AkTransform& aAKTransform);
		bool PlayEvent(const char* eventName, UINT64 AKGameObjectID, AkPlayingID& outID);

		bool ExecuteEventAction(AK::SoundEngine::AkActionOnEventType aAction, AkPlayingID aPlayingID);

		bool StopAllEvents();
		bool StopAllEvents(UINT64 AkGameObjectID);

		//GAME SYNCS
		bool SetState(const char* aStateGroup, const char* aState);
		bool SetSwitch(const char* aStateGroup, const char* aState, UINT64 AkGameObjectID);

		bool SetGlobalParameter(const char* aParameterName, float aValue);

		bool SetParameter(const char* aParameterName, float aValue, UINT64 AkGameObjectID);
		bool SetParameter(const char* aParameterName, float aValue, UINT64 AkGameObjectID, int32_t aOvertime);

		//DEBUGGING
		bool InitCommunications();

	private:
		bool isInit = false;

		WWiseEngine();
		static WWiseEngine* myInstance;

		void GetDefaultSettings();

		std::map<AkBankID, std::string> myLoadedBanks;

		CAkFilePackageLowLevelIOBlocking* myLowLevelIO;

		AkMemSettings myMemSettings;
		AkStreamMgrSettings myStreamMgrSettings;
		AkDeviceSettings myDeviceSettings;
		AkInitSettings myInitSettings;
		//AkMusicSettings myMusicSettings;
		//AkSpatialAudioInitSettings mySpatialSettings;
		AkPlatformInitSettings myPlatformSettings;
		//AK::JobWorkerMgr::InitSettings myWorkerSettings;

		AkGameObjectID DEFAULT_LISTENER_ID = 0;


	};

}
