#pragma once

#include "Project.h"

#include "Walnut/Application.h"

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

class LauncherLayer : public Walnut::Layer
{
public:
	void OnAttach() override;
	void OnUIRender() override;

	void UI_DrawAboutModal();

	void UI_DrawTabsChild();
	void UI_DrawContentChild();

	void UI_DrawProjectsContent();
	void UI_DrawEnginesContent();

	void ShowAboutModal();

private:
	bool m_aboutModalOpen = false;

	Tab m_currentTab = Tab::Projects;
	LauncherData m_data;
};
