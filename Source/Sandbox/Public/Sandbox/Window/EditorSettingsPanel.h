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
		VersionControl,
		ExternalTools,
		StyleSettings,
		EditorSettings
	};

	void DrawOutline();
	void DrawView();

	void DrawVersionControl();
	void DrawExternalTools();
	void DrawStyleSettings();
	void DrawEditorSettings();

	EditorSettings& m_editorSettings;
	SettingsMenu m_currentMenu = SettingsMenu::VersionControl;

	int32_t m_currentStream = 0;
	int32_t m_currentWorkspace = 0;
};
