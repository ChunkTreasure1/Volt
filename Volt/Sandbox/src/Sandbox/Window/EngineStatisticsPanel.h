#pragma once

#include "Sandbox/Window/EditorWindow.h"

namespace Volt
{
	class Scene;
	class SceneRenderer;
}

class EngineStatisticsPanel : public EditorWindow
{
public:
	EngineStatisticsPanel(Ref<Volt::Scene>& aScene, Ref<Volt::SceneRenderer>& sceneRenderer, Ref<Volt::SceneRenderer>& gameSceneRenderer);

	void UpdateMainContent() override;

private:
	Ref<Volt::Scene>& myScene;
	Ref<Volt::SceneRenderer>& mySceneRenderer;
	Ref<Volt::SceneRenderer>& myGameSceneRenderer;
};
