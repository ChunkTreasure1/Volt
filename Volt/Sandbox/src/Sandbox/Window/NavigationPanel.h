#pragma once

#include "Sandbox/Window/EditorWindow.h"
#include "NavigationEditor/Builder/RecastBuilder.h"

#include <Volt/Core/Application.h>
#include <Volt/Events/ApplicationEvent.h>

#include <Sandbox/UserSettingsManager.h>

class NavigationPanel : public EditorWindow
{
public:
	NavigationPanel(Ref<Volt::Scene>& currentScene)
		: EditorWindow("Navigation Settings"), myBuildSettings(UserSettingsManager::GetSettings().navmeshBuildSettings), myBuilder(myBuildSettings), myScene(currentScene), myNavigationSystem(Volt::Application::Get().GetNavigationSystem())
	{
	}

	void UpdateMainContent() override;
	void Bake();

private:
	void AgentSettingsTab();
	void BuildSettingsTab();


	Ref<Volt::Mesh> CompileWorldMeshes();
	void SetDefaultAgentSettings();
	void SetDefaultBuildSettings();
	void CompileNavLinks();

	Ref<Volt::Mesh> TestMCut(Ref<Volt::Mesh> a, Ref<Volt::Mesh> b);

	Ref<Volt::Scene>& myScene;

	RecastBuildSettings& myBuildSettings;
	RecastBuilder myBuilder;
	Volt::AI::NavigationSystem& myNavigationSystem;
};
