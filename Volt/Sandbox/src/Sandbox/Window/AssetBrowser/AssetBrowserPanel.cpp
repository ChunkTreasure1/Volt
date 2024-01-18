#include "sbpch.h"
#include "AssetBrowserPanel.h"

#include "Sandbox/Utility/EditorResources.h"
#include "Sandbox/Utility/EditorLibrary.h"

#include "Sandbox/Utility/EditorUtilities.h"
#include "Sandbox/Utility/GlobalEditorStates.h"
#include "Sandbox/Utility/AssetBrowserUtilities.h"
#include "Sandbox/Utility/Theme.h"

#include "Sandbox/Window/AssetBrowser/AssetItem.h"
#include "Sandbox/Window/AssetBrowser/DirectoryItem.h"
#include "Sandbox/Window/AssetBrowser/AssetBrowserSelectionManager.h"
#include "Sandbox/Window/AssetBrowser/PreviewRenderer.h"
#include "Sandbox/UserSettingsManager.h"

#include <Volt/Asset/AssetManager.h>
#include <Volt/Asset/Prefab.h>
#include <Volt/Asset/Mesh/MeshCompiler.h>
#include <Volt/Asset/Rendering/Material.h>
#include <Volt/Asset/Animation/Skeleton.h>
#include <Volt/Asset/Animation/Animation.h>
#include <Volt/Asset/Animation/AnimatedCharacter.h>
#include <Volt/Asset/Importers/MeshTypeImporter.h>
#include <Volt/Asset/ParticlePreset.h>
#include <Volt/Asset/Rendering/PostProcessingMaterial.h>
#include <Volt/Asset/Rendering/PostProcessingStack.h>

#include <Volt/Animation/BlendSpace.h>

#include <Volt/Components/CoreComponents.h>
#include <Volt/Scene/Scene.h>
#include <Volt/Project/ProjectManager.h>

#include <Volt/Rendering/Shader/Shader.h>
#include <Volt/Rendering/Texture/Texture2D.h>
#include <Volt/Rendering/Renderer.h>

#include <Volt/RenderingNew/Shader/ShaderMap.h>

#include <Volt/Utility/FileSystem.h>
#include <Volt/Utility/UIUtility.h>
#include <Volt/Utility/YAMLSerializationHelpers.h>
#include <Volt/Utility/SerializationMacros.h>
#include <Volt/Utility/PremadeCommands.h>

#include <Volt/Scripting/Mono/MonoScriptUtils.h>

#include <Volt/Input/Input.h>
#include <Volt/Input/KeyCodes.h>

#include <yaml-cpp/yaml.h>

AssetBrowserPanel::AssetBrowserPanel(Ref<Volt::Scene>& aScene, const std::string& id)
	: EditorWindow("Asset Browser" + id), myEditorScene(aScene)
{
	m_windowFlags = ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse;
	m_backgroundColor = EditorTheme::MiddleGreyBackground;
	m_isOpen = true;

	SetMinWindowSize({ 700.f, 300.f });

	mySelectionManager = CreateRef<AssetBrowser::SelectionManager>();

	if (!UserSettingsManager::GetSettings().sceneSettings.lowMemoryUsage)
	{
		myPreviewRenderer = CreateRef<PreviewRenderer>();
	}

	myDirectories[Volt::ProjectManager::GetAssetsDirectory()] = ProcessDirectory(Volt::ProjectManager::GetAssetsDirectory(), nullptr);
	myDirectories[FileSystem::GetEnginePath()] = ProcessDirectory(FileSystem::GetEnginePath(), nullptr);

	myAssetsDirectory = myDirectories[Volt::ProjectManager::GetAssetsDirectory()].get();

	myCurrentDirectory = myAssetsDirectory;

	myDirectoryButtons.emplace_back(myCurrentDirectory);
}

void AssetBrowserPanel::UpdateMainContent()
{
	float cellSize = GetThumbnailSize() + myThumbnailPadding;

	if (myNextDirectory)
	{
		mySelectionManager->DeselectAll();
		ClearAssetPreviewsInCurrentDirectory();
		myCurrentDirectory = myNextDirectory;
		myNextDirectory = nullptr;

		myDirectoryButtons.clear();
		myDirectoryButtons = FindParentDirectoriesOfDirectory(myCurrentDirectory);
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
				UI::ScopedColor headerColor{ ImGuiCol_Header, { 0.f } };
				UI::ScopedColor headerColorActive{ ImGuiCol_HeaderActive, { 0.f } };
				UI::ScopedColor headerColorHovered{ ImGuiCol_HeaderHovered, { 0.f } };

				UI::ShiftCursor(5.f, 5.f);

				const bool selected = myCurrentDirectory == myAssetsDirectory;
				const auto flags = ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_DefaultOpen | (selected ? ImGuiTreeNodeFlags_Selected : 0);

				UI::RenderHighlightedBackground(EditorTheme::ItemChildActive, 17.f);
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
				float panelWidth = ImGui::GetContentRegionAvail().x;
				auto columnCount = (int)(panelWidth / cellSize);

				if (columnCount < 1)
				{
					columnCount = 1;
				}

				ImGui::PushStyleColor(ImGuiCol_Button, ImVec4{ 0.f, 0.f, 0.f, 0.f });

				if (ImGui::BeginTable("##viewTable", columnCount))
				{
					ImGui::TableNextColumn();

					if (!myHasSearchQuery)
					{
						RenderView(myCurrentDirectory->subDirectories, myCurrentDirectory->assets);
					}
					else
					{
						RenderView(mySearchDirectories, mySearchAssets);
					}

					ImGui::EndTable();
				}

				RenderWindowRightClickPopup();

				ImGui::PopStyleColor();
			}
			ImGui::EndChild();

			if (void* ptr = UI::DragDropTarget("scene_entity_hierarchy"))
			{
				Volt::EntityID entity = *(Volt::EntityID*)ptr;
				if (entity != Volt::Entity::NullID())
				{
					CreatePrefabAndSetupEntities(entity);
					Reload();
				}
			}
			ImGui::EndChild();
		}

		ImGui::EndTable();
	}

	if (!myDragDroppedMeshes.empty() && !myIsImporting)
	{
		if (myDragDroppedMeshes.size() > 1)
		{
			myMeshImportData.destination = Volt::AssetManager::GetRelativePath(myCurrentDirectory->path);
			UI::OpenModal("Import Batch##assetBrowser");
		}
		else
		{
			const auto path = myDragDroppedMeshes.back();
			myDragDroppedMeshes.pop_back();

			AssetData assetData;
			assetData.handle = Volt::AssetManager::Get().AddAssetToRegistry(path);
			assetData.path = path;
			assetData.type = Volt::AssetType::MeshSource;

			myMeshImportData = {};
			myMeshToImport = assetData;
			myMeshImportData.destination = myMeshToImport.path.parent_path().string() + "\\" + myMeshToImport.path.stem().string() + ".vtmesh";

			UI::OpenModal("Import Mesh##assetBrowser");
		}

		myIsImporting = true;
	}

	if (!myDragDroppedTextures.empty())
	{
		for (const auto& path : myDragDroppedTextures)
		{
			if (path.extension().string() == ".dds")
			{
				continue;
			}

			EditorUtils::ImportTexture(path);
		}

		myDragDroppedTextures.clear();
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

	importState = EditorUtils::MeshBatchImportModal("Import Batch##assetBrowser", myMeshImportData, myDragDroppedMeshes);
	if (importState == ImportState::Imported)
	{
		Reload();
		myDragDroppedMeshes.clear();
		myIsImporting = false;
	}
	else if (importState == ImportState::Discard)
	{
		myDragDroppedMeshes.clear();
		myIsImporting = false;
	}

	if (EditorUtils::NewCharacterModal("New Character##assetBrowser", myNewAnimatedCharacter, myNewCharacterData))
	{
		myNewAnimatedCharacter = nullptr;
		Reload();
	}

	if (EditorUtils::NewAnimationGraphModal("New Animation Graph##assetBrowser", nullptr, myNewAnimationGraphData))
	{
		Reload();
	}

	CreateNewShaderModal();
	CreateNewMonoScriptModal();
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

	if (x > myViewBounds[0].x && y > myViewBounds[0].y && x < myViewBounds[1].x && y < myViewBounds[1].y)
	{
		for (const auto& path : e.GetPaths())
		{
			if (!std::filesystem::is_directory(path))
			{
				const std::string originalName = path.stem().string();
				std::string tempName = originalName;

				uint32_t i = 1;
				const auto relativePath = Volt::ProjectManager::GetDirectory() / myCurrentDirectory->path;

				while (FileSystem::Exists(relativePath / (tempName + path.extension().string())))
				{
					tempName = originalName + " (" + std::to_string(i) + ")";
					i++;
				}

				const std::filesystem::path targetPath = relativePath / (tempName + path.extension().string());

				const Volt::AssetType type = Volt::AssetManager::GetAssetTypeFromPath(targetPath);
				if (type == Volt::AssetType::MeshSource)
				{
					myDragDroppedMeshes.emplace_back(Volt::AssetManager::GetRelativePath(targetPath));
					FileSystem::Copy(path, targetPath);
				}
				//else if (type == Volt::AssetType::Texture)
				//{
				//	myDragDroppedTextures.emplace_back(path);
				//}
				else
				{
					FileSystem::Copy(path, targetPath);
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
			if (m_isFocused && mySelectionManager->IsAnySelected())
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
	if (UserSettingsManager::GetSettings().sceneSettings.lowMemoryUsage || !myPreviewRenderer)
	{
		return false;
	}

	for (const auto& asset : myCurrentDirectory->assets)
	{
		switch (asset->type)
		{
			case Volt::AssetType::Material:
			case Volt::AssetType::Mesh:
				if (!asset->previewImage)
				{
					myPreviewRenderer->RenderPreview(asset);
					return false;
				}
				break;
		}
	}
	return false;
}

Ref<AssetBrowser::DirectoryItem> AssetBrowserPanel::ProcessDirectory(const std::filesystem::path& path, AssetBrowser::DirectoryItem* parent)
{
	Ref<AssetBrowser::DirectoryItem> dirData = CreateRef<AssetBrowser::DirectoryItem>(mySelectionManager.get(), Volt::AssetManager::GetRelativePath(path));
	dirData->parentDirectory = parent;

	for (const auto& entry : std::filesystem::directory_iterator(path))
	{
		if (!entry.is_directory())
		{
			const auto type = Volt::AssetManager::GetAssetTypeFromPath(entry);
			const auto filename = entry.path().filename().string();

			if (type != Volt::AssetType::None && !Utility::StringContains(filename, ".vtthumb.png"))
			{
				if (myAssetMask == Volt::AssetType::None || (myAssetMask & type) != Volt::AssetType::None)
				{
					Ref<AssetBrowser::AssetItem> assetItem = CreateRef<AssetBrowser::AssetItem>(mySelectionManager.get(), Volt::AssetManager::GetRelativePath(entry.path()), myMeshImportData, myMeshToImport);
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

	if (ImGui::BeginChild("##controlsBar", { 0.f, std::min(height, ImGui::GetContentRegionAvail().y) }))
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

			if (UI::InputTextWithHint("", mySearchQuery, "Search..."))
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
				if (UI::ImageButton("##reloadButton", UI::GetTextureID(EditorResources::GetEditorIcon(EditorIcon::Reload)), { height - buttonSizeOffset, height - buttonSizeOffset }))
				{
					Reload();
				}

				ImGui::SameLine();

				if (UI::ImageButton("##backButton", UI::GetTextureID(EditorResources::GetEditorIcon(EditorIcon::Back)), { height - buttonSizeOffset, height - buttonSizeOffset }))
				{
					myHasSearchQuery = false;
					mySearchQuery.clear();

					if (myCurrentDirectory->path != Volt::ProjectManager::GetAssetsDirectory())
					{
						if (myCurrentDirectory->parentDirectory != nullptr)
						{
							myNextDirectory = myCurrentDirectory->parentDirectory;

							offsetToRemove = (uint32_t)(myDirectoryButtons.size() - 1);
							shouldRemove = true;
						}
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

			UI::ShiftCursor(ImGui::GetContentRegionAvail().x - 2.f * height - buttonSizeOffset, 0.f);

			// Filter button
			{
				UI::ImageButton("##filter", UI::GetTextureID(EditorResources::GetEditorIcon(EditorIcon::Filter)), { height - buttonSizeOffset, height - buttonSizeOffset });

				if (ImGui::BeginPopupContextItem("filterMenu", ImGuiPopupFlags_MouseButtonLeft))
				{
					UI::ScopedColor clearButtonBackground(ImGuiCol_Button, { 0.3f, 0.3f, 0.3f, 1.f });

					if (ImGui::Button("Clear##filterMenu"))
					{
						myAssetMask = Volt::AssetType::None;
						Reload();
					}

					for (const auto& asset : Volt::GetAssetNames())
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
				ImGui::ImageButton(UI::GetTextureID(EditorResources::GetEditorIcon(EditorIcon::Settings)), { height - buttonSizeOffset, height - buttonSizeOffset });
				if (ImGui::BeginPopupContextItem("settingsMenu", ImGuiPopupFlags_MouseButtonLeft))
				{
					ImGui::PushItemWidth(100.f);
					ImGui::SliderFloat("Icon size", &UserSettingsManager::GetSettings().assetBrowserSettings.thumbnailSize, 20.f, 200.f);
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

	auto isAnyDecendantActive = [&](Ref<AssetBrowser::DirectoryItem> dirData, auto isAnyDecendantActive, bool first)
	{
		if (myCurrentDirectory == dirData.get() && !first)
		{
			return true;
		}

		for (const auto& subDir : dirData->subDirectories)
		{
			if (isAnyDecendantActive(subDir, isAnyDecendantActive, false))
			{
				return true;
			}
		}

		return false;
	};

	const bool isDecendantActive = isAnyDecendantActive(dirData, isAnyDecendantActive, true);
	const bool selected = mySelectionManager->IsSelected(dirData.get()) || myCurrentDirectory == dirData.get() || isDecendantActive;
	const auto flags = (selected ? ImGuiTreeNodeFlags_Selected : ImGuiTreeNodeFlags_None) | ImGuiTreeNodeFlags_OpenOnArrow;

	bool hovered = false;
	constexpr float itemHeight = 17.f;

	// Check if item is hovered
	{
		const auto windowPos = ImGui::GetWindowPos();
		const auto availRegion = ImGui::GetContentRegionMax();
		const auto cursorPos = ImGui::GetCursorPos();

		const ImVec2 min = ImGui::GetWindowPos() + ImVec2{ 0.f, cursorPos.y };
		const ImVec2 max = ImGui::GetWindowPos() + ImVec2{ availRegion.x, itemHeight + cursorPos.y };
		hovered = ImGui::IsMouseHoveringRect(min, max);
	}

	// Draw background
	if (hovered)
	{
		UI::RenderHighlightedBackground(EditorTheme::ItemHovered, itemHeight);
	}
	else if (isDecendantActive)
	{
		UI::RenderHighlightedBackground(EditorTheme::ItemChildActive, itemHeight);
	}
	else if (selected)
	{
		UI::RenderHighlightedBackground(EditorTheme::ItemSelected, itemHeight);
	}


	const std::string id = dirData->path.stem().string() + "##" + dirData->path.string();
	const bool open = UI::TreeNodeImage(EditorResources::GetEditorIcon(EditorIcon::Directory), id, flags, isDecendantActive);

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
				Volt::AssetManager::Get().MoveFullFolder(item->path, newPath);
				FileSystem::MoveFolder(Volt::ProjectManager::GetDirectory() / item->path, Volt::ProjectManager::GetDirectory() / newPath);
			}
		}

		for (const auto& item : mySelectionManager->GetSelectedItems())
		{
			if (!item->isDirectory && item != dirData.get() && std::filesystem::exists(Volt::ProjectManager::GetDirectory() / item->path))
			{
				// Check for thumbnail PNG
				if (Volt::AssetManager::GetAssetTypeFromPath(item->path) == Volt::AssetType::Texture)
				{
					const std::filesystem::path thumbnailPath = Volt::ProjectManager::GetDirectory() / item->path.parent_path() / (item->path.filename().string() + ".vtthumb.png");
					if (FileSystem::Exists(thumbnailPath))
					{
						FileSystem::Move(thumbnailPath, Volt::ProjectManager::GetDirectory() / dirData->path);
					}
				}

				Volt::AssetManager::Get().MoveAsset(Volt::AssetManager::GetAssetHandleFromFilePath(item->path), dirData->path);
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

	bool skipEntitiesDir = false;
	for (const auto& asset : assets) { if (asset->type == Volt::AssetType::Scene) { skipEntitiesDir = true; break; } }

	for (const auto& dir : directories)
	{
		if (skipEntitiesDir && (dir->path.filename() == "Entities" || dir->path.filename() == "Layers")) { continue; }

		const bool changed = dir->Render();
		ImGui::TableNextColumn();

		if (changed)
		{
			reload = true;
			break;
		}

		if (dir->isNext)
		{
			myNextDirectory = dir.get();
			dir->isNext = false;
		}
	}

	if (reload)
	{
		reload = false;
		Reload();
	}

	for (const auto& asset : assets)
	{
		const bool changed = asset->Render();
		ImGui::TableNextColumn();

		if (changed)
		{
			reload = true;
			break;
		}

		MeshImportData data;
		auto meshes = AssetBrowser::AssetBrowserUtilities::GetMeshesExport();
		EditorUtils::MeshExportModal(std::format("Mesh Export##assetBrowser{0}", std::to_string(asset->handle)), myCurrentDirectory->path, data, meshes);

		if (UI::BeginModal(std::format("Reimport Animation##assetBrowser{0}", std::to_string(asset->handle))))
		{
			if (UI::BeginProperties())
			{
				EditorUtils::Property("Target Skeleton", myAnimationReimportTargetSkeleton, Volt::AssetType::Skeleton);

				UI::EndProperties();
			}

			if (ImGui::Button("Reimport"))
			{
				EditorUtils::ReimportSourceMesh(asset->handle, Volt::AssetManager::GetAsset<Volt::Skeleton>(myAnimationReimportTargetSkeleton));
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

	if (reload)
	{
		Reload();
	}

	if (ImGui::IsWindowHovered() && ImGui::IsMouseClicked(ImGuiMouseButton_Left))
	{
		mySelectionManager->DeselectAll();
	}

	UI::ShiftCursor(0.f, 200.f); // Extra space at the bottom
}

void AssetBrowserPanel::RenderWindowRightClickPopup()
{
	if (ImGui::BeginPopupContextWindow("CreateMenu", ImGuiPopupFlags_MouseButtonRight | ImGuiPopupFlags_NoOpenOverExistingPopup | ImGuiPopupFlags_NoOpenOverItems))
	{
		ImGui::SetCursorPosX(150.f);
		ImGui::SetCursorPosX(ImGui::GetStyle().WindowPadding.x);

		if (ImGui::BeginMenu("New"))
		{
			if (ImGui::BeginMenu("Materials##Menu"))
			{
				if (ImGui::MenuItem("Material"))
				{
					CreateNewAssetInCurrentDirectory(Volt::AssetType::Material);
				}

				if (ImGui::MenuItem("Mosaic Graph"))
				{
					CreateNewAssetInCurrentDirectory(Volt::AssetType::Material);
				}

				if (ImGui::MenuItem("Shader"))
				{
					CreateNewAssetInCurrentDirectory(Volt::AssetType::ShaderDefinition);
				}

				if (ImGui::MenuItem("Post Processing Stack"))
				{
					CreateNewAssetInCurrentDirectory(Volt::AssetType::PostProcessingStack);
				}

				if (ImGui::MenuItem("Post Processing Material"))
				{
					CreateNewAssetInCurrentDirectory(Volt::AssetType::PostProcessingMaterial);
				}

				if (ImGui::MenuItem("Physics Material"))
				{
					CreateNewAssetInCurrentDirectory(Volt::AssetType::PhysicsMaterial);
				}

				ImGui::EndMenu();
			}

			if (ImGui::BeginMenu("Animation##Menu"))
			{
				if (ImGui::MenuItem("Animated Character"))
				{
					CreateNewAssetInCurrentDirectory(Volt::AssetType::AnimatedCharacter);
				}

				if (ImGui::MenuItem("Animation Graph"))
				{
					CreateNewAssetInCurrentDirectory(Volt::AssetType::AnimationGraph);
				}

				if (ImGui::MenuItem("Blend Space"))
				{
					CreateNewAssetInCurrentDirectory(Volt::AssetType::BlendSpace);
				}
				ImGui::EndMenu();
			}

			UI::SmallSeparatorHeader("Other", 5.f);

			if (ImGui::MenuItem("Scene"))
			{
				CreateNewAssetInCurrentDirectory(Volt::AssetType::Scene);
			}

			if (ImGui::MenuItem("Particle Preset"))
			{
				CreateNewAssetInCurrentDirectory(Volt::AssetType::ParticlePreset);
			}

			if (ImGui::MenuItem("C# Script"))
			{
				CreateNewAssetInCurrentDirectory(Volt::AssetType::MonoScript);
			}

			UI::SmallSeparatorHeader("File system", 5.f);

			if (ImGui::MenuItem("Folder"))
			{
				const std::string originalName = "New Folder";
				std::string tempName = originalName;

				uint32_t i = 0;
				while (FileSystem::Exists(Volt::ProjectManager::GetDirectory() / myCurrentDirectory->path / tempName))
				{
					tempName = originalName + " (" + std::to_string(i) + ")";
					i++;
				}

				FileSystem::CreateDirectory(Volt::ProjectManager::GetDirectory() / myCurrentDirectory->path / tempName);
				Reload();

				auto dirIt = std::find_if(myCurrentDirectory->subDirectories.begin(), myCurrentDirectory->subDirectories.end(), [tempName](const Ref<AssetBrowser::DirectoryItem> data)
				{
					return data->path.stem().string() == tempName;
				});

				if (dirIt != myCurrentDirectory->subDirectories.end())
				{
					(*dirIt)->StartRename();

					mySelectionManager->Select((*dirIt).get());
				}
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
					Volt::AssetManager::Get().RemoveFullFolderFromRegistry(Volt::AssetManager::GetRelativePath(item->path));
					FileSystem::MoveToRecycleBin(Volt::ProjectManager::GetDirectory() / item->path);
				}
			}

			for (const auto& item : selectedItems)
			{
				if (!item->isDirectory && Volt::AssetManager::ExistsInRegistry(item->path))
				{
					const auto assetType = Volt::AssetManager::GetAssetTypeFromPath(item->path);
					switch (assetType)
					{
						case Volt::AssetType::ShaderDefinition:
						{
							auto shader = Volt::AssetManager::GetAsset<Volt::Shader>(item->path);
							//Volt::ShaderRegistry::Unregister(shader->GetName());
							break;
						}
					}

					Volt::AssetManager::Get().RemoveAsset(Volt::AssetManager::GetRelativePath(item->path));
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
	const std::filesystem::path currentPath = myCurrentDirectory ? myCurrentDirectory->path : Volt::ProjectManager::GetAssetsDirectory();

	myCurrentDirectory = nullptr;
	myNextDirectory = nullptr;
	mySelectionManager->DeselectAll();

	ClearAssetPreviewsInCurrentDirectory();

	myDirectories[Volt::ProjectManager::GetAssetsDirectory()] = ProcessDirectory(Volt::ProjectManager::GetAssetsDirectory(), nullptr);

	myAssetsDirectory = myDirectories[Volt::ProjectManager::GetAssetsDirectory()].get();

	//Find directory
	myCurrentDirectory = FindDirectoryWithPath(currentPath);
	if (!myCurrentDirectory)
	{
		myCurrentDirectory = myAssetsDirectory;
	}

	//Setup new file path buttons
	myDirectoryButtons.clear();
	myDirectoryButtons = FindParentDirectoriesOfDirectory(myCurrentDirectory);
}

void AssetBrowserPanel::Search(const std::string& inQuery)
{
	std::vector<std::string> queries;
	std::vector<std::string> types;

	std::string searchQuery = inQuery;
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

void AssetBrowserPanel::CreatePrefabAndSetupEntities(Volt::EntityID id)
{
	Volt::Entity entity = myEditorScene->GetEntityFromUUID(id);

	if (entity.HasComponent<Volt::PrefabComponent>())
	{
		UI::Notify(NotificationType::Error, "Unable to create prefab!", "Cannot create prefab of existing prefab!");
		return;
	}

	const auto& tagComp = entity.GetComponent<Volt::TagComponent>();

	std::string name = tagComp.tag;
	name.erase(std::remove_if(name.begin(), name.end(), ::isspace), name.end());

	Ref<Volt::Prefab> prefab = Volt::AssetManager::CreateAsset<Volt::Prefab>(Volt::AssetManager::GetRelativePath(myCurrentDirectory->path), name, entity);
	Volt::AssetManager::SaveAsset(prefab);

	SetupEntityAsPrefab(entity.GetID(), prefab->handle);
	Reload();
}

void AssetBrowserPanel::SetupEntityAsPrefab(Volt::EntityID id, Volt::AssetHandle prefabId)
{
	Volt::Entity entity = myEditorScene->GetEntityFromUUID(id);

	if (!entity.HasComponent<Volt::PrefabComponent>())
	{
		entity.AddComponent<Volt::PrefabComponent>();
	}

	auto& prefabComp = entity.GetComponent<Volt::PrefabComponent>();
	prefabComp.prefabAsset = prefabId;
	prefabComp.prefabEntity = entity.GetID();

	auto& relComp = entity.GetComponent<Volt::RelationshipComponent>();
	for (const auto& child : relComp.children)
	{
		SetupEntityAsPrefab(child, prefabId);
	}
}

void AssetBrowserPanel::RecursiveRemoveFolderContents(DirectoryData* aDir)
{
	for (const auto& asset : aDir->assets)
	{
		if (FileSystem::Exists(Volt::ProjectManager::GetDirectory() / asset.path))
		{
			Volt::AssetManager::Get().RemoveAsset(asset.handle);
		}
	}

	aDir->assets.clear();

	for (const auto& dir : aDir->subDirectories)
	{
		if (FileSystem::Exists(Volt::ProjectManager::GetDirectory() / dir->path))
		{
			RecursiveRemoveFolderContents(dir.get());
			FileSystem::Remove(Volt::ProjectManager::GetDirectory() / dir->path);
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

void AssetBrowserPanel::ClearAssetPreviewsInCurrentDirectory()
{
	//for (const auto& asset : myCurrentDirectory->assets)
	//{
	//	switch (asset->type)
	//	{
	//		case Volt::AssetType::Mesh:
	//			asset->preview = CreateRef<AssetPreview>(asset->path);
	//			myPreviewsToUpdate.emplace_back(asset->preview);
	//			break;
	//	}
	//}
}

float AssetBrowserPanel::GetThumbnailSize()
{
	return UserSettingsManager::GetSettings().assetBrowserSettings.thumbnailSize;

}

void AssetBrowserPanel::CreateNewAssetInCurrentDirectory(Volt::AssetType type)
{
	std::string originalName;
	std::string tempName;
	uint32_t i = 0;

	Volt::AssetHandle newAssetHandle = Volt::Asset::Null();

	switch (type)
	{
		case Volt::AssetType::Material: originalName = "M_NewMaterial"; break;
		case Volt::AssetType::AnimatedCharacter: originalName = "CHR_NewCharacter"; break;
		case Volt::AssetType::ShaderDefinition: originalName = "SH_NewShader"; break;
		case Volt::AssetType::PhysicsMaterial: originalName = "PM_NewPhysicsMaterial"; break;
		case Volt::AssetType::Scene: originalName = "SC_NewScene"; break;
		case Volt::AssetType::ParticlePreset: originalName = "PP_NewParticlePreset"; break;
		case Volt::AssetType::AnimationGraph: originalName = "AG_NewAnimationGraph"; break;
		case Volt::AssetType::BlendSpace: originalName = "BS_NewBlendSpace"; break;
		case Volt::AssetType::MonoScript: originalName = "idk.cs"; break;
		case Volt::AssetType::PostProcessingStack: originalName = "PPS_NewPostStack"; break;
		case Volt::AssetType::PostProcessingMaterial: originalName = "PPM_NewPostMaterial"; break;
	}

	tempName = originalName;

	const std::string ext = Volt::AssetManager::GetExtensionFromAssetType(type);
	while (FileSystem::Exists(Volt::ProjectManager::GetDirectory() / Volt::AssetManager::GetRelativePath(myCurrentDirectory->path) / (tempName + ext)))
	{
		tempName = originalName + " (" + std::to_string(i) + ")";
		i++;
	}

	switch (type)
	{
		case Volt::AssetType::Material:
		{
			Ref<Volt::Material> material = Volt::AssetManager::CreateAsset<Volt::Material>(Volt::AssetManager::GetRelativePath(myCurrentDirectory->path), tempName);
			Volt::AssetManager::SaveAsset(material);

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

		case Volt::AssetType::AnimationGraph:
		{
			myNewAnimationGraphData.destination = Volt::AssetManager::GetRelativePath(myCurrentDirectory->path);
			myNewAnimationGraphData.name = tempName;

			UI::OpenModal("New Animation Graph##assetBrowser");
			break;
		}

		case Volt::AssetType::ShaderDefinition:
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
			FileSystem::CreateDirectory(Volt::AssetManager::GetRelativePath(myCurrentDirectory->path) / tempName);

			Ref<Volt::Scene> scene = Volt::Scene::CreateDefaultScene("New Scene");
			const std::filesystem::path targetFilePath = (Volt::AssetManager::GetRelativePath(myCurrentDirectory->path / tempName / (tempName + ext)));
			Volt::AssetManager::SaveAssetAs(scene, targetFilePath);
			break;
		}

		case Volt::AssetType::BlendSpace:
		{
			Ref<Volt::BlendSpace> blendSpace = Volt::AssetManager::CreateAsset<Volt::BlendSpace>(Volt::AssetManager::GetRelativePath(myCurrentDirectory->path), tempName);
			Volt::AssetManager::SaveAsset(blendSpace);

			newAssetHandle = blendSpace->handle;
			break;
		}

		case Volt::AssetType::ParticlePreset:
		{
			Ref<Volt::ParticlePreset> particlePreset = Volt::AssetManager::CreateAsset<Volt::ParticlePreset>(Volt::AssetManager::GetRelativePath(myCurrentDirectory->path), tempName);
			Volt::AssetManager::SaveAsset(particlePreset);

			newAssetHandle = particlePreset->handle;
			break;
		}

		case Volt::AssetType::MonoScript:
		{
			UI::OpenModal("New MonoScript##assetBrowser");
			break;
		}

		case Volt::AssetType::PostProcessingStack:
		{
			Ref<Volt::PostProcessingStack> postStack = Volt::AssetManager::CreateAsset<Volt::PostProcessingStack>(Volt::AssetManager::GetRelativePath(myCurrentDirectory->path), tempName);
			Volt::AssetManager::SaveAsset(postStack);

			newAssetHandle = postStack->handle;

			break;
		}

		case Volt::AssetType::PostProcessingMaterial:
		{
			Ref<Volt::PostProcessingMaterial> postStack = Volt::AssetManager::CreateAsset<Volt::PostProcessingMaterial>(Volt::AssetManager::GetRelativePath(myCurrentDirectory->path), tempName, Volt::Renderer::GetDefaultData().defaultPostProcessingShader);
			Volt::AssetManager::SaveAsset(postStack);

			newAssetHandle = postStack->handle;
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
		(*assetIt)->StartRename();

		mySelectionManager->Select((*assetIt).get());
	}
}

void AssetBrowserPanel::CreateNewShaderModal()
{
	if (UI::BeginModal("New Shader##assetBrowser"))
	{
		const std::vector<std::string> shaderTypeOptions = { "PBR", "PBR Transparent", "Particle", "Post Processing", "Decal" };

		constexpr int32_t PBR_SHADER = 0;
		constexpr int32_t PBR_TRANSPARENT_SHADER = 1;
		constexpr int32_t PARTICLE_SHADER = 2;
		constexpr int32_t POST_PROCESSING_SHADER = 3;
		constexpr int32_t DECAL_SHADER = 4;

		if (UI::BeginProperties("shaderProp"))
		{
			UI::Property("Name", myNewShaderData.name);

			UI::ComboProperty("Shader Type", myNewShaderData.shaderType, shaderTypeOptions);

			switch (myNewShaderData.shaderType)
			{
				case PBR_TRANSPARENT_SHADER:
				case DECAL_SHADER:
				case PBR_SHADER:
				{
					UI::Property("Pixel Shader", myNewShaderData.createPixelShader);
					UI::Property("Vertex Shader", myNewShaderData.createVertexShader);

					break;
				}

				case PARTICLE_SHADER:
				{
					UI::Property("Pixel Shader", myNewShaderData.createPixelShader);
					UI::Property("Geometry Shader", myNewShaderData.createGeometryShader);
					UI::Property("Vertex Shader", myNewShaderData.createVertexShader);

					break;
				}
			}

			UI::EndProperties();
		}

		if (ImGui::Button("Create"))
		{
			const std::filesystem::path templatesPath = "Templates/Files/Shader";

			std::string tempName = myNewShaderData.name;
			uint32_t i = 0;

			while (FileSystem::Exists(Volt::ProjectManager::GetDirectory() / myCurrentDirectory->path / (tempName + ".vtsdef")))
			{
				tempName = myNewShaderData.name + " (" + std::to_string(i) + ")";
				i++;
			}

			const std::filesystem::path definitionDestinationPath = Volt::ProjectManager::GetDirectory() / myCurrentDirectory->path / (tempName + ".vtsdef");
			std::vector<std::filesystem::path> shaderPaths;

			switch (myNewShaderData.shaderType)
			{
				case PBR_TRANSPARENT_SHADER:
				case PBR_SHADER:
				{
					const std::filesystem::path defaultPixelPath = myNewShaderData.shaderType == PBR_SHADER ? "Engine/Shaders/Source/HLSL/Forward/ForwardPBR_ps.hlsl" : "Engine/Shaders/Source/HLSL/Forward/ForwardPBRTransparent_ps.hlsl";
					const std::filesystem::path defaultVertexPath = "Engine/Shaders/Source/HLSL/Forward/ForwardPBR_vs.hlsl";
					const std::filesystem::path pixelDestinationPath = Volt::ProjectManager::GetDirectory() / myCurrentDirectory->path / (tempName + "_ps.hlsl");
					const std::filesystem::path vertexDestinationPath = Volt::ProjectManager::GetDirectory() / myCurrentDirectory->path / (tempName + "_vs.hlsl");

					if (myNewShaderData.createPixelShader)
					{
						const std::string templateName = myNewShaderData.shaderType == PBR_SHADER ? "ps_template.hlsl" : "ps_transparent_template.hlsl";

						FileSystem::Copy(templatesPath / templateName, pixelDestinationPath);
						shaderPaths.emplace_back(Volt::AssetManager::GetRelativePath(pixelDestinationPath));
					}
					else
					{
						shaderPaths.emplace_back(defaultPixelPath);
					}

					if (myNewShaderData.createVertexShader)
					{
						FileSystem::Copy(templatesPath / "vs_template.hlsl", vertexDestinationPath);
						shaderPaths.emplace_back(Volt::AssetManager::GetRelativePath(vertexDestinationPath));
					}
					else
					{
						shaderPaths.emplace_back(defaultVertexPath);
					}

					break;
				}

				case DECAL_SHADER:
				{
					const std::filesystem::path defaultPixelPath = "Engine/Shaders/Source/HLSL/Deferred/Decal_ps.hlsl";
					const std::filesystem::path defaultVertexPath = "Engine/Shaders/Source/HLSL/Deferred/Decal_vs.hlsl";
					const std::filesystem::path pixelDestinationPath = Volt::ProjectManager::GetDirectory() / myCurrentDirectory->path / (tempName + "_ps.hlsl");
					const std::filesystem::path vertexDestinationPath = Volt::ProjectManager::GetDirectory() / myCurrentDirectory->path / (tempName + "_vs.hlsl");

					if (myNewShaderData.createPixelShader)
					{
						const std::string templateName = "Decal/decalTemplate_ps.hlsl";

						FileSystem::Copy(templatesPath / templateName, pixelDestinationPath);
						shaderPaths.emplace_back(Volt::AssetManager::GetRelativePath(pixelDestinationPath));
					}
					else
					{
						shaderPaths.emplace_back(defaultPixelPath);
					}

					if (myNewShaderData.createVertexShader)
					{
						FileSystem::Copy(templatesPath / "Decal/decalTemplate_vs.hlsl", vertexDestinationPath);
						shaderPaths.emplace_back(Volt::AssetManager::GetRelativePath(vertexDestinationPath));
					}
					else
					{
						shaderPaths.emplace_back(defaultVertexPath);
					}

					break;
				}

				case PARTICLE_SHADER:
				{
					const std::filesystem::path defaultPixelPath = "Engine/Shaders/Source/HLSL/2D/Particle_ps.hlsl";
					const std::filesystem::path defaultGeometryPath = "Engine/Shaders/Source/HLSL/2D/Particle_gs.hlsl";
					const std::filesystem::path defaultVertexPath = "Engine/Shaders/Source/HLSL/2D/Particle_vs.hlsl";

					const std::filesystem::path pixelDestinationPath = Volt::ProjectManager::GetDirectory() / myCurrentDirectory->path / (tempName + "_ps.hlsl");
					const std::filesystem::path geometryDestinationPath = Volt::ProjectManager::GetDirectory() / myCurrentDirectory->path / (tempName + "_gs.hlsl");
					const std::filesystem::path vertexDestinationPath = Volt::ProjectManager::GetDirectory() / myCurrentDirectory->path / (tempName + "_vs.hlsl");

					if (myNewShaderData.createPixelShader)
					{
						FileSystem::Copy(templatesPath / "Particle/particleTemplate_ps.hlsl", pixelDestinationPath);
						shaderPaths.emplace_back(Volt::AssetManager::GetRelativePath(pixelDestinationPath));
					}
					else
					{
						shaderPaths.emplace_back(defaultPixelPath);
					}

					if (myNewShaderData.createGeometryShader)
					{
						FileSystem::Copy(templatesPath / "Particle/particleTemplate_gs.hlsl", geometryDestinationPath);
						shaderPaths.emplace_back(Volt::AssetManager::GetRelativePath(geometryDestinationPath));
					}
					else
					{
						shaderPaths.emplace_back(defaultGeometryPath);
					}

					if (myNewShaderData.createVertexShader)
					{
						FileSystem::Copy(templatesPath / "Particle/particleTemplate_vs.hlsl", vertexDestinationPath);
						shaderPaths.emplace_back(Volt::AssetManager::GetRelativePath(vertexDestinationPath));
					}
					else
					{
						shaderPaths.emplace_back(defaultVertexPath);
					}

					break;
				}

				case POST_PROCESSING_SHADER:
				{
					const std::filesystem::path path = myCurrentDirectory->path / (tempName + "_cs.hlsl");
					const std::filesystem::path computeDestinationPath = Volt::ProjectManager::GetDirectory() / path;

					FileSystem::Copy(templatesPath / "PostProcessing/templatePostProcessing_cs.hlsl", computeDestinationPath);
					shaderPaths.emplace_back(path);

					break;
				}
			}

			// Create definition
			{
				using namespace Volt; // YAML Serialization helpers

				YAML::Emitter out;
				out << YAML::BeginMap;
				VT_SERIALIZE_PROPERTY(name, myNewShaderData.name, out);
				VT_SERIALIZE_PROPERTY(internal, false, out);

				out << YAML::Key << "paths" << YAML::Value << shaderPaths;
				out << YAML::Key << "inputTextures" << YAML::BeginSeq;
				out << YAML::EndSeq;
				out << YAML::EndMap;

				std::ofstream fout(definitionDestinationPath);
				fout << out.c_str();
				fout.close();

				Ref<Volt::Shader> newShader = Volt::AssetManager::CreateAsset<Volt::Shader>(Volt::AssetManager::GetRelativePath(myCurrentDirectory->path), tempName, tempName, shaderPaths, false);
				//Volt::ShaderRegistry::Register(tempName, newShader);
			}

			ImGui::CloseCurrentPopup();
			Reload();
		}

		ImGui::SameLine();

		if (ImGui::Button("Cancel"))
		{
			ImGui::CloseCurrentPopup();
		}

		UI::EndModal();
	}
}

void AssetBrowserPanel::CreateNewMonoScriptModal()
{
	if (UI::BeginModal("New MonoScript##assetBrowser"))
	{
		static std::string name;
		static bool regeneratePrj = true;

		if (UI::BeginProperties("scriptProp"))
		{
			UI::Property("Name", name);
			UI::Property("Regenerate Project", regeneratePrj);

			UI::EndProperties();
		}

		if (ImGui::Button("Create"))
		{
			Volt::MonoScriptUtils::CreateNewCSFile(name, myCurrentDirectory->path, regeneratePrj);

			name = "";
			ImGui::CloseCurrentPopup();
		}

		ImGui::SameLine();

		if (ImGui::Button("Cancel"))
		{
			name = "";
			ImGui::CloseCurrentPopup();
		}

		UI::EndModal();
	}
}
