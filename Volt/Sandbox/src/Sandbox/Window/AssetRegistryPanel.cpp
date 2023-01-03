#include "sbpch.h"
#include "AssetRegistryPanel.h"

#include <Volt/Asset/AssetManager.h>
#include <Volt/Utility/UIUtility.h>

AssetRegistryPanel::AssetRegistryPanel()
	: EditorWindow("Asset Registry")
{
	myWindowFlags = ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse;
}

void AssetRegistryPanel::UpdateMainContent()
{
	if (ImGui::BeginTable("RegistryTable", 2, ImGuiTableFlags_BordersInnerV | ImGuiTableFlags_BordersInnerH | ImGuiTableFlags_SizingStretchProp | ImGuiTableFlags_Resizable | ImGuiTableFlags_RowBg | ImGuiTableFlags_ScrollY))
	{
		ImGui::TableSetupColumn("Path", ImGuiTableColumnFlags_WidthStretch);
		ImGui::TableSetupColumn("Handle", ImGuiTableColumnFlags_WidthStretch, 0.4f);
		ImGui::TableSetupScrollFreeze(ImGui::TableGetColumnCount(), 1);
		ImGui::TableHeadersRow();

		for (const auto& [path, handle] : Volt::AssetManager::Get().GetAssetRegistry())
		{
			ImGui::TableNextColumn();
			const std::filesystem::path completePath = Volt::AssetManager::IsEngineAsset(path) ? path : Volt::ProjectManager::GetDirectory() / path;

			if (!FileSystem::Exists(completePath))
			{
				ImGui::TableSetBgColor(ImGuiTableBgTarget_RowBg0, ImColor(1.f, 0.f, 0.f, 1.f));
			}
			ImGui::PushItemWidth(ImGui::GetColumnWidth());
			std::string pathString = path.string();
			UI::InputText("", pathString, ImGuiInputTextFlags_ReadOnly);
			ImGui::PopItemWidth();

			ImGui::TableNextColumn();

			const float buttonPadding = 10.f;
			const float buttonSize = 22.f;

			ImGui::PushItemWidth(ImGui::GetColumnWidth() - buttonSize - buttonPadding);
			std::string handleText = std::to_string(handle);
			UI::InputText("", handleText, ImGuiInputTextFlags_ReadOnly);
			ImGui::PopItemWidth();
		
			ImGui::SameLine();

			const std::string remId = "-##" + std::to_string(handle);
			if (ImGui::Button(remId.c_str(), { buttonSize, buttonSize }))
			{
				Volt::AssetManager::Get().RemoveFromRegistry(handle);
				break;
			}
		}

		ImGui::EndTable();
	}
}