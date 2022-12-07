#pragma once
#include "Sandbox/Window/EditorWindow.h"

#include <imgui.h>

class ThemesPanel : public EditorWindow
{
public:
	ThemesPanel();
	void UpdateMainContent() override;

	void SetMinecraftTheme();

private:
	ImGuiIO* myIo;
};