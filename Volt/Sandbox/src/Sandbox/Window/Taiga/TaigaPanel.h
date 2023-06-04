#pragma once

#include "Sandbox/Window/EditorWindow.h"
#include "TaigaApi.h"

class TaigaPanel : public EditorWindow
{
public:
	TaigaPanel();
	~TaigaPanel() override = default;

	void UpdateMainContent() override;
	void UpdateContent() override;

private:
	void LogInPopup();
	void SettingsPopup();

	void ProjectsTab();
	void MilestonesTab();
	void UserstoriesTab();
	void TasksTab();
	void IssuesTab();

	void ItemTab();

	void DrawEntry(const ItemEntry& item, uint32_t index, uint32_t& selectedIndex);

	std::string username, password;
	TaigaGeneralInfo info;

	uint32_t selectedProject = 0;
	uint32_t selectedMilestone = 0;
	uint32_t selectedUserstory = 0;
	uint32_t selectedTask = 0;
	uint32_t selectedIssue = 0;

	std::vector<ItemEntry> myProjects;
	std::vector<ItemEntry> myMilestones;
	std::vector<ItemEntry> myUserstories;
	std::vector<ItemEntry> myTasks;
	std::vector<ItemEntry> myIssues;

	ItemType selectedTab = ItemType::Unknown;
};
