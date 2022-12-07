#include "sbpch.h"
#include "EngineStatisticsPanel.h"

#include <Volt/Rendering/Renderer.h>
#include <Volt/Scene/Scene.h>

EngineStatisticsPanel::EngineStatisticsPanel(Ref<Volt::Scene>& aScene)
	: EditorWindow("Engine Statistics"), myScene(aScene)
{}

void EngineStatisticsPanel::UpdateMainContent()
{


#ifdef VT_PROFILE_GPU
	if (ImGui::CollapsingHeader("Engine Times", ImGuiTreeNodeFlags_DefaultOpen))
	{
		for (const auto& [name, profilingStats] : Volt::Renderer::GetProfilingData())
		{
			std::string id = name + "##times";

			if (ImGui::TreeNode(id.c_str()))
			{
				const uint32_t lastFrame = profilingStats.lastFrame;

				ImGui::Text("Total GPU Time: %.2fms", profilingStats.frameAverageGPUTime);
				for (const auto& sectionName : profilingStats.sectionNames.at(lastFrame))
				{
					if (profilingStats.sectionAverageTimes.find(sectionName) != profilingStats.sectionAverageTimes.end())
					{
						ImGui::Text("%s: %.2fms", sectionName.c_str(), profilingStats.sectionAverageTimes.at(sectionName));
					}
				}

				ImGui::TreePop();
			}
		}
	}
#endif

	if (ImGui::CollapsingHeader("Renderer", ImGuiTreeNodeFlags_DefaultOpen))
	{
		const auto& stats = Volt::Renderer::GetStatistics();


		if (ImGui::CollapsingHeader("Global"))
		{
			ImGui::Text("Draw calls: %d", stats.drawCalls);
			ImGui::Text("Material binds: %d", stats.materialBinds);
			ImGui::Text("Vertex/Index buffer binds: %d", stats.vertexIndexBufferBinds);
		}

		if (ImGui::CollapsingHeader("Context"))
		{
			for (const auto& [name, data] : Volt::Renderer::GetProfilingData())
			{
				const auto& pipelineStats = data.sectionPipelineDatas;
				const uint32_t lastFrame = data.lastFrame;

				std::string id = name + "##renderer";

				if (ImGui::TreeNode(id.c_str()))
				{
					uint32_t index = 0;
					for (const auto& sectionName : data.sectionNames.at(lastFrame))
					{
						if (stats.perSectionStatistics.find(sectionName) == stats.perSectionStatistics.end())
						{
							break;
						}

						const auto& sectionStats = stats.perSectionStatistics.at(sectionName);

						std::string id = sectionName + "##" + std::to_string(index);
						if (ImGui::TreeNode(id.c_str()))
						{
							ImGui::Text("Draw calls: %d", sectionStats.drawCalls);
							ImGui::Text("Material binds: %d", sectionStats.materialBinds);
							ImGui::Text("Vertex/Index buffer binds: %d", sectionStats.vertexIndexBufferBinds);

#ifdef VT_PROFILE_GPU
							ImGui::Text("Total Vertex Count: %d", pipelineStats.at(lastFrame).at(sectionName).vertexCount);
							ImGui::Text("Total Primitive Count: %d", pipelineStats.at(lastFrame).at(sectionName).primitiveCount);
							ImGui::Text("Total Vertex Invocations: %d", pipelineStats.at(lastFrame).at(sectionName).vsInvocations);
							ImGui::Text("Total Pixel Invocations: %d", pipelineStats.at(lastFrame).at(sectionName).psInvocations);
#endif
							ImGui::TreePop();
						}
					}
					ImGui::TreePop();
				}
			}
		}
	}

	if (ImGui::CollapsingHeader("Scene", ImGuiTreeNodeFlags_DefaultOpen))
	{
		const auto& stats = myScene->GetStatistics();
		ImGui::Text("Entity count: %d", stats.entityCount);
	}
}
