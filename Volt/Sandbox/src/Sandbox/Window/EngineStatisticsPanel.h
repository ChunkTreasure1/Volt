#pragma once

#include "Sandbox/Window/EditorWindow.h"

namespace Volt
{
	class Scene;
}

class EngineStatisticsPanel : public EditorWindow
{
public:
	EngineStatisticsPanel(Ref<Volt::Scene>& aScene);

	void UpdateMainContent() override;

private:
	Ref<Volt::Scene>& myScene;
};