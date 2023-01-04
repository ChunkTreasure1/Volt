#include "sbpch.h"
#include "AssetBrowserPanel.h"

#include "Sandbox/Utility/EditorResources.h"
#include "Sandbox/Utility/EditorLibrary.h"

#include "Sandbox/Utility/EditorUtilities.h"
#include "Sandbox/Utility/GlobalEditorStates.h"
#include "Sandbox/Utility/AssetBrowserUtilities.h"

#include "Sandbox/Window/AssetBrowser/AssetItem.h"
#include "Sandbox/Window/AssetBrowser/DirectoryItem.h"
#include "Sandbox/Window/AssetBrowser/AssetBrowserSelectionManager.h"

#include <Volt/Asset/AssetManager.h>
#include <Volt/Asset/Prefab.h>
#include <Volt/Asset/Mesh/MeshCompiler.h>
#include <Volt/Asset/Animation/Skeleton.h>
#include <Volt/Asset/Animation/Animation.h>
#include <Volt/Asset/Animation/AnimatedCharacter.h>
#include <Volt/Asset/Importers/MeshTypeImporter.h>
#include <Volt/Asset/Mesh/Material.h>
#include <Volt/Asset/Mesh/SubMaterial.h>
#include <Volt/Asset/ParticlePreset.h>

#include <Volt/Components/Components.h>
#include <Volt/Scene/Scene.h>
#include <Volt/Project/ProjectManager.h>

#include <Volt/Rendering/Shader/Shader.h>
#include <Volt/Rendering/Shader/ShaderRegistry.h>
#include <Volt/Rendering/Texture/Texture2D.h>

#include <Volt/Utility/FileSystem.h>
#include <Volt/Utility/UIUtility.h>
#include <Volt/Utility/YAMLSerializationHelpers.h>
#include <Volt/Utility/SerializationMacros.h>

#include <Volt/Input/Input.h>
#include <Volt/Input/KeyCodes.h>

#include <yaml-cpp/yaml.h>

namespace Utility
{
	static ImVec4 GetColorFromType(Volt::AssetType type)
	{
		switch (type)
		{
			case Volt::AssetType::Mesh: return { 0.73f, 0.9f, 0.26f, 1.f };
			case Volt::AssetType::MeshSource: return { 0.43f, 0.9f, 0.26f, 1.f };
			case Volt::AssetType::Animation: return { 0.65f, 0.18f, 0.69f, 1.f };
			case Volt::AssetType::Skeleton: return { 1.f, 0.49f, 0.8f, 1.f };
			case Volt::AssetType::Texture: return { 0.9f, 0.26f, 0.27f, 1.f };
			case Volt::AssetType::Material: return { 0.26f, 0.35f, 0.9f, 1.f };
			case Volt::AssetType::Shader: return { 0.26f, 0.6f, 0.9f, 1.f };
			case Volt::AssetType::ShaderSource: return { 0.26f, 0.72f, 0.9f, 1.f };
			case Volt::AssetType::Scene: return { 0.9f, 0.54f, 0.26f, 1.f };
			case Volt::AssetType::AnimatedCharacter: return { 0.9f, 0.25f, 0.49f, 1.f };
			case Volt::AssetType::Prefab: return { 0.25f, 0.93f, 0.92f, 1.f };
			case Volt::AssetType::ParticlePreset: return { 1.f, 0.62f, 0.f, 1.f };
			default: return { 0.f, 0.f, 0.f, 0.f };
		}

		return { 0.f, 0.f, 0.f, 0.f };
	}
}

AssetBrowserPanel::AssetBrowserPanel(Ref<Volt::Scene>& aScene, const std::string& id)
	: EditorWindow("Asset Browser" + id), myEditorScene(aScene)
{
	myWindowFlags = ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse;
	myIsOpen = true;

	SetMinWindowSize({ 700.f, 300.f });

	mySelectionManager = CreateRef<AssetBrowser::SelectionManager>();

	myDirectories[Volt::ProjectManager::GetAssetsPath()] = ProcessDirectory(Volt::ProjectManager::GetAssetsPath(), nullptr);
	myDirectories[FileSystem::GetEnginePath()] = ProcessDirectory(FileSystem::GetEnginePath(), nullptr);

	myEngineDirectory = myDirectories[FileSystem::GetEnginePath()].get();
	myAssetsDirectory = myDirectories[Volt::ProjectManager::GetAssetsPath()].get();

	myCurrentDirectory = myAssetsDirectory;
	GenerateAssetPreviewsInCurrentDirectory();

	myDirectoryButtons.emplace_back(myCurrentDirectory);
}

void AssetBrowserPanel::UpdateMainContent()
{
	float cellSize = myThumbnailSize + myThumbnailPadding;

	if (myNextDirectory)
	{
		mySelectionManager->DeselectAll();
		myCurrentDirectory = myNextDirectory;
		myNextDirectory = nullptr;

		myDirectoryButtons.clear();
		myDirectoryButtons = FindParentDirectoriesOfDirectory(myCurrentDirectory);
		GenerateAssetPreviewsInCurrentDirectory();
	}

	auto windowSize = ImGui::GetWindowSize();

	if (windowSize.y < 50.f)
	{
		return;
	}

	const float controlsBarHeight = 30.f;

	// Controls bar
	{
		UI::ScopedStyleFloat rounding(ImGuiStyleVar_FrameRounding, 2.f);

		ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 0.f);
		RenderControlsBar(controlsBarHeight);
		ImGui::PopStyleVar();
	}

	const ImGuiTableFlags tableFlags = ImGuiTableFlags_BordersInnerV | ImGuiTableFlags_SizingFixedFit | ImGuiTableFlags_Resizable;
	if (ImGui::BeginTable("assetBrowserMain", 2, tableFlags))
	{
		ImGui::TableSetupColumn("Outline", 0, 250.f);
		ImGui::TableSetupColumn("View", ImGuiTableColumnFlags_WidthStretch);

		ImGui::TableNextRow();
		ImGui::TableNextColumn();

		bool reload = false;

		//Draw outline
		{
			ImGuiStyle& style = ImGui::GetStyle();
			auto color = style.Colors[ImGuiCol_FrameBg];

			UI::ScopedColor newColor(ImGuiCol_ChildBg, { color.x, color.y, color.z, color.w });
			UI::ScopedStyleFloat rounding(ImGuiStyleVar_ChildRounding, 2.f);

			if (ImGui::BeginChild("##outline"))
			{
				UI::ShiftCursor(5.f, 5.f);
				const auto flags = ImGuiTreeNodeFlags_Selected | ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_DefaultOpen;

				bool open = UI::TreeNodeImage(EditorResources::GetEditorIcon(EditorIcon::Directory), "Assets", flags);

				if (ImGui::IsItemClicked())
				{
					mySelectionManager->DeselectAll();
					myNextDirectory = myAssetsDirectory;
				}

				if (open)
				{
					UI::ScopedStyleFloat2 spacing(ImGuiStyleVar_ItemSpacing, { 0.f, 0.f });

					for (const auto& subDir : myAssetsDirectory->subDirectories)
					{
						reload |= RenderDirectory(subDir);
						if (reload)
						{
							break;
						}
					}
					UI::TreeNodePop();
				}

				ImGui::EndChild();
			}
		}

		if (reload)
		{
			Reload();
			ImGui::EndTable();
			return;
		}

		ImGui::TableNextColumn();

		if (ImGui::BeginChild("##view", ImVec2(ImGui::GetContentRegionAvail().x, ImGui::GetWindowHeight() - controlsBarHeight)))
		{
			const auto viewportMinRegion = ImGui::GetWindowContentRegionMin();
			const auto viewportMaxRegion = ImGui::GetWindowContentRegionMax();
			const auto viewportOffset = ImGui::GetWindowPos();

			myViewBounds[0] = { viewportMinRegion.x + viewportOffset.x, viewportMinRegion.y + viewportOffset.y };
			myViewBounds[1] = { viewportMaxRegion.x + viewportOffset.x, viewportMaxRegion.y + viewportOffset.y };

			ImGui::BeginChild("Scrolling");
			{
				static float padding = 16.f;

				float cellSize = myThumbnailSize + padding;
				float panelWidth = ImGui::GetContentRegionAvail().x;
				auto columnCount = (int)(panelWidth / cellSize);

				if (columnCount < 1)
				{
					columnCount = 1;
				}

				ImGui::Columns(columnCount, nullptr, false);
				ImGui::PushStyleColor(ImGuiCol_Button, ImVec4{ 0.f, 0.f, 0.f, 0.f });

				if (!myHasSearchQuery)
				{
					RenderView(myCurrentDirectory->subDirectories, myCurrentDirectory->assets);
				}
				else
				{
					RenderView(mySearchDirectories, mySearchAssets);
				}

				RenderWindowRightClickPopup();

				ImGui::PopStyleColor();
			}
			ImGui::EndChild();

			if (void* ptr = UI::DragDropTarget("scene_entity_hierarchy"))
			{
				Wire::EntityId entity = *(Wire::EntityId*)ptr;
				if (entity != Wire::NullID)
				{
					CreatePrefabAndSetupEntities(entity);
				}
			}
			ImGui::EndChild();
		}

		ImGui::EndTable();
	}

	if (!myDragDroppedMeshes.empty() && !myIsImporting)
	{
		const auto path = Volt::AssetManager::Get().GetRelativePath(myDragDroppedMeshes.back());
		myDragDroppedMeshes.pop_back();

		AssetData assetData;
		assetData.handle = Volt::AssetManager::Get().AddToRegistry(path);
		assetData.path = path;
		assetData.type = Volt::AssetType::MeshSource;

		myMeshImportData = {};
		myMeshToImport = assetData;
		myMeshImportData.destination = myMeshToImport.path.parent_path().string() + "\\" + myMeshToImport.path.stem().string() + ".vtmesh";

		UI::OpenModal("Import Mesh##assetBrowser");
		myIsImporting = true;
	}

	if (myShouldDeleteSelected)
	{
		UI::OpenModal("Delete Selected Files?");
		myShouldDeleteSelected = false;
	}

	ImportState importState = EditorUtils::MeshImportModal("Import Mesh##assetBrowser", myMeshImportData, myMeshToImport.path);
	if (importState == ImportState::Imported)
	{
		Reload();
		myIsImporting = false;
	}
	else if (importState == ImportState::Discard)
	{
		myIsImporting = false;
	}

	if (EditorUtils::NewCharacterModal("New Character##assetBrowser", myNewAnimatedCharacter, myNewCharacterData))
	{
		myNewAnimatedCharacter = nullptr;

		Reload();
	}

	CreateNewShaderModal();
	DeleteFilesModal();
}

void AssetBrowserPanel::OnEvent(Volt::Event& e)
{
	Volt::EventDispatcher dispatcher(e);
	dispatcher.Dispatch<Volt::WindowDragDropEvent>(VT_BIND_EVENT_FN(AssetBrowserPanel::OnDragDropEvent));
	dispatcher.Dispatch<Volt::KeyPressedEvent>(VT_BIND_EVENT_FN(AssetBrowserPanel::OnKeyPressedEvent));
	dispatcher.Dispatch<Volt::AppRenderEvent>(VT_BIND_EVENT_FN(AssetBrowserPanel::OnRenderEvent));
}

bool AssetBrowserPanel::OnDragDropEvent(Volt::WindowDragDropEvent& e)
{
	auto [x, y] = Volt::Input::GetMousePosition();
	const auto [wX, wY] = Volt::Application::Get().GetWindow().GetPosition();

	x += wX;
	y += wY;

	VT_CORE_INFO("X: {0}, Y: {1}", x, y);

	if (x > myViewBounds[0].x && y > myViewBounds[0].y && x < myViewBounds[1].x && y < myViewBounds[1].y)
	{
		for (const auto& path : e.GetPaths())
		{
			if (!std::filesystem::is_directory(path))
			{
				const std::string originalName = path.stem().string();
				std::string tempName = originalName;

				uint32_t i = 0;
				while (FileSystem::Exists(myCurrentDirectory->path / (tempName + path.extension().string())))
				{
					tempName = originalName + " (" + std::to_string(i) + ")";
					i++;
				}

				const std::filesystem::path targetPath = myCurrentDirectory->path / (tempName + path.extension().string());
				FileSystem::Copy(path, targetPath);

				const Volt::AssetType type = Volt::AssetManager::Get().GetAssetTypeFromPath(targetPath);
				if (type == Volt::AssetType::MeshSource)
				{
					myDragDroppedMeshes.emplace_back(targetPath);
				}
			}
		}

		Reload();
	}

	return false;
}

bool AssetBrowserPanel::OnKeyPressedEvent(Volt::KeyPressedEvent& e)
{
	switch (e.GetKeyCode())
	{
		case VT_KEY_DELETE:
		{
			if (myIsFocused && mySelectionManager->IsAnySelected())
			{
				myShouldDeleteSelected = true;
			}

			break;
		}
	}

	return false;
}

bool AssetBrowserPanel::OnMouseReleasedEvent(Volt::MouseButtonReleasedEvent& e)
{
	if (e.GetMouseButton() == VT_MOUSE_BUTTON_LEFT)
	{
		if (ImGui::IsWindowHovered() && GlobalEditorStates::isDragging)
		{
			GlobalEditorStates::isDragging = false;
			GlobalEditorStates::dragStartedInAssetBrowser = false;
			GlobalEditorStates::dragAsset = Volt::Asset::Null();
		}
	}

	return false;
}

bool AssetBrowserPanel::OnRenderEvent(Volt::AppRenderEvent& e)
{
	constexpr uint32_t maxPreviewsPerFrame = 1;

	if (!myPreviewsToUpdate.empty())
	{
		for (uint32_t i = 0; i < maxPreviewsPerFrame; i++)
		{
			Ref<AssetPreview> preview = myPreviewsToUpdate.back();
			preview->Render();

			myPreviewsToUpdate.pop_back();
		}
	}

	return false;
}

Ref<AssetBrowser::DirectoryItem> AssetBrowserPanel::ProcessDirectory(const std::filesystem::path& path, AssetBrowser::DirectoryItem* parent)
{
	Ref<AssetBrowser::DirectoryItem> dirData = CreateRef<AssetBrowser::DirectoryItem>(mySelectionManager.get(), path, myThumbnailSize);
	dirData->parentDirectory = parent;

	for (const auto& entry : std::filesystem::directory_iterator(path))
	{
		if (!entry.is_directory())
		{
			auto type = Volt::AssetManager::Get().GetAssetTypeFromPath(entry);

			if (type != Volt::AssetType::None && !entry.path().filename().string().contains(".vtthumb.png"))
			{
				if (myAssetMask == Volt::AssetType::None || (myAssetMask & type) != Volt::AssetType::None)
				{
					Ref<AssetBrowser::AssetItem> assetItem = CreateRef<AssetBrowser::AssetItem>(mySelectionManager.get(), Volt::AssetManager::Get().GetRelativePath(entry.path()), myThumbnailSize, myMeshImportData, myMeshToImport);
					dirData->assets.emplace_back(assetItem);
				}
			}
		}
		else
		{
			auto nextDirData = ProcessDirectory(entry.path(), dirData.get());
			if ((!nextDirData->assets.empty() || !nextDirData->subDirectories.empty()) || myAssetMask == Volt::AssetType::None)
			{
				dirData->subDirectories.emplace_back(nextDirData);
			}
		}
	}

	std::sort(dirData->subDirectories.begin(), dirData->subDirectories.end(), [](const Ref<AssetBrowser::DirectoryItem>& a, const Ref<AssetBrowser::DirectoryItem>& b) { return a->path.string() < b->path.string(); });
	std::sort(dirData->assets.begin(), dirData->assets.end(), [](const Ref<AssetBrowser::AssetItem>& a, const Ref<AssetBrowser::AssetItem>& b) { return a->path.stem().string() < b->path.stem().string(); });

	return dirData;
}

std::vector<AssetBrowser::DirectoryItem*> AssetBrowserPanel::FindParentDirectoriesOfDirectory(AssetBrowser::DirectoryItem* directory)
{
	std::vector<AssetBrowser::DirectoryItem*> directories;
	directories.emplace_back(directory);

	for (auto dir = directory->parentDirectory; dir != nullptr; dir = dir->parentDirectory)
	{
		directories.emplace_back(dir);
	}

	std::reverse(directories.begin(), directories.end());
	return directories;
}

void AssetBrowserPanel::RenderControlsBar(float height)
{
	UI::ScopedColor childColor{ ImGuiCol_ChildBg, { 0.2f, 0.2f, 0.2f, 1.f } };

	ImGui::BeginChild("##controlsBar", { 0.f, height });
	{
		const float buttonSizeOffset = 10.f;
		int32_t offsetToRemove = 0;
		bool shouldRemove = false;

		UI::ShiftCursor(5.f, 4.f);
		{
			UI::ScopedColor buttonBackground(ImGuiCol_Button, { 0.f, 0.f, 0.f, 0.f });
			ImGui::Image(UI::GetTextureID(EditorResources::GetEditorIcon(EditorIcon::Search)), { height - buttonSizeOffset, height - buttonSizeOffset });

			ImGui::SameLine();
			UI::ShiftCursor(0.f, -0.5f);
			ImGui::PushItemWidth(200.f);

			if (UI::InputText("", mySearchQuery))
			{
				if (!mySearchQuery.empty())
				{
					myHasSearchQuery = true;
					Search(mySearchQuery);
				}
				else
				{
					myHasSearchQuery = false;
				}
			}

			ImGui::PopItemWidth();
			ImGui::SameLine();
			UI::ShiftCursor(0.f, -1.f);
			{
				UI::ScopedColor buttonBackground(ImGuiCol_Button, { 0.f, 0.f, 0.f, 0.f });

				if (UI::ImageButton("##reloadButton", UI::GetTextureID(EditorResources::GetEditorIcon(EditorIcon::Reload)), { height - buttonSizeOffset, height - buttonSizeOffset }))
				{
					Reload();
				}

				ImGui::SameLine();

				if (UI::ImageButton("##backButton", UI::GetTextureID(EditorResources::GetEditorIcon(EditorIcon::Back)), { height - buttonSizeOffset, height - buttonSizeOffset }))
				{
					myHasSearchQuery = false;
					mySearchQuery.clear();

					if (myCurrentDirectory->path != Volt::ProjectManager::GetAssetsPath() && myCurrentDirectory->path != FileSystem::GetEnginePath())
					{
						myNextDirectory = myCurrentDirectory->parentDirectory;

						offsetToRemove = (uint32_t)(myDirectoryButtons.size() - 1);
						shouldRemove = true;
					}
				}
			}

			for (size_t i = 0; i < myDirectoryButtons.size(); i++)
			{
				ImGui::SameLine();
				std::string dirName = myDirectoryButtons[i]->path.stem().string();

				const float buttonWidth = ImGui::CalcTextSize(dirName.c_str()).x + 5.f;
				UI::ScopedColor bgColor(ImGuiCol_Button, { 0.5f, 0.5f, 0.5f, 1.f });

				if (ImGui::BeginChild(myDirectoryButtons[i]->path.string().c_str(), { buttonWidth, height - 4.f }))
				{
					const bool hovered = ImGui::IsWindowHovered();
					if (hovered)
					{
						ImGui::PushStyleColor(ImGuiCol_Text, { 0.4f, 0.67f, 1.000f, 1.000f });
					}

					UI::ShiftCursor(0.f, 4.f);
					ImGui::TextUnformatted(dirName.c_str());

					if (void* ptr = UI::DragDropTarget({ "ASSET_BROWSER_ITEM" }))
					{
						Volt::AssetHandle handle = *(Volt::AssetHandle*)ptr;
						Volt::AssetManager::Get().MoveAsset(handle, myDirectoryButtons.at(i)->path);
						Reload();

						ImGui::EndChild();

						if (hovered)
						{
							ImGui::PopStyleColor();
						}

						break;
					}

					if (hovered)
					{
						ImGui::PopStyleColor();
					}

					if (ImGui::IsMouseClicked(ImGuiMouseButton_Left) && ImGui::IsWindowHovered())
					{
						myNextDirectory = myDirectoryButtons[i];

						offsetToRemove = (int32_t)(i + 1);
						shouldRemove = true;

						myHasSearchQuery = false;
						mySearchQuery.clear();
					}

					ImGui::EndChild();
				}

				ImGui::SameLine();

				if (myDirectoryButtons.size() - 1 > i)
				{
					ImGui::TextUnformatted("/");
				}
			}

			ImGui::SameLine();

			// Asset type
			{
				UI::ScopedColor frameBg{ ImGuiCol_FrameBgHovered, { 1.000f, 1.000f, 1.000f, 0.2f } };

				const char* items = "Game\0Engine";

				auto currentValue = (int32_t)myShowEngineAssets;

				UI::ShiftCursor(ImGui::GetContentRegionAvail().x - 2.f * height - 160.f - buttonSizeOffset, 0.f);

				ImGui::PushItemWidth(150.f);
				if (ImGui::Combo("##assetType", &currentValue, items))
				{
					myShowEngineAssets = (bool)currentValue;
					if (currentValue == 0)
					{
						myCurrentDirectory = myAssetsDirectory;
					}
					else
					{
						myCurrentDirectory = myEngineDirectory;
					}

				}
				ImGui::PopItemWidth();
			}

			ImGui::SameLine();

			// Filter button
			{
				{
					UI::ScopedColor buttonBackground(ImGuiCol_Button, { 0.f, 0.f, 0.f, 0.f });
					UI::ImageButton("##filter", UI::GetTextureID(EditorResources::GetEditorIcon(EditorIcon::Filter)), { height - buttonSizeOffset, height - buttonSizeOffset });
				}

				if (ImGui::BeginPopupContextItem("filterMenu", ImGuiPopupFlags_MouseButtonLeft))
				{
					UI::ScopedColor buttonBackground(ImGuiCol_Button, { 0.3f, 0.3f, 0.3f, 1.f });

					if (ImGui::Button("Clear##filterMenu"))
					{
						myAssetMask = Volt::AssetType::None;
						Reload();
					}

					for (const auto& asset : Volt::s_assetNamesMap)
					{
						bool selected = (myAssetMask & asset.second) != Volt::AssetType::None;
						if (ImGui::Checkbox(asset.first.c_str(), &selected))
						{
							if (selected)
							{
								myAssetMask = myAssetMask | asset.second;
							}
							else
							{
								myAssetMask = myAssetMask & ~asset.second;
							}

							Reload();
						}
					}
					ImGui::EndPopup();
				}
			}

			ImGui::SameLine();

			// Settings button
			{
				UI::ScopedColor buttonBackground(ImGuiCol_Button, { 0.f, 0.f, 0.f, 0.f });

				ImGui::ImageButton(UI::GetTextureID(EditorResources::GetEditorIcon(EditorIcon::Settings)), { height - buttonSizeOffset, height - buttonSizeOffset });
				if (ImGui::BeginPopupContextItem("settingsMenu", ImGuiPopupFlags_MouseButtonLeft))
				{
					ImGui::PushItemWidth(100.f);
					ImGui::SliderFloat("Icon size", &myThumbnailSize, 20.f, 200.f);
					ImGui::PopItemWidth();

					ImGui::EndPopup();
				}
			}


			if (shouldRemove)
			{
				for (int32_t i = (int32_t)myDirectoryButtons.size() - 1; i >= offsetToRemove; i--)
				{
					myDirectoryButtons.erase(myDirectoryButtons.begin() + i);
				}
			}
		}
	}
	ImGui::EndChild();
}

bool AssetBrowserPanel::RenderDirectory(const Ref<AssetBrowser::DirectoryItem> dirData)
{
	bool reload = false;

	const std::string id = dirData->path.stem().string() + "##" + dirData->path.string();
	const bool selected = mySelectionManager->IsSelected(dirData.get());

	const auto flags = (selected ? ImGuiTreeNodeFlags_Selected : ImGuiTreeNodeFlags_None) | ImGuiTreeNodeFlags_OpenOnArrow;

	const bool open = UI::TreeNodeImage(EditorResources::GetEditorIcon(EditorIcon::Directory), id, flags);
	if (ImGui::IsItemClicked() && !selected)
	{
		mySelectionManager->Select(dirData.get());
		myNextDirectory = dirData.get();
	}

	if (void* ptr = UI::DragDropTarget({ "ASSET_BROWSER_ITEM", "ASSET_BROWSER_FOLDER" }))
	{
		for (const auto& item : mySelectionManager->GetSelectedItems())
		{
			if (item->isDirectory && item != dirData.get())
			{
				const std::filesystem::path newPath = dirData->path / item->path.stem();
				Volt::AssetManager::Get().MoveFolder(item->path, newPath);
				FileSystem::MoveFolder(item->path, newPath);
			}
		}

		for (const auto& item : mySelectionManager->GetSelectedItems())
		{
			if (!item->isDirectory && item != dirData.get() && std::filesystem::exists(item->path))
			{
				// Check for thumbnail PNG
				if (Volt::AssetManager::Get().GetAssetTypeFromPath(item->path) == Volt::AssetType::Texture)
				{
					const std::filesystem::path thumbnailPath = item->path.parent_path() / (item->path.filename().string() + ".vtthumb.png");
					if (FileSystem::Exists(thumbnailPath))
					{
						FileSystem::Move(thumbnailPath, dirData->path);
					}
				}

				Volt::AssetManager::Get().MoveAsset(Volt::AssetManager::Get().GetAssetHandleFromPath(item->path), dirData->path);
			}
		}

		reload = true;
	}

	if (open)
	{
		for (const auto& subDir : dirData->subDirectories)
		{
			RenderDirectory(subDir);
		}

		for (const auto& asset : dirData->assets)
		{
			std::string assetId = asset->path.stem().string() + "##" + std::to_string(asset->handle);
			ImGui::Selectable(assetId.c_str());
		}

		ImGui::TreePop();
	}

	return reload;
}

void AssetBrowserPanel::RenderView(std::vector<Ref<AssetBrowser::DirectoryItem>>& directories, std::vector<Ref<AssetBrowser::AssetItem>>& assets)
{
	bool reload = false;

	for (const auto& dir : directories)
	{
		if (dir->Render())
		{
			reload = true;
			break;
		}

		if (dir->isNext)
		{
			myNextDirectory = dir.get();
			dir->isNext = false;
		}

		ImGui::NextColumn();
	}

	if (reload)
	{
		reload = false;
		Reload();
	}

	for (const auto& asset : assets)
	{
		if (asset->Render())
		{
			reload = true;
			break;
		}
		ImGui::NextColumn();
	}

	if (reload)
	{
		Reload();
	}

	if (ImGui::IsWindowHovered() && ImGui::IsMouseClicked(ImGuiMouseButton_Left))
	{
		mySelectionManager->DeselectAll();
	}

	UI::ShiftCursor(0.f, 170.f); // Extra space at the bottom
}

void AssetBrowserPanel::RenderWindowRightClickPopup()
{

	if (ImGui::BeginPopupContextWindow("CreateMenu", ImGuiPopupFlags_MouseButtonRight | ImGuiPopupFlags_NoOpenOverExistingPopup | ImGuiPopupFlags_NoOpenOverItems))
	{
		ImGui::SetCursorPosX(150.f);
		ImGui::SetCursorPosX(ImGui::GetStyle().WindowPadding.x);

		if (ImGui::BeginMenu("New"))
		{
			if (ImGui::MenuItem("New Folder"))
			{
				const std::string originalName = "New Folder";
				std::string tempName = originalName;

				uint32_t i = 0;
				while (FileSystem::Exists(myCurrentDirectory->path / tempName))
				{
					tempName = originalName + " (" + std::to_string(i) + ")";
					i++;
				}

				FileSystem::CreateFolder(myCurrentDirectory->path / tempName);
				Reload();

				auto dirIt = std::find_if(myCurrentDirectory->subDirectories.begin(), myCurrentDirectory->subDirectories.end(), [tempName](const Ref<AssetBrowser::DirectoryItem> data)
					{
						return data->path.stem().string() == tempName;
					});

				if (dirIt != myCurrentDirectory->subDirectories.end())
				{
					(*dirIt)->isRenaming = true;
					(*dirIt)->currentRenamingName = tempName;

					mySelectionManager->Select((*dirIt).get());
				}
			}

			ImGui::Separator();

			if (ImGui::MenuItem("New Material"))
			{
				CreateNewAssetInCurrentDirectory(Volt::AssetType::Material);
			}

			if (ImGui::MenuItem("New Animated Character"))
			{
				CreateNewAssetInCurrentDirectory(Volt::AssetType::AnimatedCharacter);
			}

			if (ImGui::MenuItem("New Shader"))
			{
				CreateNewAssetInCurrentDirectory(Volt::AssetType::Shader);
			}

			if (ImGui::MenuItem("New Physics Material"))
			{
				CreateNewAssetInCurrentDirectory(Volt::AssetType::PhysicsMaterial);
			}

			if (ImGui::MenuItem("New Scene"))
			{
				CreateNewAssetInCurrentDirectory(Volt::AssetType::Scene);
			}

			if (ImGui::MenuItem("New Particle Preset"))
			{
				CreateNewAssetInCurrentDirectory(Volt::AssetType::ParticlePreset);
			}

			ImGui::EndMenu();
		}

		ImGui::Separator();

		if (ImGui::MenuItem("Reload"))
		{
			Reload();
		}

		ImGui::EndPopup();
	}

}

void AssetBrowserPanel::DeleteFilesModal()
{
	UI::ScopedStyleFloat buttonRounding{ ImGuiStyleVar_FrameRounding, 2.f };

	if (UI::BeginModal("Delete Selected Files?", ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse | ImGuiWindowFlags_AlwaysAutoResize))
	{
		ImGui::TextUnformatted("Are you sure you want to delete the selected files?");

		ImGui::PushItemWidth(80.f);
		if (ImGui::Button("Yes"))
		{
			const auto& selectedItems = mySelectionManager->GetSelectedItems();

			for (const auto& item : selectedItems)
			{
				if (item->isDirectory)
				{
					Volt::AssetManager::Get().RemoveFolderFromRegistry(item->path);

					FileSystem::MoveToRecycleBin(item->path);
				}
			}

			for (const auto& item : selectedItems)
			{
				if (!item->isDirectory && Volt::AssetManager::Get().ExistsInRegistry(item->path))
				{
					Volt::AssetManager::Get().RemoveAsset(item->path);
				}
			}

			Reload();
			ImGui::CloseCurrentPopup();
		}

		ImGui::SameLine();

		if (ImGui::Button("No"))
		{
			ImGui::CloseCurrentPopup();
		}

		ImGui::PopItemWidth();

		UI::EndModal();
	}
}

void AssetBrowserPanel::Reload()
{
	const std::filesystem::path currentPath = myCurrentDirectory ? myCurrentDirectory->path : Volt::ProjectManager::GetAssetsPath();

	myCurrentDirectory = nullptr;
	myNextDirectory = nullptr;
	mySelectionManager->DeselectAll();

	myDirectories[Volt::ProjectManager::GetAssetsPath()] = ProcessDirectory(Volt::ProjectManager::GetAssetsPath(), nullptr);
	myDirectories[FileSystem::GetEnginePath()] = ProcessDirectory(FileSystem::GetEnginePath(), nullptr);

	myEngineDirectory = myDirectories[FileSystem::GetEnginePath()].get();
	myAssetsDirectory = myDirectories[Volt::ProjectManager::GetAssetsPath()].get();

	//Find directory
	myCurrentDirectory = FindDirectoryWithPath(currentPath);
	if (!myCurrentDirectory)
	{
		myCurrentDirectory = myAssetsDirectory;
	}

	//Setup new file path buttons
	myDirectoryButtons.clear();
	myDirectoryButtons = FindParentDirectoriesOfDirectory(myCurrentDirectory);

	GenerateAssetPreviewsInCurrentDirectory();
}

void AssetBrowserPanel::Search(const std::string& query)
{
	std::vector<std::string> queries;
	std::vector<std::string> types;

	std::string searchQuery = query;

	searchQuery.push_back(' ');

	for (auto next = searchQuery.find_first_of(' '); next != std::string::npos; next = searchQuery.find_first_of(' '))
	{
		std::string split = searchQuery.substr(0, next);
		searchQuery = searchQuery.substr(next + 1);

		if (split.front() == '*')
		{
			types.emplace_back(split.substr(1));
		}
		else
		{
			queries.emplace_back(split);
		}
	}

	//Find all folders and files containing queries
	mySearchDirectories.clear();
	mySearchAssets.clear();
	for (const auto& query : queries)
	{
		FindFoldersAndFilesWithQuery(myAssetsDirectory->subDirectories, mySearchDirectories, mySearchAssets, query);
	}

	for (const auto& type : types)
	{
		FindFoldersAndFilesWithQuery(myAssetsDirectory->subDirectories, mySearchDirectories, mySearchAssets, type);
	}
}

void AssetBrowserPanel::FindFoldersAndFilesWithQuery(const std::vector<Ref<AssetBrowser::DirectoryItem>>& dirList, std::vector<Ref<AssetBrowser::DirectoryItem>>& directories, std::vector<Ref<AssetBrowser::AssetItem>>& assets, const std::string& query)
{
	for (const auto& dir : dirList)
	{
		std::string dirStem = dir->path.stem().string();
		std::transform(dirStem.begin(), dirStem.end(), dirStem.begin(), [](unsigned char c)
			{
				return std::tolower(c);
			});

		if (dirStem.find(query) != std::string::npos)
		{
			directories.emplace_back(dir);
		}

		for (const auto& asset : dir->assets)
		{
			std::string assetFilename = asset->path.filename().string();
			std::transform(assetFilename.begin(), assetFilename.end(), assetFilename.begin(), [](unsigned char c)
				{
					return std::tolower(c);
				});

			if (assetFilename.find(query) != std::string::npos)
			{
				assets.emplace_back(asset);
			}
		}

		FindFoldersAndFilesWithQuery(dir->subDirectories, directories, assets, query);
	}
}

AssetBrowser::DirectoryItem* AssetBrowserPanel::FindDirectoryWithPath(const std::filesystem::path& path)
{
	std::vector<Ref<AssetBrowser::DirectoryItem>> dirList;
	for (const auto& dir : myDirectories)
	{
		dirList.emplace_back(dir.second);
	}

	return FindDirectoryWithPathRecursivly(dirList, path);
}

AssetBrowser::DirectoryItem* AssetBrowserPanel::FindDirectoryWithPathRecursivly(const std::vector<Ref<AssetBrowser::DirectoryItem>> dirList, const std::filesystem::path& path)
{
	for (const auto& dir : dirList)
	{
		if (dir->path == path)
		{
			return dir.get();
		}
	}

	for (const auto& dir : dirList)
	{
		if (auto it = FindDirectoryWithPathRecursivly(dir->subDirectories, path))
		{
			return it;
		}
	}

	return nullptr;
}

void AssetBrowserPanel::CreatePrefabAndSetupEntities(Wire::EntityId entity)
{
	const auto& tagComp = myEditorScene->GetRegistry().GetComponent<Volt::TagComponent>(entity);

	std::string name = tagComp.tag + ".vtprefab";
	name.erase(std::remove_if(name.begin(), name.end(), ::isspace), name.end());

	Ref<Volt::Prefab> prefab = Volt::AssetManager::CreateAsset<Volt::Prefab>(myCurrentDirectory->path, name, myEditorScene->GetRegistry(), entity);
	Volt::AssetManager::Get().SaveAsset(prefab);

	SetupEntityAsPrefab(entity, prefab->handle);
	Reload();
}

void AssetBrowserPanel::SetupEntityAsPrefab(Wire::EntityId entity, Volt::AssetHandle prefabId)
{
	if (!myEditorScene->GetRegistry().HasComponent<Volt::PrefabComponent>(entity))
	{
		myEditorScene->GetRegistry().AddComponent<Volt::PrefabComponent>(entity);
	}

	auto& prefabComp = myEditorScene->GetRegistry().GetComponent<Volt::PrefabComponent>(entity);
	prefabComp.prefabAsset = prefabId;
	prefabComp.prefabEntity = entity;

	auto& relComp = myEditorScene->GetRegistry().GetComponent<Volt::RelationshipComponent>(entity);
	for (const auto& child : relComp.Children)
	{
		SetupEntityAsPrefab(child, prefabId);
	}
}

void AssetBrowserPanel::RecursiveRemoveFolderContents(DirectoryData* aDir)
{
	for (const auto& asset : aDir->assets)
	{
		if (FileSystem::Exists(asset.path))
		{
			Volt::AssetManager::Get().RemoveAsset(asset.handle);
		}
	}

	aDir->assets.clear();

	for (const auto& dir : aDir->subDirectories)
	{
		if (FileSystem::Exists(dir->path))
		{
			RecursiveRemoveFolderContents(dir.get());
			FileSystem::Remove(dir->path);
		}
	}

	aDir->subDirectories.clear();
}

void AssetBrowserPanel::RecursiceRenameFolderContents(DirectoryData* aDir, const std::filesystem::path& newDir)
{
	FileSystem::MoveFolder(aDir->path, newDir);
	aDir->path = newDir / aDir->path.filename();

	for (const auto& asset : aDir->assets)
	{
		Volt::AssetManager::Get().MoveAsset(asset.handle, newDir);
	}

	for (const auto& dir : aDir->subDirectories)
	{
		RecursiceRenameFolderContents(dir.get(), aDir->path);
	}
}

void AssetBrowserPanel::GenerateAssetPreviewsInCurrentDirectory()
{
	for (const auto& asset : myCurrentDirectory->assets)
	{
		switch (asset->type)
		{
			case Volt::AssetType::Mesh:
				asset->preview = CreateRef<AssetPreview>(asset->path);
				myPreviewsToUpdate.emplace_back(asset->preview);
				break;
		}
	}
}

void AssetBrowserPanel::CreateNewAssetInCurrentDirectory(Volt::AssetType type)
{
	const std::string extension = Volt::AssetManager::Get().GetExtensionFromAssetType(type);
	std::string originalName;
	std::string tempName;
	uint32_t i = 0;

	Volt::AssetHandle newAssetHandle = Volt::Asset::Null();

	switch (type)
	{
		case Volt::AssetType::Material: originalName = "M_NewMaterial"; break;
		case Volt::AssetType::AnimatedCharacter: originalName = "CHR_NewCharacter"; break;
		case Volt::AssetType::Shader: originalName = "SH_NewShader"; break;
		case Volt::AssetType::PhysicsMaterial: originalName = "PM_NewPhysicsMaterial"; break;
		case Volt::AssetType::Scene: originalName = "SC_NewScene"; break;
		case Volt::AssetType::ParticlePreset: originalName = "PP_NewParticlePreset"; break;
	}

	tempName = originalName;

	while (FileSystem::Exists(Volt::ProjectManager::GetDirectory() / myCurrentDirectory->path / (tempName + extension)))
	{
		tempName = originalName + " (" + std::to_string(i) + ")";
		i++;
	}

	switch (type)
	{
		case Volt::AssetType::Material:
		{
			Ref<Volt::Material> material = Volt::AssetManager::CreateAsset<Volt::Material>(Volt::AssetManager::GetRelativePath(myCurrentDirectory->path), tempName + extension);
			material->SetName(std::filesystem::path(tempName).stem().string());
			material->CreateSubMaterial(Volt::ShaderRegistry::Get("Deferred"));
			Volt::AssetManager::Get().SaveAsset(material);

			newAssetHandle = material->handle;
			break;
		}

		case Volt::AssetType::AnimatedCharacter:
		{
			myNewCharacterData.destination = Volt::AssetManager::GetRelativePath(myCurrentDirectory->path);
			myNewCharacterData.name = tempName;

			UI::OpenModal("New Character##assetBrowser");

			break;
		}

		case Volt::AssetType::Shader:
		{
			myNewShaderData = {};
			UI::OpenModal("New Shader##assetBrowser");
			break;
		}

		case Volt::AssetType::PhysicsMaterial:
		{
			break;
		}

		case Volt::AssetType::Scene:
		{
			break;
		}

		case Volt::AssetType::ParticlePreset:
		{
			Ref<Volt::ParticlePreset> particlePreset = Volt::AssetManager::CreateAsset<Volt::ParticlePreset>(Volt::AssetManager::GetRelativePath(myCurrentDirectory->path), tempName + extension);
			Volt::AssetManager::Get().SaveAsset(particlePreset);

			newAssetHandle = particlePreset->handle;
			break;
		}
	}

	Reload();

	auto assetIt = std::find_if(myCurrentDirectory->assets.begin(), myCurrentDirectory->assets.end(), [&tempName](const auto& lhs)
		{
			return lhs->path.stem().string() == tempName;
		});

	if (assetIt != myCurrentDirectory->assets.end())
	{
		(*assetIt)->currentRenamingName = tempName;
		(*assetIt)->isRenaming = true;

		mySelectionManager->Select((*assetIt).get());
	}
}

void AssetBrowserPanel::CreateNewShaderModal()
{
	if (UI::BeginModal("New Shader##assetBrowser"))
	{
		if (UI::BeginProperties("shaderProp"))
		{
			UI::Property("Name", myNewShaderData.name);
			UI::Property("Pixel Shader", myNewShaderData.createPixelShader);
			UI::Property("Vertex Shader", myNewShaderData.createVertexShader);

			UI::EndProperties();
		}

		if (ImGui::Button("Create"))
		{
			const std::string newShaderName = "SH_NewShader";
			const std::filesystem::path templatesPath = "Templates/Files/Shader";

			std::string tempName = newShaderName;
			uint32_t i = 0;

			while (FileSystem::Exists(Volt::ProjectManager::GetDirectory() / myCurrentDirectory->path / (tempName + ".vtsdef")))
			{
				tempName = newShaderName + " (" + std::to_string(i) + ")";
				i++;
			}

			const std::filesystem::path defaultPixelPath = "Engine/Shaders/HLSL/Forward/ForwardPBR_ps.hlsl";
			const std::filesystem::path defaultVertexPath = "Engine/Shaders/HLSL/Forward/ForwardPBR_vs.hlsl";

			const std::filesystem::path pixelDestinationPath = Volt::ProjectManager::GetDirectory() / myCurrentDirectory->path / (tempName + "_ps.hlsl");
			const std::filesystem::path vertexDestinationPath = Volt::ProjectManager::GetDirectory() / myCurrentDirectory->path / (tempName + "_vs.hlsl");
			const std::filesystem::path definitionDestinationPath = Volt::ProjectManager::GetDirectory() / myCurrentDirectory->path / (tempName + ".vtsdef");

			if (myNewShaderData.createPixelShader)
			{
				FileSystem::Copy(templatesPath / "ps_template.hlsl", pixelDestinationPath);
			}

			if (myNewShaderData.createVertexShader)
			{
				FileSystem::Copy(templatesPath / "vs_template.hlsl", vertexDestinationPath);
			}

			// Create definition
			{
				using namespace Volt; // YAML Serialization helpers

				YAML::Emitter out;
				out << YAML::BeginMap;
				VT_SERIALIZE_PROPERTY(name, myNewShaderData.name, out);
				VT_SERIALIZE_PROPERTY(internal, false, out);

				const std::filesystem::path pixelShader = myNewShaderData.createPixelShader ? myCurrentDirectory->path / (tempName + "_ps.hlsl") : defaultPixelPath;
				const std::filesystem::path vertexShader = myNewShaderData.createVertexShader ? myCurrentDirectory->path / (tempName + "_vs.hlsl") : defaultVertexPath;

				out << YAML::Key << "paths" << YAML::Value << std::vector<std::filesystem::path>{ pixelShader, vertexShader };
				out << YAML::Key << "inputTextures" << YAML::BeginSeq;
				{
					// Albedo
					{
						out << YAML::BeginMap;
						VT_SERIALIZE_PROPERTY(binding, 0, out);
						VT_SERIALIZE_PROPERTY(name, "Albedo", out);
						out << YAML::EndMap;
					}

					// Normal
					{
						out << YAML::BeginMap;
						VT_SERIALIZE_PROPERTY(binding, 1, out);
						VT_SERIALIZE_PROPERTY(name, "Normal", out);
						out << YAML::EndMap;
					}

					// Material
					{
						out << YAML::BeginMap;
						VT_SERIALIZE_PROPERTY(binding, 2, out);
						VT_SERIALIZE_PROPERTY(name, "Material", out);
						out << YAML::EndMap;
					}
				}
				out << YAML::EndSeq;
				out << YAML::EndMap;

				std::ofstream fout(definitionDestinationPath);
				fout << out.c_str();
				fout.close();
			}

			ImGui::CloseCurrentPopup();
		}

		ImGui::SameLine();

		if (ImGui::Button("Cancel"))
		{
			ImGui::CloseCurrentPopup();
		}

		UI::EndModal();
	}
}
