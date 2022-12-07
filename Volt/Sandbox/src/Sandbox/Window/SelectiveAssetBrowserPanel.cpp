#include "sbpch.h"
#include "SelectiveAssetBrowserPanel.h"

#include "Sandbox/Window/EditorIconLibrary.h"

#include <Volt/Utility/FileSystem.h>
#include <Volt/Asset/AssetManager.h>
#include <Volt/Utility/UIUtility.h>

static uint32_t s_assetBrowserCount;

SelectiveAssetBrowserPanel::SelectiveAssetBrowserPanel(Volt::AssetType assetType, const std::string& id)
	: EditorWindow("Asset Browser##" + id), mySelectiveAssetType(assetType)
{
	myIsOpen = true;
	myWindowFlags = ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse;
	UpdateAssetList();
}

void SelectiveAssetBrowserPanel::UpdateMainContent()
{
	RenderControlsBar();

	ImGui::BeginChild("##view", ImVec2(ImGui::GetContentRegionAvail().x, ImGui::GetWindowHeight()));
	{
		ImGui::BeginChild("Scrolling");
		{
			const float cellSize = myThumbnailSize + myThumbnailPadding;
			const float panelWidth = ImGui::GetContentRegionAvail().x;
			int32_t columnCount = (int32_t)(panelWidth / cellSize);

			if (columnCount < 1)
			{
				columnCount = 1;
			}

			ImGui::Columns(columnCount, nullptr, false);
			ImGui::PushStyleColor(ImGuiCol_Button, ImVec4{ 0.f, 0.f, 0.f, 0.f });

			for (const auto& asset : myAllAssetsOfType)
			{
				ImGui::PushID(asset.path.filename().string().c_str());

				Ref<Volt::Texture2D> icon = EditorIconLibrary::GetIcon(EditorIcon::GenericFile);

				UI::ImageButton(asset.path.filename().string(), UI::GetTextureID(icon), { myThumbnailSize, myThumbnailSize });
				if (ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left) && ImGui::IsItemHovered())
				{
					if (myOpenFileCallback)
					{
						myOpenFileCallback(asset.handle);
					}
				}

				if (ImGui::BeginDragDropSource())
				{
					ImGui::SetDragDropPayload("ASSET_BROWSER_ITEM", &asset.handle, sizeof(Volt::AssetHandle), ImGuiCond_Once);
					ImGui::EndDragDropSource();
				}

				ImGui::TextWrapped(asset.path.filename().string().c_str());
				ImGui::NextColumn();

				ImGui::PopID();
			}

			ImGui::PopStyleColor();
		}
		ImGui::EndChild();
	}
	ImGui::EndChild();
}

void SelectiveAssetBrowserPanel::UpdateAssetList()
{
	myAllAssetsOfType.clear();

	for (auto it : std::filesystem::recursive_directory_iterator(FileSystem::GetEnginePath()))
	{
		if (!it.is_directory())
		{
			const std::filesystem::path path = it.path();
			const Volt::AssetType type = Volt::AssetManager::Get().GetAssetTypeFromPath(path);

			if (type == mySelectiveAssetType)
			{
				auto& data = myAllAssetsOfType.emplace_back();
				data.path = path;
				data.handle = Volt::AssetManager::Get().GetAssetHandleFromPath(path);
				data.type = Volt::AssetManager::Get().GetAssetTypeFromPath(path);
			}
		}
	}
}

void SelectiveAssetBrowserPanel::RenderControlsBar()
{
	ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 0.f);

	UI::ScopedColor childColor{ ImGuiCol_ChildBg, { 0.2f, 0.2f, 0.2f, 1.f } };
	ImGui::BeginChild("##controlsBar", { 0.f, myControlsBarHeight });
	{
		const float buttonSizeOffset = 10.f;

		UI::ShiftCursor(5.f, 2.f);
		{
			UI::ScopedColor buttonBackground(ImGuiCol_Button, { 0.f, 0.f, 0.f, 0.f });
			ImGui::Image(UI::GetTextureID(EditorIconLibrary::GetIcon(EditorIcon::Search)), { myControlsBarHeight - buttonSizeOffset, myControlsBarHeight - buttonSizeOffset });

			ImGui::SameLine();
			ImGui::PushItemWidth(200.f);

			std::string REMOVE_TEXT;
			if (UI::InputText("", REMOVE_TEXT, ImGuiInputTextFlags_EnterReturnsTrue))
			{
			}

			ImGui::PopItemWidth();
			ImGui::SameLine();

			if (UI::ImageButton("##reloadButton", UI::GetTextureID(EditorIconLibrary::GetIcon(EditorIcon::Reload)), { myControlsBarHeight - buttonSizeOffset, myControlsBarHeight - buttonSizeOffset }))
			{
				UpdateAssetList();
			}
		}
	}
	ImGui::EndChild();

	ImGui::PopStyleVar();
}
