#include "sbpch.h"
#include "TaigaPanel.h"

#include <Volt/Utility/UIUtility.h>

TaigaPanel::TaigaPanel()
	: EditorWindow("Taiga")
{
}

void TaigaPanel::UpdateMainContent()
{
	if (ImGui::Button("Auth")) { UI::OpenPopup("TaigaLogin"); }
	if (ImGui::Button("Settings")) { UI::OpenPopup("TaigaSettings"); }
	LogInPopup();
	SettingsPopup();
}

void TaigaPanel::UpdateContent()
{
	ImGui::Begin("Main View", nullptr, ImGuiWindowFlags_NoTabBar);
	if (ImGui::BeginTabBar("tabs"))
	{
		if (ImGui::BeginTabItem("Projects"))
		{
			if (selectedTab != ItemType::Project)
			{
				TaigaAPI::GetProjects(info, myProjects);
			}
			selectedTab = ItemType::Project;
			ProjectsTab();
			ImGui::EndTabItem();
		}

		if (ImGui::BeginTabItem("Milestones"))
		{
			if (selectedTab != ItemType::Milestone)
			{
				//TaigaAPI::GetUserstories(info, myUserstories);
			}
			selectedTab = ItemType::Milestone;
			MilestonesTab();
			ImGui::EndTabItem();
		}

		if (ImGui::BeginTabItem("Userstories"))
		{
			if (selectedTab != ItemType::Userstory)
			{
				TaigaAPI::GetUserstories(info, myUserstories);
			}
			selectedTab = ItemType::Userstory;
			UserstoriesTab();
			ImGui::EndTabItem();
		}

		if (ImGui::BeginTabItem("Tasks"))
		{
			if (selectedTab != ItemType::Task)
			{
				TaigaAPI::GetTasks(info, myTasks);
			}
			selectedTab = ItemType::Task;
			TasksTab();
			ImGui::EndTabItem();
		}

		if (ImGui::BeginTabItem("Issues"))
		{
			if (selectedTab != ItemType::Issue)
			{
				//TaigaAPI::GetIssues(info, s_issues);
			}
			selectedTab = ItemType::Issue;
			IssuesTab();
			ImGui::EndTabItem();
		}
		ImGui::EndTabBar();
	}
	ImGui::End();
	ImGui::Begin("Item View", nullptr, ImGuiWindowFlags_NoTabBar);
	ItemTab();
	ImGui::End();
}

void TaigaPanel::LogInPopup()
{
	if (UI::BeginPopup("TaigaLogin", ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse))
	{
		UI::InputText("Username", username);
		UI::InputText("Password", password, ImGuiInputTextFlags_Password);

		if (ImGui::Button("Login"))
		{
			if (TaigaAPI::Auth(username, password, info))
			{
				TaigaAPI::GetProjects(info, myProjects);
			}
			else
			{
				VT_LOG(LogSeverity::Warning, "Taiga Auth Failed.");
			}
			password.clear();
			ImGui::CloseCurrentPopup();
		}
		UI::EndPopup();
	}
}

void TaigaPanel::SettingsPopup()
{
	if (UI::BeginPopup("TaigaSettings", ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse))
	{
		ImGui::Checkbox("Show only assigned to me", &info.filterAssignedUserOnly);
		ImGui::Checkbox("Hide closed", &info.filterHideClosed);
		UI::EndPopup();
	}
}

void TaigaPanel::ProjectsTab()
{
	for (uint32_t i = 0; const auto & project : myProjects)
	{
		DrawEntry(project, i, selectedProject);
		i++;
	}
}

void TaigaPanel::MilestonesTab()
{
	for (uint32_t i = 0; const auto & milestone : myMilestones)
	{
		DrawEntry(milestone, i, selectedMilestone);
		i++;
	}
}

void TaigaPanel::UserstoriesTab()
{
	for (uint32_t i = 0; const auto & userstory : myUserstories)
	{
		DrawEntry(userstory, i, selectedUserstory);
		i++;
	}
}

void TaigaPanel::TasksTab()
{
	for (uint32_t i = 0; const auto & task : myTasks)
	{
		DrawEntry(task, i, selectedTask);
		i++;
	}
}

void TaigaPanel::IssuesTab()
{
	for (uint32_t i = 0; const auto & issue : myIssues)
	{
		DrawEntry(issue, i, selectedIssue);
		i++;
	}
}

void TaigaPanel::ItemTab()
{
	switch (selectedTab)
	{
		case ItemType::Project:
		{
			if (selectedProject < myProjects.size())
			{
				ImGui::Text(std::format("Name: {0}", myProjects[selectedProject].name).c_str());
				ImGui::Text(std::format("Id: {0}", std::to_string(myProjects[selectedProject].id)).c_str());
			}
			break;
		}
		case ItemType::Userstory:
		{
			if (selectedUserstory < myUserstories.size())
			{
				ImGui::Text(std::format("Name: {0}", myUserstories[selectedUserstory].name).c_str());
				ImGui::Text(std::format("Id: {0}", std::to_string(myUserstories[selectedUserstory].id)).c_str());
			}
			break;
		}
		case ItemType::Task:
		{
			if (selectedTask < myTasks.size())
			{
				ImGui::Text(std::format("Name: {0}", myTasks[selectedTask].name).c_str());
				ImGui::Text(std::format("Id: {0}", std::to_string(myTasks[selectedTask].id)).c_str());
			}
			break;
		}
		case ItemType::Issue:
		{
			if (selectedIssue < myIssues.size())
			{
				ImGui::Text(std::format("Name: {0}", myIssues[selectedIssue].name).c_str());
				ImGui::Text(std::format("Id: {0}", std::to_string(myIssues[selectedIssue].id)).c_str());
			}
			break;
		}
		case ItemType::Unknown:
		default:
			break;
	}
}

void TaigaPanel::DrawEntry(const ItemEntry& item, uint32_t index, uint32_t& selectedIndex)
{
	if (ImGui::Selectable((item.name + "###" + std::to_string(index)).c_str(), index == selectedIndex))
	{
		selectedIndex = index;
		switch (item.type)
		{
			case ItemType::Project:
				info.selectedProjectId = myProjects[selectedIndex].id;
				break;
			case ItemType::Userstory:
				info.selectedUserstoryId = myUserstories[selectedIndex].id;
				break;
			case ItemType::Task:
				info.selectedTaskId = myTasks[selectedIndex].id;
				break;
			case ItemType::Issue:
				info.selectedIssueId = myIssues[selectedIndex].id;
				break;
			case ItemType::Unknown:
			default:
				break;
		}
	}
}
