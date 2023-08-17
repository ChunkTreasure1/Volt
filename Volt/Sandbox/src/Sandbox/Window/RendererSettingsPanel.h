#pragma once

#include "Sandbox/Window/EditorWindow.h"

namespace Volt
{
	class SceneRendererNew;
}

class RendererSettingsPanel : public EditorWindow
{
public:
	RendererSettingsPanel(Ref<Volt::SceneRendererNew>& sceneRenderer);
	void UpdateMainContent() override;

private:
	Ref<Volt::SceneRendererNew>& mySceneRenderer;
};
