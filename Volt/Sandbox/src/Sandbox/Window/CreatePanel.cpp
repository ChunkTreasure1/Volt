#include "sbpch.h"
#include "CreatePanel.h"

#include "Sandbox/Window/EditorIconLibrary.h"

#include <Volt/Utility/UIUtility.h>
#include <Volt/Components/Components.h>

CreatePanel::CreatePanel(std::vector<Wire::EntityId>& selectedEntites, Ref<Volt::Scene>& scene)
	: EditorWindow("Create"), mySelectedEntites(selectedEntites), myScene(scene)
{
	myWindowFlags = ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse;
	myIsOpen = false;

	myDirectories[FileSystem::GetAssetsPath().string()] = ProcessDirectory(FileSystem::GetAssetsPath().string(), nullptr);
	myCurrentDirectory = myDirectories[FileSystem::GetAssetsPath().string()].get();
}

void CreatePanel::UpdateMainContent()
{
	UI::ScopedColor buttonColor(ImGuiCol_Button, { 0.313f, 0.313f, 0.313f, 1.f });
	UI::ScopedStyleFloat buttonRounding(ImGuiStyleVar_FrameRounding, 2.f);

	if (!myMeshListOpen)
	{
		const ImGuiTableFlags tableFlags = ImGuiTableFlags_BordersInnerV | ImGuiTableFlags_SizingFixedFit;
		if (ImGui::BeginTable("createButtons", 2, tableFlags))
		{
			ImGui::TableSetupColumn("Column1", 0, ImGui::GetContentRegionAvail().x / 2.f);
			ImGui::TableSetupColumn("Column2", 0, ImGui::GetContentRegionAvail().x / 2.f);

			ImGui::TableNextRow();
			ImGui::TableNextColumn();

			if (ImGui::Button("Entity", { ImGui::GetContentRegionAvail().x, myButtonSize }))
			{
				Volt::Entity newEntity = myScene->CreateEntity();

				mySelectedEntites.clear();
				mySelectedEntites.emplace_back(newEntity.GetId());
			}

			ImGui::TableNextColumn();

			if (ImGui::Button("Mesh", { ImGui::GetContentRegionAvail().x, myButtonSize }))
			{
				myMeshListOpen = true;
			}

			ImGui::EndTable();
		}
	}
	else
	{

		if (UI::ImageButton("##backButton", UI::GetTextureID(EditorIconLibrary::GetIcon(EditorIcon::Back)), { myButtonSize, myButtonSize }))
		{
			myMeshListOpen = false;
		}

		ImGui::SameLine();

		if (UI::ImageButton("##reloadButton", UI::GetTextureID(EditorIconLibrary::GetIcon(EditorIcon::Reload)), { myButtonSize, myButtonSize }))
		{

		}

		ImGui::SameLine();
		UI::ShiftCursor(0.f, 2.f);
		ImGui::Image(UI::GetTextureID(EditorIconLibrary::GetIcon(EditorIcon::Search)), { myButtonSize, myButtonSize }, { 0.f, 0.f }, { 1.f, 1.f });

		ImGui::SameLine();
		UI::ShiftCursor(0.f, -2.f);
		ImGui::PushItemWidth(ImGui::GetContentRegionAvail().x);

		UI::PushId();
		if (UI::InputText("", mySearchQuery, ImGuiInputTextFlags_EnterReturnsTrue))
		{
			if (mySearchQuery.empty())
			{
				myHasSearchQuery = false;
			}
			else
			{
				myHasSearchQuery = true;
			}
		}
		UI::PopId();
		ImGui::PopItemWidth();

		UI::ScopedColor childColor(ImGuiCol_ChildBg, { 0.18f, 0.18f, 0.18f, 1.f });

		ImGui::BeginChild("meshes");
		{
			UI::ShiftCursor(5.f, 5.f);

			if (myHasSearchQuery)
			{
			}
			else
			{
				UI::ScopedStyleFloat2 spacing(ImGuiStyleVar_ItemSpacing, { 0.f, 0.f });

				uint32_t i = 0;
				RenderDirectory(myDirectories[FileSystem::GetAssetsPath().string()], i);
			}
		}
		ImGui::EndChild();
	}
}

std::shared_ptr<DirectoryData> CreatePanel::ProcessDirectory(const std::filesystem::path& path, std::shared_ptr<DirectoryData> parent)
{
	std::shared_ptr<DirectoryData> dirData = std::make_shared<DirectoryData>();
	dirData->path = path;
	dirData->parentDir = parent.get();

	for (const auto& entry : std::filesystem::directory_iterator(path))
	{
		if (!entry.is_directory())
		{
			AssetData data;
			data.path = entry;

			if (Volt::AssetManager::Get().GetAssetTypeFromPath(data.path) == Volt::AssetType::MeshSource)
			{
				dirData->assets.push_back(data);
			}
		}
		else
		{
			dirData->subDirectories.emplace_back(ProcessDirectory(entry, dirData));
		}
	}

	for (int32_t i = (int32_t)dirData->subDirectories.size() - 1; i >= 0; --i)
	{
		if (dirData->subDirectories[i]->assets.empty() && dirData->subDirectories[i]->subDirectories.empty())
		{
			dirData->subDirectories.erase(dirData->subDirectories.begin() + i);
		}
	}

	//Sort directories and assets by name
	std::sort(dirData->subDirectories.begin(), dirData->subDirectories.end(), [](const std::shared_ptr<DirectoryData> dataOne, const std::shared_ptr<DirectoryData> dataTwo)
		{
			return dataOne->path.stem().string() < dataTwo->path.stem().string();
		});

	std::sort(dirData->assets.begin(), dirData->assets.end(), [](const AssetData& dataOne, const AssetData& dataTwo)
		{
			return dataOne.path.stem().string() < dataTwo.path.stem().string();
		});

	return dirData;
}

void CreatePanel::RenderDirectory(const std::shared_ptr<DirectoryData> dirData, uint32_t& i)
{
	std::string id = dirData->path.stem().string() + "##" + std::to_string(i++);

	ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_None;
	if (i == 0)
	{
		flags = (dirData->selected ? ImGuiTreeNodeFlags_Selected : ImGuiTreeNodeFlags_None) | ImGuiTreeNodeFlags_DefaultOpen;
	}
	else
	{
		flags = (dirData->selected ? ImGuiTreeNodeFlags_Selected : ImGuiTreeNodeFlags_None);
	}

	bool open = UI::TreeNodeImage(EditorIconLibrary::GetIcon(EditorIcon::Directory), id.c_str(), flags);

	if (open)
	{
		for (const auto& subDir : dirData->subDirectories)
		{
			RenderDirectory(subDir, i);
		}

		uint32_t j = 0;
		for (const auto& asset : dirData->assets)
		{
			std::string assetId = asset.path.stem().string() + "##" + std::to_string(j);

			UI::ImageSelectable(EditorIconLibrary::GetIcon(EditorIcon::GenericFile), assetId);

			if (ImGui::BeginDragDropSource())
			{
				const wchar_t* itemPath = asset.path.c_str();
				ImGui::SetDragDropPayload("CONTENT_BROWSER_ITEM", itemPath, (wcslen(itemPath) + 1) * sizeof(wchar_t), ImGuiCond_Once);
				ImGui::EndDragDropSource();
			}
			j++;
		}

		ImGui::TreePop();
	}
}
