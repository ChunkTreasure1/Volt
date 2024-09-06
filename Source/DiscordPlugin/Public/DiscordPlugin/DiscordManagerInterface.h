#pragma once

#include <string_view>

enum class ActivityType
{
	Playing,
	Streaming,
	Listening,
	Watching
};

class IDiscordManager
{
public:
	virtual ~IDiscordManager() = default;

	virtual void SetApplicationID(int64_t appId) = 0;
	virtual void SetDetails(std::string_view text) = 0;
	virtual void SetState(std::string_view text) = 0;
	virtual void SetStartTime(time_t time) = 0;
	virtual void SetLargeImage(std::string_view imageName) = 0;
	virtual void SetLargeText(std::string_view text) = 0;
	virtual void SetActivityType(ActivityType type) = 0;
	virtual void SetPartySize(int32_t size) = 0;
	virtual void SetMaxPartySize(int32_t size) = 0;

	virtual void UpdateChanges() = 0;
	virtual void Update() = 0;
};
