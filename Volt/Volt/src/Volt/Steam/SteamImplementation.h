#pragma once

#include "Volt/Core/Base.h"

#include <steam/steam_api.h>

#include <string>

namespace Volt
{
	// Enum for possible game states on the client
	enum EClientGameState
	{
		k_EClientGameStartServer,
		k_EClientGameActive,
		k_EClientGameWaitingForPlayers,
		k_EClientGameMenu,
		k_EClientGameQuitMenu,
		k_EClientGameExiting,
		k_EClientGameInstructions,
		k_EClientGameDraw,
		k_EClientGameWinner,
		k_EClientGameConnecting,
		k_EClientGameConnectionFailure,
		k_EClientFindInternetServers,
		k_EClientStatsAchievements,
		k_EClientCreatingLobby,
		k_EClientInLobby,
		k_EClientFindLobby,
		k_EClientJoiningLobby,
		k_EClientFindLANServers,
		k_EClientRemoteStorage,
		k_EClientLeaderboards,
		k_EClientFriendsList,
		k_EClientMinidump,
		k_EClientConnectingToSteam,
		k_EClientLinkSteamAccount,
		k_EClientAutoCreateAccount,
		k_EClientRetrySteamConnection,
		k_EClientClanChatRoom,
		k_EClientWebCallback,
		k_EClientMusic,
		k_EClientWorkshop,
		k_EClientHTMLSurface,
		k_EClientInGameStore,
		k_EClientRemotePlayInvite,
		k_EClientRemotePlaySessions,
		k_EClientOverlayAPI,
	};

	struct SteamData
	{
		// SteamID for the local user on this client
		CSteamID SteamIDLocalUser;
		CGameID gameId;

		// Current game state
		EClientGameState eGameState;
	};

	class SteamImplementation
	{
	public:
		SteamImplementation();
		virtual ~SteamImplementation();

		std::string GetPersona() const;

		void StartLobby(std::string address);
		void ClearRichPresence();

		void StoreStats();
		void RequestStats();

		void SetStat(std::string_view name, int32_t value);
		void SetStat(std::string_view name, float value);

		const int32_t GetStatInt(std::string_view name);
		const float GetStatFloat(std::string_view name);

		inline const bool IsStatsValid() const { return myStatsValid; }

		static Scope<SteamImplementation> Create();

		// Stats & Achievements
		STEAM_CALLBACK(SteamImplementation, OnUserStatsStored, UserStatsStored_t, myUserStatsStoredCallback);
		STEAM_CALLBACK(SteamImplementation, OnUserStatsRecieved, UserStatsReceived_t, myUserStatsRecievedCallback);
		STEAM_CALLBACK(SteamImplementation, OnUserAchievementsStored, UserAchievementStored_t, myUserAchievementsStoredCallback);

	private:
		friend class Application;
		
		void Update();
		void UpdateOccationally();

		bool mySteamInitialized = false;

		SteamData myData;

		ISteamUser* mySteamUser = nullptr;
		ISteamFriends* mySteamFriends = nullptr;
		ISteamUserStats* mySteamUserStats = nullptr;

		bool myRequestedStats = false;
		bool myStatsValid = false;

		// Lobby stuff
		STEAM_CALLBACK(SteamImplementation, OnLobbyGameCreated, LobbyGameCreated_t);
		STEAM_CALLBACK(SteamImplementation, OnGameJoinRequested, GameRichPresenceJoinRequested_t);

		//void OnLobbyCreated(LobbyCreated_t* pCallback, bool bIOFailure);
		//CCallResult<SteamImplementation, LobbyCreated_t> m_SteamCallResultLobbyCreated;

		//void OnLobbyEntered(LobbyEnter_t* pCallback, bool bIOFailure);
		//CCallResult<SteamImplementation, LobbyEnter_t> m_SteamCallResultLobbyEntered;
	};
}
