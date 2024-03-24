#include "vtpch.h"
#include "SteamImplementation.h"

#include "Volt/Scripting/Mono/MonoScriptGlue.h"

namespace Volt
{
	bool ParseCommandLine(const char* pchCmdLine, const char** ppchServerAddress, const char** ppchLobbyID)
	{
		// Look for the +connect ipaddress:port parameter in the command line,
		// Steam will pass this when a user has used the Steam Server browser to find
		// a server for our game and is trying to join it. 
		const char* pchConnectParam = "+connect ";
		const char* pchConnect = strstr(pchCmdLine, pchConnectParam);
		*ppchServerAddress = NULL;
		if (pchConnect && strlen(pchCmdLine) > (pchConnect - pchCmdLine) + strlen(pchConnectParam))
		{
			// Address should be right after the +connect
			*ppchServerAddress = pchCmdLine + (pchConnect - pchCmdLine) + strlen(pchConnectParam);
		}

		// look for +connect_lobby lobbyid paramter on the command line
		// Steam will pass this in if a user taken up an invite to a lobby
		const char* pchConnectLobbyParam = "+connect_lobby ";
		const char* pchConnectLobby = strstr(pchCmdLine, pchConnectLobbyParam);
		*ppchLobbyID = NULL;
		if (pchConnectLobby && strlen(pchCmdLine) > (pchConnectLobby - pchCmdLine) + strlen(pchConnectLobbyParam))
		{
			// lobby ID should be right after the +connect_lobby
			*ppchLobbyID = pchCmdLine + (pchConnectLobby - pchCmdLine) + strlen(pchConnectLobbyParam);
		}

		return *ppchServerAddress || *ppchLobbyID;

	}

#pragma warning( push )
	//  warning C4355: 'this' : used in base member initializer list
	//  This is OK because it's warning on setting up the Steam callbacks, they won't use this until after construction is done
#pragma warning( disable : 4355 ) 

	SteamImplementation::SteamImplementation()
		: myUserStatsStoredCallback(this, &SteamImplementation::OnUserStatsStored),
		myUserStatsRecievedCallback(this, &SteamImplementation::OnUserStatsRecieved),
		myUserAchievementsStoredCallback(this, &SteamImplementation::OnUserAchievementsStored)
	{
		if (SteamAPI_Init())
		{
			mySteamInitialized = true;
			mySteamUser = SteamUser();

			if (mySteamUser->BLoggedOn())
			{
				myData.SteamIDLocalUser = mySteamUser->GetSteamID();
				myData.eGameState = k_EClientGameMenu;
				myData.gameId = CGameID{ SteamUtils()->GetAppID() };

				mySteamUserStats = SteamUserStats();
				mySteamFriends = SteamFriends();
			}
		}
	}
#pragma warning( pop )

	SteamImplementation::~SteamImplementation()
	{
		if (mySteamInitialized)
		{
			SteamAPI_Shutdown();
		}
	}

	void SteamImplementation::StartLobby(std::string address)
	{
		if (!mySteamInitialized || !mySteamFriends)
		{
			return;
		}

		mySteamFriends->SetRichPresence("status", "Creating a lobby");
		mySteamFriends->SetRichPresence("connect", std::format("+connect {0}", address).c_str());
	}

	void SteamImplementation::ClearRichPresence()
	{
		if (!mySteamInitialized || !mySteamFriends)
		{
			return;
		}

		mySteamFriends->ClearRichPresence();
	}

	std::string SteamImplementation::GetPersona() const
	{
		if (mySteamInitialized)
		{
			return mySteamFriends->GetPersonaName();
		}
		return "";
	}

	void SteamImplementation::StoreStats()
	{
		if (!mySteamInitialized || !mySteamUserStats)
		{
			return;
		}

		mySteamUserStats->StoreStats();
	}

	void SteamImplementation::RequestStats()
	{
		if (!mySteamInitialized || !mySteamUserStats)
		{
			return;
		}

		myRequestedStats = mySteamUserStats->RequestCurrentStats();
	}

	void SteamImplementation::SetStat(std::string_view name, int32_t value)
	{
		if (!mySteamInitialized || !mySteamUserStats || !myStatsValid)
		{
			return;
		}

		mySteamUserStats->SetStat(name.data(), value);
	}

	void SteamImplementation::SetStat(std::string_view name, float value)
	{
		if (!mySteamInitialized || !mySteamUserStats || !myStatsValid)
		{
			return;
		}

		mySteamUserStats->SetStat(name.data(), value);
	}

	const int32_t SteamImplementation::GetStatInt(std::string_view name)
	{
		if (!mySteamInitialized || !mySteamUserStats || !myStatsValid)
		{
			return 0;
		}

		int32_t value = 0;
		
		if (!mySteamUserStats->GetStat(name.data(), &value))
		{
			return 0;
		}

		return value;
	}

	const float SteamImplementation::GetStatFloat(std::string_view name)
	{
		if (!mySteamInitialized || !mySteamUserStats || !myStatsValid)
		{
			return 0.f;
		}

		float value = 0;

		if (!mySteamUserStats->GetStat(name.data(), &value))
		{
			return 0.f;
		}

		return value;
	}

	void SteamImplementation::Update()
	{
		if (!mySteamInitialized)
		{
			return;
		}

		SteamAPI_RunCallbacks();

		// Run every second
		{
			static time_t lastCheck = 0;
			time_t now = time(nullptr);

			if (now != lastCheck)
			{
				lastCheck = now;
				UpdateOccationally();
			}
		}

	}

	void SteamImplementation::UpdateOccationally()
	{
		if (!mySteamUser || !mySteamUserStats)
		{
			myRequestedStats = true;
		}

		if (!myRequestedStats)
		{
			bool success = mySteamUserStats->RequestCurrentStats();
			myRequestedStats = success;
		}
	}

	//void SteamImplementation::OnLobbyCreated(LobbyCreated_t* pCallback, bool bIOFailure)
	//{

	//}

	//void SteamImplementation::OnLobbyEntered(LobbyEnter_t* pCallback, bool bIOFailure)
	//{

	//}

	Scope<SteamImplementation> SteamImplementation::Create()
	{
		return CreateScope<SteamImplementation>();
	}

	void SteamImplementation::OnUserStatsRecieved(UserStatsReceived_t* callback)
	{
		if (!mySteamUserStats || callback->m_nGameID != myData.gameId.ToUint64())
		{
			return;
		}

		if (callback->m_eResult != k_EResultOK)
		{
			return;
		}

		myStatsValid = true;
	}

	void SteamImplementation::OnUserStatsStored(UserStatsStored_t* callback)
	{
		if (!mySteamUserStats || callback->m_nGameID != myData.gameId.ToUint64())
		{
			return;
		}

		if (callback->m_eResult == k_EResultInvalidParam)
		{
			UserStatsReceived_t invalidParamCallback;
			invalidParamCallback.m_eResult = k_EResultOK;
			invalidParamCallback.m_nGameID = myData.gameId.ToUint64();
			OnUserStatsRecieved(&invalidParamCallback);
		}
	}

	void SteamImplementation::OnUserAchievementsStored(UserAchievementStored_t* callback)
	{
		if (!mySteamUserStats)
		{
			return;
		}
	}

	void SteamImplementation::OnGameJoinRequested(GameRichPresenceJoinRequested_t* pCallback)
	{
		// parse out the connect 
		const char* pchServerAddress, *pchLobbyID;

		if (ParseCommandLine(pCallback->m_rgchConnect, &pchServerAddress, &pchLobbyID))
		{
			MonoScriptGlue::SteamAPI_OnJoinRequest(pchServerAddress);
		}
	}

	void SteamImplementation::OnLobbyGameCreated(LobbyGameCreated_t* pCallback)
	{
		if (myData.eGameState != k_EClientInLobby)
		{
			return;
		}
	}
}
