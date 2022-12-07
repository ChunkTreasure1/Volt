#pragma once

#include "Sandbox/Window/EditorWindow.h"

namespace Volt
{
	class SceneRenderer;
}

class RendererSettingsPanel : public EditorWindow
{
public:
	RendererSettingsPanel(Ref<Volt::SceneRenderer>& sceneRenderer);
	void UpdateMainContent() override;

private:
	Ref<Volt::SceneRenderer>& mySceneRenderer;
};