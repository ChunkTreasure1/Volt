#include "sbpch.h"
#include "EngineStatisticsPanel.h"

#include <Volt/Core/Application.h>
#include <Volt/Scene/Scene.h>

#include <Volt/Rendering/SceneRenderer.h>

#include <Volt/Utility/UIUtility.h>
#include <Volt/Utility/StringUtility.h>

EngineStatisticsPanel::EngineStatisticsPanel(Ref<Volt::Scene>& aScene, Ref<Volt::SceneRenderer>& sceneRenderer, Ref<Volt::SceneRenderer>& gameSceneRenderer)
	: EditorWindow("Engine Statistics"), myScene(aScene), mySceneRenderer(sceneRenderer), myGameSceneRenderer(gameSceneRenderer)
{
}

void EngineStatisticsPanel::UpdateMainContent()
{
	if (ImGui::CollapsingHeader("Application", ImGuiTreeNodeFlags_DefaultOpen))
	{
		const float appTime = Volt::Application::Get().GetAverageFrameTime();
		const float maxAppTime = Volt::Application::Get().GetMaxFrameTime();

		ImGui::Text("Main Thread Average Time: %.2f ms", appTime);
		ImGui::Text("Average FPS: %.1f", 1.f / appTime * 1000.f);
		ImGui::Text("Min FPS: %.1f", 1.f / maxAppTime * 1000.f);
	}

	if (ImGui::CollapsingHeader("Renderer"))
	{
		UI::Header("Time");

		/*const auto timeInfo = (myScene->IsPlaying() && myGameSceneRenderer) ? myGameSceneRenderer->GetTimestamps() : mySceneRenderer->GetTimestamps();
		for (const auto& timestamp : timeInfo)
		{
			ImGui::TextUnformatted(std::format("{}: {:.2f} ms", timestamp.label, timestamp.time).c_str());
		}

		ImGui::Separator();

		UI::Header("Statistics");

		const auto& stats = (myScene->IsPlaying() && myGameSceneRenderer) ? myGameSceneRenderer->GetStatistics() : mySceneRenderer->GetStatistics();
		const auto& pipelineStats = (myScene->IsPlaying() && myGameSceneRenderer) ? myGameSceneRenderer->GetStatistics().pipelineStatistics : mySceneRenderer->GetStatistics().pipelineStatistics;

		ImGui::Text("Input Assembly Vertices: %d", pipelineStats.inputAssemblyVertices);
		ImGui::Text("Input Assembly Primitives: %d", pipelineStats.inputAssemblyPrimitives);
		ImGui::Text("Vertex Shader Invocations: %d", pipelineStats.vertexShaderInvocations);
		ImGui::Text("Clipping Invocations: %d", pipelineStats.clippingInvocations);
		ImGui::Text("Clipping Primitives: %d", pipelineStats.clippingPrimitives);
		ImGui::Text("Pixel Shader Invocations: %d", pipelineStats.fragmentShaderInvocations);
		ImGui::Text("Compute Shader Invocations: %d", pipelineStats.computeShaderInvocations);
		ImGui::Text("Total Scene Triangle Count: %d", stats.triangleCount); */

		UI::Header("Allocation");

		std::string allocSizeStr = Utility::ToStringWithMetricPrefixCharacterForBytes(mySceneRenderer->GetFrameTotalGPUAllocationSize());

		ImGui::Text("Frame Total GPU Allocation: %s", allocSizeStr.c_str());
	}

	if (ImGui::CollapsingHeader("Scene", ImGuiTreeNodeFlags_DefaultOpen))
	{
		const auto& stats = myScene->GetStatistics();
		ImGui::Text("Entity count: %d", stats.entityCount);
	}
}
