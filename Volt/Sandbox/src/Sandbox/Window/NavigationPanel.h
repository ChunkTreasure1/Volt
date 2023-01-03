#pragma once

#include "Sandbox/Window/EditorWindow.h"

#include "Volt/Asset/Mesh/Mesh.h"
#include "Volt/Scene/Scene.h"

class NavigationPanel : public EditorWindow
{
public:
	NavigationPanel(Ref<Volt::Scene>& currentScene) 
		: EditorWindow("Navigation"), myCurrentScene(currentScene) {}
	void UpdateMainContent() override;

private:
	Ref<Volt::Scene>& myCurrentScene;
};