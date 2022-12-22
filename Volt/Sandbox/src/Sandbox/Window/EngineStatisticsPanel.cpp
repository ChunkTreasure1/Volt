#include "sbpch.h"
#include "EngineStatisticsPanel.h"

#include <Volt/Rendering/Renderer.h>
#include <Volt/Scene/Scene.h>

EngineStatisticsPanel::EngineStatisticsPanel(Ref<Volt::Scene>& aScene)
	: EditorWindow("Engine Statistics"), myScene(aScene)
{}

void EngineStatisticsPanel::UpdateMainContent()
{
	if (ImGui::CollapsingHeader("Renderer", ImGuiTreeNodeFlags_DefaultOpen))
	{
	}

	if (ImGui::CollapsingHeader("Scene", ImGuiTreeNodeFlags_DefaultOpen))
	{
		const auto& stats = myScene->GetStatistics();
		ImGui::Text("Entity count: %d", stats.entityCount);
	}
}
