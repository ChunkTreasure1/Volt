#include "sbpch.h"
#include "Window/WorldEnginePanel.h"

#include <Volt/Utility/UIUtility.h>
#include <Volt/Math/Math.h>

WorldEnginePanel::WorldEnginePanel(Ref<Volt::Scene>& editorScene)
	: EditorWindow("World Engine"), m_editorScene(editorScene)
{
}

void WorldEnginePanel::UpdateMainContent()
{
	auto& worldEngine = m_editorScene->GetWorldEngineMutable();

	if (UI::BeginProperties("worldEngineSettings"))
	{
		if (UI::Property("Cell Size", worldEngine.GetSettingsMutable().cellSize))
		{
			worldEngine.GenerateCells();
		}

		if (UI::Property("World Size", worldEngine.GetSettingsMutable().worldSize))
		{
			worldEngine.GenerateCells();
		}

		UI::EndProperties();
	}

	ImGui::Text("Cells");

	const glm::uvec2& worldSize = worldEngine.GetSettings().worldSize;
	const int32_t cellSize = worldEngine.GetSettings().cellSize;

	const uint32_t cellCountX = Math::DivideRoundUp(worldSize.x, static_cast<uint32_t>(cellSize));
	const uint32_t cellCountY = Math::DivideRoundUp(worldSize.y, static_cast<uint32_t>(cellSize));

	const float contentWidth = ImGui::GetContentRegionAvail().x;
	const float buttonSize = contentWidth / static_cast<float>(cellCountX);

	UI::ScopedStyleFloat2 padding{ ImGuiStyleVar_ItemSpacing, { 0.f } };

	for (uint32_t x = 0; x < cellCountX; x++)
	{
		for (uint32_t y = 0; y < cellCountY; y++)
		{
			glm::vec4 color = { 1.f, 0.f, 0.f, 1.f };

			Volt::WorldCellID cellId = (x * cellCountX) + y;

			if (worldEngine.IsCellLoaded(cellId))
			{
				color = { 0.f, 1.f, 0.f, 1.f };
			}

			UI::ScopedColor buttonColor{ ImGuiCol_Button, color };

			std::string id = "Cell " + std::to_string(cellId) + "##" + std::to_string(x + y);
			if (ImGui::Button(id.c_str(), { buttonSize, buttonSize }))
			{
				worldEngine.BeginStreamingCell(cellId);
			}

			if (y < cellCountY - 1)
			{
				ImGui::SameLine();
			}
		}
	}
}

