#pragma once

#include "Sandbox/Window/EditorWindow.h"

class WorldEnginePanel : public EditorWindow
{
public:
	WorldEnginePanel(Ref<Volt::Scene>& editorScene);

	void UpdateMainContent() override;

private:
	Ref<Volt::Scene>& m_editorScene;
};
