#pragma once

#include "Project.h"

#include "Walnut/Application.h"
#include "Walnut/Image.h"

#include <future>

enum class Tab
{
	Projects,
	Engines
};

struct LauncherData
{
	std::vector<Project> projects;
	EngineInfo engineInfo{};
};

struct CreateProjectData
{
	std::string projectName;
	std::filesystem::path targetDir;
};

class LauncherLayer : public Walnut::Layer
{
public:
	void OnAttach() override;
	void OnDetach() override;

	void OnUIRender() override;

	void UI_DrawAboutModal();
	void UI_DrawNewProjectModal();

	void UI_DrawTabsChild();
	void UI_DrawContentChild();

	void UI_DrawProjectsContent();
	void UI_DrawEnginesContent();

	void ShowAboutModal();
	void ShowCreateNewProjectModal();

private:
	void CreateNewProject(const CreateProjectData& newData);

	void SerializeData();
	void DeserializeData();

	Project ReadProjectInfo(const std::filesystem::path& projectFilePath);
	void WriteProjectInfo(const std::filesystem::path& projectFilePath, const Project& project);
	
	const bool IsProjectRegistered(const std::filesystem::path& projectFilePath);

	bool m_aboutModalOpen = false;
	bool m_newProjectModalOpen = false;

	CreateProjectData m_newProjectData{};

	std::atomic_bool m_isInstalling = false;

	std::future<void> m_downloadFuture;

	std::shared_ptr<Walnut::Image> m_playIcon;
	std::shared_ptr<Walnut::Image> m_editIcon;
	std::shared_ptr<Walnut::Image> m_dotsIcon;

	Tab m_currentTab = Tab::Projects;
	LauncherData m_data;
};
