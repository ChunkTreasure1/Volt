#include "DiscordManager.h"

#include <LogModule/Log.h>

VT_DEFINE_LOG_CATEGORY(LogDiscord);

void DiscordManager::SetApplicationID(int64_t appId)
{
	m_state = {};

	discord::Core* corePtr;
	auto result = discord::Core::Create(appId, DiscordCreateFlags_Default, &corePtr);

	m_state.core.reset(corePtr);

	if (!m_state.core)
	{
		VT_LOGC(Error, LogDiscord, "Failed to instantiate discord core! Error: {}", static_cast<int>(result));
	}
}

void DiscordManager::SetDetails(std::string_view text)
{
	m_state.currentActivity.SetDetails(text.data());
}

void DiscordManager::SetState(std::string_view text)
{
	m_state.currentActivity.SetState(text.data());
}

void DiscordManager::SetStartTime(time_t time)
{
	m_state.currentActivity.GetTimestamps().SetStart(time);
}

void DiscordManager::SetLargeImage(std::string_view imageName)
{
	m_state.currentActivity.GetAssets().SetLargeImage(imageName.data());
}

void DiscordManager::SetLargeText(std::string_view text)
{
	m_state.currentActivity.GetAssets().SetLargeText(text.data());
}

void DiscordManager::SetActivityType(ActivityType type)
{
	m_state.currentActivity.SetType(static_cast<discord::ActivityType>(type));
}

void DiscordManager::SetPartySize(int32_t size)
{
	m_state.currentActivity.GetParty().GetSize().SetCurrentSize(size);
}

void DiscordManager::SetMaxPartySize(int32_t size)
{
	m_state.currentActivity.GetParty().GetSize().SetMaxSize(size);
}

void DiscordManager::UpdateChanges()
{
	if (m_state.core)
	{
		m_state.core->ActivityManager().UpdateActivity(m_state.currentActivity, [](discord::Result result) {
			if (result != discord::Result::Ok)
			{
				VT_LOGC(Warning, LogDiscord, "Failed to update discord rich presence");
			}
		});
	}
}

void DiscordManager::Update()
{
	if (m_state.core)
	{
		m_state.core->RunCallbacks();
	}
}
