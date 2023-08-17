#pragma once

#include "Sandbox/Window/EditorWindow.h"

namespace Volt
{
	class Scene;
	class SceneRendererNew;
}

class EngineStatisticsPanel : public EditorWindow
{
public:
	EngineStatisticsPanel(Ref<Volt::Scene>& aScene, Ref<Volt::SceneRendererNew>& sceneRenderer, Ref<Volt::SceneRendererNew>& gameSceneRenderer);

	void UpdateMainContent() override;

private:
	Ref<Volt::Scene>& myScene;
	Ref<Volt::SceneRendererNew>& mySceneRenderer;
	Ref<Volt::SceneRendererNew>& myGameSceneRenderer;
};
