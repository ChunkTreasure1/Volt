#include "vtpch.h"
#include "DiscordSDK.h"

namespace Volt
{
	void DiscordSDK::Init(int64_t appId, bool isOverrideable)
	{
		static bool s_isOverrideable = true;
		if (s_isOverrideable)
		{
			discord::Core* core{};
			auto result = discord::Core::Create(appId, DiscordCreateFlags_Default, &core);

			myState.core.reset(core);

			if (!myState.core)
			{
				VT_LOG(Error, "Failed to instantiate discord core! (err ", static_cast<int>(result), ")");
				return;
			}

			s_isOverrideable = isOverrideable;
		}
	}

	void DiscordSDK::Update()
	{
		if (myState.core)
		{
			myState.core->RunCallbacks();
		}
	}

	void DiscordSDK::UpdateRichPresence()
	{
		if (myState.core)
		{
			myState.core->ActivityManager().UpdateActivity(myState.currentActivity, [](discord::Result result) {
				if (result != discord::Result::Ok)
				{
					VT_LOG(Warning, "Failed to update discord rich presence");
				}
			});
		}
	}
}
