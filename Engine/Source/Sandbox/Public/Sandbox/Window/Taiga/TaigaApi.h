#pragma once

#include <CoreUtilities/Containers/Vector.h>

#include <json.h>

#include <optional>

struct TaigaGeneralInfo
{
	std::string authToken = "";
	std::string refreshToken = "";
	std::optional<uint32_t> userId;

	std::optional<uint32_t> selectedProjectId;
	std::optional<uint32_t> selectedUserstoryId;
	std::optional<uint32_t> selectedTaskId;
	std::optional<uint32_t> selectedIssueId;

	bool filterAssignedUserOnly = true;
	bool filterHideClosed = true;
};

enum class ItemType : uint32_t
{
	Project,
	Milestone,
	Userstory,
	Task,
	Issue,
	Unknown
};

struct ItemEntry
{
	std::string name;
	uint32_t id;
	ItemType type;
};

class TaigaAPI
{
public:
	TaigaAPI() = delete;

	static bool Auth(const std::string& username, const std::string& password, TaigaGeneralInfo& info);
	static bool GetProjects(const TaigaGeneralInfo& info, Vector<ItemEntry>& outProjects);
	static bool GetUserstories(const TaigaGeneralInfo& info, Vector<ItemEntry>& outUserstories);
	static bool GetTasks(const TaigaGeneralInfo& info, Vector<ItemEntry>& outTasks);
	static bool GetIssues(const TaigaGeneralInfo& info, Vector<ItemEntry>& outIssues);
};
