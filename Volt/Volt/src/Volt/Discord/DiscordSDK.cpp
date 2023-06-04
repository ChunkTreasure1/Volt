#include "vtpch.h"
#include "DiscordSDK.h"

#include "Volt/Log/Log.h"

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
				VT_CORE_ERROR("Failed to instantiate discord core! (err ", static_cast<int>(result), ")");
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
					VT_CORE_WARN("Failed to update discord rich presence");
				}
			});
		}
	}
}
