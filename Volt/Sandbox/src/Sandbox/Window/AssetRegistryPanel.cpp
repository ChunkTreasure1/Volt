#include "sbpch.h"
#include "AssetRegistryPanel.h"
#include "Sandbox/Utility/EditorUtilities.h"

#include <AssetSystem/AssetManager.h>
#include <Volt/Utility/UIUtility.h>
#include <Volt/Utility/StringUtility.h>

AssetRegistryPanel::AssetRegistryPanel()
	: EditorWindow("Asset Registry")
{
	m_windowFlags = ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse;
}

void AssetRegistryPanel::UpdateMainContent()
{
	if (ImGui::Button("Add"))
	{
		UI::OpenModal("Add New Handle##AssetRegistryPanel");
	}

	AddNewModal();

	static bool hasReds = false;

	if (hasReds)
	{
		ImGui::SameLine();

		ImGui::PushStyleColor(ImGuiCol_Button, { 1.f, 0.f, 0.f, 1.f });
		if (ImGui::Button("Remove Reds"))
		{
			static Vector<std::filesystem::path> pathsToClear;
			for (const auto& [handle, metadata] : Volt::AssetManager::Get().GetAssetRegistry())
			{
				const std::filesystem::path completePath = Volt::AssetManager::GetContextPath(metadata.filePath) / metadata.filePath;
				if (!FileSystem::Exists(completePath))
				{
					pathsToClear.emplace_back(metadata.filePath);
				}
			}

			for (const auto& path : pathsToClear)
			{
				Volt::AssetManager::Get().RemoveAssetFromRegistry(path);
			}
			hasReds = false;
		}
		ImGui::PopStyleColor(1);
	}

	// Search bar
	{
		bool t;
		EditorUtils::SearchBar(mySearchQuery, t, myActivateSearch);
		if (myActivateSearch)
		{
			myActivateSearch = false;
		}
	}

	if (ImGui::BeginTable("RegistryTable", 2, ImGuiTableFlags_BordersInnerV | ImGuiTableFlags_BordersInnerH | ImGuiTableFlags_SizingStretchProp | ImGuiTableFlags_Resizable | ImGuiTableFlags_RowBg | ImGuiTableFlags_ScrollY))
	{
		ImGui::TableSetupColumn("Path", ImGuiTableColumnFlags_WidthStretch);
		ImGui::TableSetupColumn("Handle", ImGuiTableColumnFlags_WidthStretch, 0.4f);
		ImGui::TableSetupScrollFreeze(ImGui::TableGetColumnCount(), 1);
		ImGui::TableHeadersRow();

		for (const auto& [handle, metadata] : Volt::AssetManager::Get().GetAssetRegistry())
		{
			if (!mySearchQuery.empty())
			{
				auto path_lower = Utility::ToLower(metadata.filePath.string());
				auto handle_lower = Utility::ToLower(std::to_string(handle));
				auto search_lower = Utility::ToLower(mySearchQuery);

				if (!Utility::StringContains(path_lower, search_lower) && !Utility::StringContains(handle_lower, search_lower))
				{
					continue;
				}
			}

			ImGui::TableNextColumn();
			const std::filesystem::path completePath = Volt::AssetManager::GetContextPath(metadata.filePath) / metadata.filePath;

			if (!FileSystem::Exists(completePath))
			{
				ImGui::TableSetBgColor(ImGuiTableBgTarget_RowBg0, ImColor(1.f, 0.f, 0.f, 1.f));
				hasReds = true;
			}
			ImGui::PushItemWidth(ImGui::GetColumnWidth());
			std::string pathString = metadata.filePath.string();
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
				Volt::AssetManager::Get().RemoveAssetFromRegistry(handle);
				break;
			}
		}

		ImGui::EndTable();
	}
}

void AssetRegistryPanel::AddNewModal()
{
	if (UI::BeginModal("Add New Handle##AssetRegistryPanel"))
	{
		static std::filesystem::path assetPath = "";
		static Volt::AssetHandle assetHandle = 0;

		UI::Property("Asset", assetPath);
		ImGui::InputScalar("Handle", ImGuiDataType_U64, &assetHandle);

		if (ImGui::Button("Add"))
		{
			if (assetHandle && !assetPath.empty() && !Volt::AssetManager::Get().ExistsInRegistry(assetPath) && !Volt::AssetManager::Get().ExistsInRegistry(assetHandle))
			{
				UI::Notify(NotificationType::Success, "Assethandle registered!", "Successfully added assethandle.");
				//Volt::AssetManager::Get().AddAssetToRegistry(assetPath, assetHandle);
			}
			else
			{
				UI::Notify(NotificationType::Error, "Failed registration!", "Asset already exists");
			}
			assetPath.clear();
			assetHandle = 0;
			ImGui::CloseCurrentPopup();
		}

		if (ImGui::Button("Cancel"))
		{
			assetPath.clear();
			assetHandle = 0;
			ImGui::CloseCurrentPopup();
		}

		UI::EndModal();
	}
}
