#pragma once

#include "Volt/Core/Base.h"

#include <discord/discord.h>

namespace Volt
{
	struct DiscordState
	{
		discord::User currentUser;
		discord::Activity currentActivity;
		Scope<discord::Core> core = nullptr;
	};

	class DiscordSDK
	{
	public:
		DiscordSDK() = delete;
		virtual ~DiscordSDK() = delete;

		static void Init(int64_t appId, bool isOverrideable = true);
		static discord::Activity& GetRichPresence() { return myState.currentActivity; };
		static void UpdateRichPresence();

	private:
		friend class Application;
		static void Update();
		
		inline static DiscordState myState;
	};
}
