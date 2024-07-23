#include "amppch.h"
#include <iostream>
#include <thread>

#include "WWiseEngine.h"

#include <AK/SoundEngine/Common/AkMemoryMgr.h>
#include <AK/SoundEngine/Common/AkSoundEngine.h>
#include <AK/SoundEngine/Common/AkModule.h>
#include <AK/SoundEngine/Common/IAkStreamMgr.h>

#include <AK/Tools/Common/AkPlatformFuncs.h>

#include "Amp/WWiseEngine/SoundEngine/Win32/AkFilePackageLowLevelIOBlocking.h"

#include "AK/Plugin/AkRoomVerbFXFactory.h"

//#ifndef AK_OPTIMIZED
//#include <AK/Comm/AkCommunication.h>
//#endif // AK_OPTIMIZED

Amp::WWiseEngine* Amp::WWiseEngine::myInstance = { nullptr };

namespace AK
{
	void* AllocHook(size_t s) { return malloc(s); }
	void FreeHook(void* p) { free(p); }
}

namespace Amp
{
	WWiseEngine::WWiseEngine()
	{
		myLowLevelIO = new CAkFilePackageLowLevelIOBlocking();
		GetDefaultSettings();
	}

	void WWiseEngine::GetDefaultSettings()
	{
		AK::MemoryMgr::GetDefaultSettings(myMemSettings);
		AK::StreamMgr::GetDefaultSettings(myStreamMgrSettings);
		AK::StreamMgr::GetDefaultDeviceSettings(myDeviceSettings);
		AK::SoundEngine::GetDefaultInitSettings(myInitSettings);
		//AK::MusicEngine::GetDefaultInitSettings(myMusicSettings);
		AK::SoundEngine::GetDefaultPlatformInitSettings(myPlatformSettings);
	}

	WWiseEngine::~WWiseEngine()
	{
		delete myLowLevelIO;
	}

	bool WWiseEngine::InitWWise(std::filesystem::path aDefaultPath)
	{
		if (AK::MemoryMgr::Init(&myMemSettings) != AK_Success)
		{
			assert(!"Could not create the memory manager.");
			return false;
		}

		if (!AK::StreamMgr::Create(myStreamMgrSettings))
		{
			assert(!"Could not create the Streaming Manager");
			return false;
		}
		AK::StreamMgr::SetCurrentLanguage(AKTEXT("English(US)"));


		if (myLowLevelIO->Init(myDeviceSettings) != AK_Success)
		{
			assert(!"Could not create the streaming device and Low-Level I/O system");
			return false;
		}
		myLowLevelIO->SetBasePath(aDefaultPath.c_str());


		if (AK::SoundEngine::Init(&myInitSettings, &myPlatformSettings) != AK_Success)
		{
			assert(!"Could not initialize the Sound Engine.");
			return false;
		}

		isInit = true;
		return isInit;
	}

	void WWiseEngine::TermWwise()
	{
		UnloadAllBanks();

		AK::SoundEngine::UnregisterAllGameObj();

		AK::SoundEngine::Term();
		myLowLevelIO->Term();

		if (AK::IAkStreamMgr::Get())
		{
			AK::IAkStreamMgr::Get()->Destroy();
		}

		AK::MemoryMgr::Term();

	}

	void WWiseEngine::Update()
	{
		AK::SoundEngine::RenderAudio();
	}

	bool WWiseEngine::LoadBank(const char* aBankFile)
	{
		AkBankID bankID;
		if (AK::SoundEngine::LoadBank(aBankFile, bankID) != AK_Success)
		{
			return false;
		}
		myLoadedBanks.insert({ bankID, aBankFile });
		return true;
	}

	bool WWiseEngine::UnloadBank(const char* aBankFile)
	{
		//TODO: MAKE LESS AKWARD
		for (auto bank : myLoadedBanks)
		{
			if (bank.second == aBankFile)
			{
				if (AK::SoundEngine::UnloadBank(aBankFile, nullptr) != AK_Success)
				{
					return false;
				}
				//myLoadedBanks.erase(bank.first);
				break;
			}
		}
		return true;
	}

	bool WWiseEngine::UnloadAllBanks()
	{
		for (auto bank : myLoadedBanks)
		{
			UnloadBank(bank.second.c_str());
		}

		return true;
	}

	Vector<std::string> WWiseEngine::GetBankNames()
	{
		Vector<std::string> bankNames;

		for (auto bank : myLoadedBanks)
		{
			if (bank.second == "Init.bnk")
			{
				continue;
			}

			bankNames.push_back(bank.second);
		}

		return bankNames;
	}

	bool WWiseEngine::RegisterListener(uint32_t aEntityID, const char* aEntityName, bool isDefault)
	{
		if (AK::SoundEngine::RegisterGameObj(aEntityID, aEntityName) != AK_Success)
		{
			//ERROR MESSAGE
			return false;
		}
		
		if (isDefault)
		{
			DEFAULT_LISTENER_ID = aEntityID;
			if (AK::SoundEngine::SetDefaultListeners(&DEFAULT_LISTENER_ID, 1) != AK_Success)
			{
				//ERROR MESSAGE
				return false;
			}
		}
		else
		{
			//TODO: IMPLEMENT MULTIPLE LISTNERS
			return false;
		}

		return true;
	}

	bool WWiseEngine::UnregisterListener()
	{
		if (AK::SoundEngine::RemoveDefaultListener(DEFAULT_LISTENER_ID) != AK_Success)
		{	
			//ERROR MESSAGE
			return false;
		}

		return true;
	}

	bool WWiseEngine::SetListenerPosition(const AkTransform& aListenerTransform)
	{
		if (AK::SoundEngine::SetPosition(DEFAULT_LISTENER_ID, aListenerTransform) != AK_Success)
		{
			//ERROR MESSAGE
			return false;
		}
		return true;
	}

	bool WWiseEngine::RegisterObject(const char* eventName, UINT64 AKGameObjectID)
	{
		if (AK::SoundEngine::RegisterGameObj(AKGameObjectID, eventName) != AK_Success)
		{
			return false;
		}
		return true;
	}

	bool WWiseEngine::UnRegisterObject(UINT64 AKGameObjectID)
	{
		if (AK::SoundEngine::UnregisterGameObj(AKGameObjectID) != AK_Success)
		{
			return false;
		}
		return true;
	}

	bool WWiseEngine::RemoveAllObjects()
	{
		if (AK::SoundEngine::UnregisterAllGameObj() != AK_Success)
		{
			return false;
		}
		return true;
	}

	bool WWiseEngine::SetObjectPosition(const UINT64& AKGameObjectID, const AkTransform& aAKTranform)
	{
		if (AK::SoundEngine::SetPosition(AKGameObjectID, aAKTranform) != AK_Success)
		{
			return false;
		}
		return true;
	}

	bool WWiseEngine::InitCommunications()
	{
//#ifndef AK_OPTIMIZED
//		AkCommSettings commSettings;
//		AK::Comm::GetDefaultInitSettings(commSettings);
//		if (AK::Comm::Init(commSettings) != AK_Success)
//		{
//			assert(!"WWISE: Could not initialize communication.");
//			return false;
//		}
//
//#endif // AK_OPTIMIZED
		return true;
	}

	//TODO: CHANGE THIS
	bool WWiseEngine::PlayOneShotEvent(const char* eventName, const AkTransform& aAKTransform)
	{
		AkGameObjectID myTempGameObj = 1;
		AK::SoundEngine::RegisterGameObj(myTempGameObj, "OneShotEvent");
		if (AK::SoundEngine::PostEvent(eventName, myTempGameObj) == AK_INVALID_PLAYING_ID)
		{
			assert(!"WWISE: Could not initialize a Event.");
		}
		AK::SoundEngine::UnregisterGameObj(myTempGameObj);
		return true;
	}

	bool WWiseEngine::PlayEvent(const char* eventName, UINT64 AKGameObjectID, AkPlayingID& outID)
	{
		AkPlayingID aID = AK::SoundEngine::PostEvent(eventName, AKGameObjectID);

		if (aID == AK_INVALID_PLAYING_ID)
		{
			assert(!"WWISE: Could not initialize a Event.");
		}

		outID = aID;

		return true;
	}

	bool WWiseEngine::ExecuteEventAction(AK::SoundEngine::AkActionOnEventType aAction, AkPlayingID aPlayingID)
	{
		AK::SoundEngine::ExecuteActionOnPlayingID(aAction, aPlayingID);
		return true;
	}

	bool WWiseEngine::StopAllEvents()
	{
		AK::SoundEngine::StopAll();
		return true;
	}

	bool WWiseEngine::StopAllEvents(UINT64 AkGameObjectID)
	{
		AK::SoundEngine::StopAll(AkGameObjectID);
		return true;
	}

	bool WWiseEngine::SetState(const char* aStateGroup, const char* aState)
	{
		if (AK::SoundEngine::SetState(aStateGroup, aState) != AK_Success)
		{
			return false;
		}
		return true;
	}

	bool WWiseEngine::SetSwitch(const char* aStateGroup, const char* aState, UINT64 AkGameObjectID)
	{
		if (AK::SoundEngine::SetSwitch(aStateGroup, aState, AkGameObjectID) != AK_Success)
		{
			return false;
		}
		return true;
	}

	bool WWiseEngine::SetGlobalParameter(const char* aParameterName, float aValue)
	{
		if (AK::SoundEngine::SetRTPCValue(aParameterName, aValue) != AK_Success)
		{
			return false;
		}
		return true;
	}

	bool WWiseEngine::SetParameter(const char* aParameterName, float aValue, UINT64 AkGameObjectID)
	{
		if (AK::SoundEngine::SetRTPCValue(aParameterName, aValue, AkGameObjectID) != AK_Success)
		{
			return false;
		}
		return true;
	}

	bool WWiseEngine::SetParameter(const char* aParameterName, float aValue, UINT64 AkGameObjectID, int32_t aOvertime)
	{
		if (AK::SoundEngine::SetRTPCValue(aParameterName, aValue, AkGameObjectID, AkTimeMs(aOvertime)) != AK_Success)
		{
			return false;
		}
		return true;
	}
}


