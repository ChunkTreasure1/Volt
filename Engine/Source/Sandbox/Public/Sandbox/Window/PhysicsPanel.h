#pragma once

#include "Sandbox/Window/EditorWindow.h"

class PhysicsPanel : public EditorWindow
{
public:
	PhysicsPanel();

	void UpdateMainContent() override;

private:
	enum class Menu
	{
		General,
		Layers
	};

	void DrawOutline();
	void DrawView();

	void DrawGeneralMenu();
	void DrawLayersMenu();

	Menu myCurrentMenu = Menu::General;
};