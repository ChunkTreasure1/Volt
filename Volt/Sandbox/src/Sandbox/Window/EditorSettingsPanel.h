#pragma once

#include "Sandbox/Window/EditorWindow.h"

struct EditorSettings;

class EditorSettingsPanel : public EditorWindow
{
public:
	EditorSettingsPanel(EditorSettings& settings);

	void UpdateMainContent();

private:
	enum class SettingsMenu
	{
		VersionControl
	};

	void DrawOutline();
	void DrawView();

	void DrawVersionControl();

	EditorSettings& m_editorSettings;
	SettingsMenu m_currentMenu = SettingsMenu::VersionControl;

	int32_t m_currentStream = 0;
	int32_t m_currentWorkspace = 0;
};