#pragma once
#pragma once

namespace Volt
{
	class MonoScriptGlue
	{
	public:
		static void RegisterFunctions();

		static void SteamAPI_Clean();
		static void SteamAPI_OnJoinRequest(std::string address);

		static void Net_OnConnectCallback();

	private:
		MonoScriptGlue() = delete;
	};
}
