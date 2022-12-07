#include "sbpch.h"
#include "AssetBrowserPanel.h"

#include "Sandbox/Window/EditorIconLibrary.h"
#include "Sandbox/Window/EditorLibrary.h"

#include "Sandbox/Utility/EditorUtilities.h"
#include "Sandbox/Utility/GlobalEditorStates.h"
#include "Sandbox/Utility/AssetBrowserUtilities.h"

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

#include <Volt/Rendering/Shader/Shader.h>
#include <Volt/Rendering/Shader/ShaderRegistry.h>
#include <Volt/Rendering/Texture/Texture2D.h>

#include <Volt/Utility/FileSystem.h>
#include <Volt/Utility/UIUtility.h>

#include <Volt/Input/Input.h>
#include <Volt/Input/KeyCodes.h>

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

AssetBrowserPanel::AssetBrowserPanel(Ref<Volt::Scene>& aScene)
	: EditorWindow("Asset Browser"), myEditorScene(aScene)
{
	myWindowFlags = ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse;
	myIsOpen = true;

	SetMinWindowSize({ -1.f, 100.f });

	myDirectories[FileSystem::GetAssetsPath().string()] = ProcessDirectory(FileSystem::GetAssetsPath().string(), nullptr);
	myDirectories[FileSystem::GetEnginePath().string()] = ProcessDirectory(FileSystem::GetEnginePath().string(), nullptr);

	myEngineDirectory = myDirectories[FileSystem::GetEnginePath().string()].get();
	myAssetsDirectory = myDirectories[FileSystem::GetAssetsPath().string()].get();

	myAssetIcons[Volt::AssetType::Material] = Volt::AssetManager::GetAsset<Volt::Texture2D>("Editor/Textures/Icons/AssetIcons/icon_material.dds");
	myAssetIcons[Volt::AssetType::Mesh] = Volt::AssetManager::GetAsset<Volt::Texture2D>("Editor/Textures/Icons/AssetIcons/icon_mesh.dds");
	myAssetIcons[Volt::AssetType::MeshSource] = Volt::AssetManager::GetAsset<Volt::Texture2D>("Editor/Textures/Icons/AssetIcons/icon_meshSource.dds");
	myAssetIcons[Volt::AssetType::Skeleton] = Volt::AssetManager::GetAsset<Volt::Texture2D>("Editor/Textures/Icons/AssetIcons/icon_skeleton.dds");
	myAssetIcons[Volt::AssetType::Animation] = Volt::AssetManager::GetAsset<Volt::Texture2D>("Editor/Textures/Icons/AssetIcons/icon_animation.dds");
	myAssetIcons[Volt::AssetType::AnimatedCharacter] = Volt::AssetManager::GetAsset<Volt::Texture2D>("Editor/Textures/Icons/AssetIcons/icon_animatedCharacter.dds");
	myAssetIcons[Volt::AssetType::Scene] = Volt::AssetManager::GetAsset<Volt::Texture2D>("Editor/Textures/Icons/AssetIcons/icon_scene.dds");
	myAssetIcons[Volt::AssetType::ParticlePreset] = Volt::AssetManager::GetAsset<Volt::Texture2D>("Editor/Textures/Icons/AssetIcons/icon_particlePreset.dds");
	myAssetIcons[Volt::AssetType::Prefab] = Volt::AssetManager::GetAsset<Volt::Texture2D>("Editor/Textures/Icons/AssetIcons/icon_prefab.dds");

	myCurrentDirectory = myAssetsDirectory;
	GenerateAssetPreviewsInCurrentDirectory();

	myDirectoryButtons.emplace_back(myCurrentDirectory);
}

void AssetBrowserPanel::UpdateMainContent()
{
	float cellSize = myThumbnailSize + myThumbnailPadding;

	if (myNextDirectory)
	{
		DeselectAllAssets(myCurrentDirectory);
		DeselectAllDirectories(myCurrentDirectory);

		myCurrentDirectory->selected = false;
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

	UI::PushId();
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

		//Draw outline
		{
			ImGuiStyle& style = ImGui::GetStyle();
			auto color = style.Colors[ImGuiCol_FrameBg];

			UI::ScopedColor newColor(ImGuiCol_ChildBg, { color.x, color.y, color.z, color.w });
			UI::ScopedStyleFloat rounding(ImGuiStyleVar_ChildRounding, 2.f);

			ImGui::BeginChild("##outline");

			UI::ShiftCursor(5.f, 5.f);
			auto flags = (myAssetsDirectory->selected ? ImGuiTreeNodeFlags_Selected : ImGuiTreeNodeFlags_None) | ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_DefaultOpen;

			bool open = UI::TreeNodeImage(EditorIconLibrary::GetIcon(EditorIcon::Directory), "Assets", flags);

			if (ImGui::IsItemClicked())
			{
				myAssetsDirectory->selected = true;
				myNextDirectory = myAssetsDirectory;
			}

			if (open)
			{
				UI::ScopedStyleFloat2 spacing(ImGuiStyleVar_ItemSpacing, { 0.f, 0.f });

				for (const auto& subDir : myAssetsDirectory->subDirectories)
				{
					RenderDirectory(subDir);
				}
				UI::TreeNodePop();
			}

			ImGui::EndChild();
		}

		ImGui::TableNextColumn();

		ImGui::BeginChild("##view", ImVec2(ImGui::GetContentRegionAvail().x, ImGui::GetWindowHeight() - controlsBarHeight));
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

				if (ImGui::BeginPopupContextWindow("CreateMenu", ImGuiPopupFlags_MouseButtonRight | ImGuiPopupFlags_NoOpenOverExistingPopup | ImGuiPopupFlags_NoOpenOverItems))
				{
					ImGui::SetCursorPosX(300.f);
					ImGui::SetCursorPosX(ImGui::GetStyle().WindowPadding.x);

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

						myCurrentlyRenamingDirectoryName = tempName;

						auto dirIt = std::find_if(myCurrentDirectory->subDirectories.begin(), myCurrentDirectory->subDirectories.end(), [tempName](const Ref<DirectoryData> data)
							{
								return data->path.stem().string() == tempName;
							});

						if (dirIt != myCurrentDirectory->subDirectories.end())
						{
							myCurrentlyRenamingDirectory = (*dirIt).get();
							myCurrentlyRenamingDirectory->selected = true;
						}
					}

					ImGui::Separator();

					if (ImGui::MenuItem("Create Material"))
					{
						CreateNewAssetInCurrentDirectory(Volt::AssetType::Material);
					}

					if (ImGui::MenuItem("Create Animated Character"))
					{
						CreateNewAssetInCurrentDirectory(Volt::AssetType::AnimatedCharacter);
					}

					if (ImGui::MenuItem("Create Shader"))
					{
						CreateNewAssetInCurrentDirectory(Volt::AssetType::Shader);
					}

					if (ImGui::MenuItem("Create Physics Material"))
					{
						CreateNewAssetInCurrentDirectory(Volt::AssetType::PhysicsMaterial);
					}

					if (ImGui::MenuItem("Create Scene"))
					{
						CreateNewAssetInCurrentDirectory(Volt::AssetType::Scene);
					}

					if (ImGui::MenuItem("Create Particle Preset"))
					{
						CreateNewAssetInCurrentDirectory(Volt::AssetType::ParticlePreset);
					}

					ImGui::Separator();

					if (ImGui::MenuItem("Reload"))
					{
						Reload();
					}

					ImGui::EndPopup();
				}

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
		}
		ImGui::EndChild();
		ImGui::EndTable();
	}

	UI::PopId();

	if (!myDragDroppedMeshes.empty() && !myIsImporting)
	{
		const auto path = myDragDroppedMeshes.back();
		myDragDroppedMeshes.pop_back();

		AssetData assetData;
		assetData.handle = Volt::AssetManager::Get().GetAssetHandleFromPath(path);
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
					tempName = originalName + "(" + std::to_string(i) + ")";
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
			if (myIsFocused)
			{
				bool isAnySelected = false;
				for (const auto& asset : myCurrentDirectory->assets)
				{
					if (asset.selected)
					{
						isAnySelected = true;
						break;
					}
				}

				for (const auto& dir : myCurrentDirectory->subDirectories)
				{
					if (dir->selected)
					{
						isAnySelected = true;
						break;
					}
				}

				if (isAnySelected)
				{
					myShouldDeleteSelected = true;
				}
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

Ref<DirectoryData> AssetBrowserPanel::ProcessDirectory(const std::filesystem::path& path, Ref<DirectoryData> parent)
{
	Ref<DirectoryData> dirData = CreateRef<DirectoryData>();
	dirData->path = path;
	dirData->parentDir = parent.get();

	for (const auto& entry : std::filesystem::directory_iterator(path))
	{
		if (!entry.is_directory())
		{
			AssetData assetData;
			assetData.path = entry;
			assetData.type = Volt::AssetManager::Get().GetAssetTypeFromPath(entry);

			if (assetData.type != Volt::AssetType::None && !assetData.path.filename().string().contains(".vtthumb.png"))
			{
				if (myAssetMask == Volt::AssetType::None || (myAssetMask & assetData.type) != Volt::AssetType::None)
				{
					assetData.handle = Volt::AssetManager::Get().GetAssetHandleFromPath(entry);
					dirData->assets.emplace_back(assetData);
				}
			}
		}
		else
		{
			auto nextDirData = ProcessDirectory(entry, dirData);
			if ((!nextDirData->assets.empty() || !nextDirData->subDirectories.empty()) || myAssetMask == Volt::AssetType::None)
			{
				dirData->subDirectories.emplace_back(nextDirData);
			}
		}
	}

	std::sort(dirData->subDirectories.begin(), dirData->subDirectories.end(), [](const Ref<DirectoryData>& a, const Ref<DirectoryData>& b) { return a->path.string() < b->path.string(); });
	std::sort(dirData->assets.begin(), dirData->assets.end(), [](const AssetData& a, const AssetData& b) { return a.path.stem().string() < b.path.stem().string(); });

	return dirData;
}

std::vector<DirectoryData*> AssetBrowserPanel::FindParentDirectoriesOfDirectory(DirectoryData* directory)
{
	std::vector<DirectoryData*> directories;
	directories.emplace_back(directory);

	for (auto dir = directory->parentDir; dir != nullptr; dir = dir->parentDir)
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
			ImGui::Image(UI::GetTextureID(EditorIconLibrary::GetIcon(EditorIcon::Search)), { height - buttonSizeOffset, height - buttonSizeOffset });

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

				if (UI::ImageButton("##reloadButton", UI::GetTextureID(EditorIconLibrary::GetIcon(EditorIcon::Reload)), { height - buttonSizeOffset, height - buttonSizeOffset }))
				{
					Reload();
				}

				ImGui::SameLine();

				if (UI::ImageButton("##backButton", UI::GetTextureID(EditorIconLibrary::GetIcon(EditorIcon::Back)), { height - buttonSizeOffset, height - buttonSizeOffset }))
				{
					myHasSearchQuery = false;
					mySearchQuery.clear();

					if (myCurrentDirectory->path != FileSystem::GetAssetsPath() && myCurrentDirectory->path != FileSystem::GetEnginePath())
					{
						myNextDirectory = myCurrentDirectory->parentDir;
						myNextDirectory->selected = true;

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
						myNextDirectory->selected = true;

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
					UI::ImageButton("##filter", UI::GetTextureID(EditorIconLibrary::GetIcon(EditorIcon::Filter)), { height - buttonSizeOffset, height - buttonSizeOffset });
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

				ImGui::ImageButton(UI::GetTextureID(EditorIconLibrary::GetIcon(EditorIcon::Settings)), { height - buttonSizeOffset, height - buttonSizeOffset });
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

void AssetBrowserPanel::RenderDirectory(const Ref<DirectoryData> dirData)
{
	std::string id = dirData->path.stem().string() + "##" + std::to_string(dirData->handle);

	auto flags = (dirData->selected ? ImGuiTreeNodeFlags_Selected : ImGuiTreeNodeFlags_None) | ImGuiTreeNodeFlags_OpenOnArrow;

	bool open = UI::TreeNodeImage(EditorIconLibrary::GetIcon(EditorIcon::Directory), id, flags);
	if (ImGui::IsItemClicked())
	{
		dirData->selected = true;
		myNextDirectory = dirData.get();
	}

	if (void* ptr = UI::DragDropTarget({ "ASSET_BROWSER_ITEM" }))
	{
		Volt::AssetHandle handle = *(Volt::AssetHandle*)ptr;
		Volt::AssetManager::Get().MoveAsset(handle, dirData->path);
		Reload();
	}

	if (open)
	{
		for (const auto& subDir : dirData->subDirectories)
		{
			RenderDirectory(subDir);
		}

		for (const auto& asset : dirData->assets)
		{
			std::string assetId = asset.path.stem().string() + "##" + std::to_string(asset.handle);
			ImGui::Selectable(assetId.c_str());
		}

		ImGui::TreePop();
	}
}

void AssetBrowserPanel::RenderView(std::vector<Ref<DirectoryData>>& dirData, std::vector<AssetData>& assetData)
{
	RenderDirectories(dirData);
	RenderAssets(assetData);

	if (ImGui::IsWindowHovered() && ImGui::IsMouseClicked(ImGuiMouseButton_Left))
	{
		DeselectAllDirectories(myCurrentDirectory);
		DeselectAllAssets(myCurrentDirectory);
	}

	UI::ShiftCursor(0.f, 170.f); // Extra space at the bottom
}

void AssetBrowserPanel::RenderDirectories(std::vector<Ref<DirectoryData>>& dirData)
{
	for (auto& dir : dirData)
	{
		ImGui::PushID((uint32_t)dir->handle);

		const ImVec2 itemSize = AssetBrowserUtilities::GetBrowserItemSize();
		const float itemPadding = AssetBrowserUtilities::GetBrowserItemPadding();

		const ImVec2 minChild = AssetBrowserUtilities::GetBrowserItemMinPos();

		ImGui::PushStyleColor(ImGuiCol_ChildBg, { 0.f, 0.f, 0.f, 0.f });
		ImGui::BeginChild("hoverWindow", itemSize, false, ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse);
		ImGui::PopStyleColor();

		{
			const bool itemHovered = ImGui::IsWindowHovered(ImGuiHoveredFlags_ChildWindows);

			ImVec4 childBgCol = AssetBrowserUtilities::GetBrowserItemDefaultColor();

			if (itemHovered && ImGui::IsMouseDown(ImGuiMouseButton_Left))
			{
				childBgCol = AssetBrowserUtilities::GetBrowserItemClickedColor();
			}
			else if (itemHovered)
			{
				childBgCol = AssetBrowserUtilities::GetBrowserItemHoveredColor();
			}
			else if (dir->selected)
			{
				childBgCol = AssetBrowserUtilities::GetBrowserItemSelectedColor();
			}

			ImGui::PushStyleColor(ImGuiCol_ChildBg, childBgCol);
			ImGui::PushStyleVar(ImGuiStyleVar_ChildRounding, 2.f);
			ImGui::BeginChild("item", itemSize, false, ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse);
			{
				UI::ShiftCursor(itemPadding / 2.f, 10.f);


				ImGui::PushStyleColor(ImGuiCol_ChildBg, { 0.2f, 0.2f, 0.2f, 1.f });
				ImGui::BeginChild("image", { myThumbnailSize, myThumbnailSize });
				{
					{
						const ImVec4 typeColor = { 0.1f, 0.1f, 0.1f, 1.f };
						const ImVec2 barMin = minChild;
						const ImVec2 barMax = barMin + ImVec2{ 10.f, myThumbnailSize + itemPadding };
						ImGui::GetWindowDrawList()->AddRectFilled(barMin, barMax, ImColor(typeColor));
					}

					UI::ShiftCursor(itemPadding / 2.f, itemPadding / 2.f);
					ImGui::Image(UI::GetTextureID(EditorIconLibrary::GetIcon(EditorIcon::Directory)), { myThumbnailSize - itemPadding, myThumbnailSize - itemPadding });
					if (void* ptr = UI::DragDropTarget({ "ASSET_BROWSER_ITEM" }))
					{
						Volt::AssetHandle handle = *(Volt::AssetHandle*)ptr;
						Volt::AssetManager::Get().MoveAsset(handle, dir->path);
						Reload();

						ImGui::EndChild();
						ImGui::PopStyleVar();
						ImGui::PopStyleColor();
						ImGui::EndChild();
						ImGui::PopStyleColor();
						ImGui::NextColumn();
						ImGui::PopID();
						break;
					}

					if (RenderFolderPopup(dir.get()))
					{
						Reload();

						ImGui::EndChild();
						ImGui::PopStyleVar();
						ImGui::PopStyleColor();
						ImGui::EndChild();
						ImGui::PopStyleColor();
						ImGui::NextColumn();
						ImGui::PopID();

						break;
					}
				}
				ImGui::EndChild();
				ImGui::PopStyleVar();
				ImGui::PopStyleColor();

				UI::ShiftCursor(itemPadding / 2.f, 0.f);

				if (dir.get() == myCurrentlyRenamingDirectory)
				{
					std::string renameId = "###renameId" + dir->path.stem().string();
					ImGui::PushItemWidth(ImGui::GetContentRegionAvail().x);
					UI::ScopedColor background{ ImGuiCol_FrameBg, { 0.1f, 0.1f, 0.1f, 0.1f } };

					if (ImGui::InputTextString(renameId.c_str(), &myCurrentlyRenamingDirectoryName, ImGuiInputTextFlags_EnterReturnsTrue))
					{
						const std::filesystem::path newDir = dir->path.parent_path() / myCurrentlyRenamingDirectoryName;
						RecursiceRenameFolderContents(dir.get(), newDir);

						myLastRenamingDirectory = nullptr;

						Reload();

						myCurrentlyRenamingDirectory->selected = true;
						myCurrentlyRenamingDirectory = nullptr;

						ImGui::PopItemWidth();
						ImGui::EndChild();
						ImGui::EndChild();
						ImGui::PopStyleColor();
						ImGui::NextColumn();
						ImGui::PopID();
						break;
					}

					if (myCurrentlyRenamingDirectory != myLastRenamingDirectory)
					{
						ImGuiID widgetId = ImGui::GetCurrentWindow()->GetID(renameId.c_str());
						ImGui::SetFocusID(widgetId, ImGui::GetCurrentWindow());
						ImGui::SetKeyboardFocusHere(-1);
						myLastRenamingDirectory = myCurrentlyRenamingDirectory;
					}

					if (!ImGui::IsItemFocused())
					{
						myLastRenamingDirectory = nullptr;
						myCurrentlyRenamingDirectory = nullptr;
					}

					if (!ImGui::IsItemHovered() && ImGui::IsMouseClicked(ImGuiMouseButton_Left))
					{
						const std::filesystem::path newDir = dir->path.parent_path() / myCurrentlyRenamingDirectoryName;
						RecursiceRenameFolderContents(dir.get(), newDir);
						myLastRenamingDirectory = nullptr;

						Reload();
						myCurrentlyRenamingDirectory->selected = true;
						myCurrentlyRenamingDirectory = nullptr;
						ImGui::PopItemWidth();
						ImGui::EndChild();
						ImGui::EndChild();
						ImGui::PopStyleColor();
						ImGui::NextColumn();
						ImGui::PopID();
						break;
					}

					ImGui::PopItemWidth();
				}
				else
				{
					ImGui::TextWrapped(dir->path.filename().string().c_str());
				}


				if (ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left) && itemHovered)
				{
					dir->selected = true;
					myNextDirectory = dir.get();
				}

				if (ImGui::IsMouseClicked(ImGuiMouseButton_Left) && itemHovered)
				{
					if (!Volt::Input::IsKeyDown(VT_KEY_LEFT_CONTROL))
					{
						DeselectAllDirectories(myCurrentDirectory);
						DeselectAllAssets(myCurrentDirectory);
					}
					dir->selected = !dir->selected;
				}
			}
			ImGui::EndChild();
		}

		ImGui::EndChild();
		ImGui::PopStyleColor();
		ImGui::NextColumn();
		ImGui::PopID();
	}
}

void AssetBrowserPanel::RenderAssets(std::vector<AssetData>& assetData)
{
	for (AssetData& asset : assetData)
	{
		ImGui::PushID((uint32_t)asset.handle);

		Ref<Volt::Image2D> icon = (myAssetPreviews.contains(asset.handle) && myAssetPreviews.at(asset.handle)->IsRendered()) ? myAssetPreviews.at(asset.handle)->GetPreview() : nullptr;
		if (!icon && myAssetIcons.find(asset.type) != myAssetIcons.end())
		{
			icon = myAssetIcons.at(asset.type)->GetImage();
		}

		if (asset.type == Volt::AssetType::Texture)
		{
			//if (EditorUtils::HasThumbnail(asset.path))
			//{
			//	icon = Volt::AssetManager::GetAsset<Volt::Texture2D>(EditorUtils::GetThumbnailPathFromPath(asset.path))->GetImage();
			//}
			//else
			//{
			//	icon = EditorUtils::GenerateThumbnail(asset.path)->GetImage();
			//}
		}

		if (!icon)
		{
			icon = EditorIconLibrary::GetIcon(EditorIcon::GenericFile)->GetImage();
		}

		const ImVec2 itemSize = AssetBrowserUtilities::GetBrowserItemSize();
		const float itemPadding = AssetBrowserUtilities::GetBrowserItemPadding();

		const ImVec2 minChild = AssetBrowserUtilities::GetBrowserItemMinPos();

		ImGui::PushStyleColor(ImGuiCol_ChildBg, { 0.f, 0.f, 0.f, 0.f });
		ImGui::BeginChild("hoverWindow", itemSize, false, ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse);
		ImGui::PopStyleColor();

		{
			const bool itemHovered = ImGui::IsWindowHovered(ImGuiHoveredFlags_ChildWindows);

			ImVec4 childBgCol = AssetBrowserUtilities::GetBrowserItemDefaultColor();

			if (itemHovered && ImGui::IsMouseDown(ImGuiMouseButton_Left))
			{
				childBgCol = AssetBrowserUtilities::GetBrowserItemClickedColor();
			}
			else if (itemHovered)
			{
				childBgCol = AssetBrowserUtilities::GetBrowserItemHoveredColor();
			}
			else if (asset.selected)
			{
				childBgCol = AssetBrowserUtilities::GetBrowserItemSelectedColor();
			}

			ImGui::PushStyleColor(ImGuiCol_ChildBg, childBgCol);
			ImGui::PushStyleVar(ImGuiStyleVar_ChildRounding, 2.f);
			ImGui::BeginChild("item", itemSize, false, ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse);
			{
				UI::ShiftCursor(itemPadding / 2.f, 10.f);

				ImVec4 typeColor = Utility::GetColorFromType(asset.type);

				ImGui::PushStyleColor(ImGuiCol_ChildBg, { typeColor.x * 0.6f, typeColor.y * 0.6f, typeColor.z * 0.6f, 1.f });
				ImGui::BeginChild("image", { myThumbnailSize, myThumbnailSize }, false, ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse);
				{
					{
						ImVec2 barMin = minChild;
						ImVec2 barMax = barMin + ImVec2{ 10.f, myThumbnailSize + itemPadding };
						typeColor.w = 1.f;
						ImGui::GetWindowDrawList()->AddRectFilled(barMin, barMax, ImColor(typeColor));
					}

					UI::ShiftCursor(itemPadding / 2.f, itemPadding / 2.f);
					ImGui::Image(UI::GetTextureID(icon), { myThumbnailSize - itemPadding, myThumbnailSize - itemPadding });
					if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_SourceAllowNullID))
					{
						//Data being copied
						ImGui::SetDragDropPayload("ASSET_BROWSER_ITEM", &asset.handle, sizeof(Volt::AssetHandle), ImGuiCond_Once);

						// Viewport Drag drop
						GlobalEditorStates::dragStartedInAssetBrowser = true;
						GlobalEditorStates::dragAsset = asset.handle;
						GlobalEditorStates::isDragging = true;

						ImGui::EndDragDropSource();
					}

					if (RenderFilePopup(asset))
					{
						ImGui::EndChild();
						ImGui::PopStyleVar();
						ImGui::PopStyleColor();
						ImGui::EndChild();
						ImGui::EndChild();
						ImGui::PopStyleColor();
						ImGui::NextColumn();
						ImGui::PopID();

						break;
					}
				}
				ImGui::EndChild();
				ImGui::PopStyleVar();
				ImGui::PopStyleColor();

				UI::ShiftCursor(itemPadding / 2.f, 0.f);

				if (asset.handle == myCurrentlyRenamingAsset)
				{
					std::string renameId = "###renameId" + std::to_string(asset.handle);
					ImGui::PushItemWidth(ImGui::GetContentRegionAvail().x);
					UI::ScopedColor background{ ImGuiCol_FrameBg, { 0.1f, 0.1f, 0.1f, 0.1f } };

					if (ImGui::InputTextString(renameId.c_str(), &myCurrentlyRenamingAssetName, ImGuiInputTextFlags_EnterReturnsTrue))
					{
						Volt::AssetManager::Get().RenameAsset(asset.handle, myCurrentlyRenamingAssetName);
						switch (asset.type)
						{
							case Volt::AssetType::Material:
								Volt::AssetManager::GetAsset<Volt::Material>(asset.handle)->SetName(myCurrentlyRenamingAssetName);
								break;
						}

						myLastRenamingAsset = Volt::Asset::Null();
						myCurrentlyRenamingAsset = Volt::Asset::Null();

						Reload();
					}

					if (myCurrentlyRenamingAsset != myLastRenamingAsset)
					{
						ImGuiID widgetId = ImGui::GetCurrentWindow()->GetID(renameId.c_str());
						ImGui::SetFocusID(widgetId, ImGui::GetCurrentWindow());
						ImGui::SetKeyboardFocusHere(-1);
						myLastRenamingAsset = myCurrentlyRenamingAsset;
					}
					if (!ImGui::IsItemFocused())
					{
						myCurrentlyRenamingAsset = Volt::Asset::Null();
						myLastRenamingAsset = Volt::Asset::Null();
					}
					if (!ImGui::IsItemHovered() && ImGui::IsMouseClicked(ImGuiMouseButton_Left))
					{
						Volt::AssetManager::Get().RenameAsset(asset.handle, myCurrentlyRenamingAssetName);
						myCurrentlyRenamingAsset = Volt::Asset::Null();
						myLastRenamingAsset = Volt::Asset::Null();
					}

					ImGui::PopItemWidth();
				}
				else
				{
					ImGui::TextWrapped(asset.path.stem().string().c_str());
				}

				if (ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left) && itemHovered)
				{
					EditorLibrary::OpenAsset(Volt::AssetManager::Get().GetAssetRaw(asset.handle));
				}

				if (ImGui::IsMouseClicked(ImGuiMouseButton_Left) && itemHovered)
				{
					if (!Volt::Input::IsKeyDown(VT_KEY_LEFT_CONTROL))
					{
						DeselectAllAssets(myCurrentDirectory);
						DeselectAllDirectories(myCurrentDirectory);
					}
					asset.selected = !asset.selected;
				}
			}

			ImGui::EndChild();
		}

		ImGui::EndChild();

		ImGui::PopStyleColor();

		ImGui::NextColumn();
		ImGui::PopID();
	}
}

bool AssetBrowserPanel::RenderFilePopup(AssetData& data)
{
	bool removed = false;

	if (UI::BeginPopupItem(data.path.string(), ImGuiPopupFlags_MouseButtonRight))
	{
		DeselectAllAssets(myCurrentDirectory);
		DeselectAllDirectories(myCurrentDirectory);
		data.selected = true;

		if (ImGui::MenuItem("Open Externally"))
		{
			FileSystem::OpenFileExternally(data.path);
		}

		if (ImGui::MenuItem("Show in Explorer"))
		{
			FileSystem::ShowFileInExplorer(data.path);
		}

		if (ImGui::MenuItem("Reload asset"))
		{
			Volt::AssetManager::Get().ReloadAsset(data.handle);
		}

		ImGui::Separator();

		switch (data.type)
		{
			case Volt::AssetType::Shader: RenderShaderPopup(data); break;
			case Volt::AssetType::MeshSource: RenderMeshSourcePopup(data); break;
			case Volt::AssetType::Mesh: RenderMeshPopup(data); break;
			case Volt::AssetType::Animation: RenderAnimationPopup(data); break;
			case Volt::AssetType::Skeleton: RenderSkeletonPopup(data); break;

			default:
				break;
		}

		switch (data.type)
		{
			case Volt::AssetType::Shader:
			case Volt::AssetType::MeshSource:
			case Volt::AssetType::Mesh:
			case Volt::AssetType::Animation:
			case Volt::AssetType::Skeleton:
				ImGui::Separator();
		}

		if (ImGui::MenuItem("Rename"))
		{
			myCurrentlyRenamingAsset = data.handle;
			myCurrentlyRenamingAssetName = data.path.stem().string();
		}

		if (ImGui::MenuItem("Delete"))
		{
			removed = true;
			UI::OpenModal("Delete Selected Files?");
		}

		UI::EndPopup();
	}

	return removed;
}

bool AssetBrowserPanel::RenderFolderPopup(DirectoryData* data)
{
	bool removed = false;

	if (UI::BeginPopupItem(data->path.string(), ImGuiPopupFlags_MouseButtonRight))
	{
		DeselectAllAssets(myCurrentDirectory);
		DeselectAllDirectories(myCurrentDirectory);
		data->selected = true;

		if (ImGui::MenuItem("Show in Explorer"))
		{
			FileSystem::ShowDirectoryInExplorer(data->path);
		}

		if (ImGui::MenuItem("Rename"))
		{
			myCurrentlyRenamingDirectory = data;
			myCurrentlyRenamingDirectoryName = data->path.stem().string();
		}

		if (ImGui::MenuItem("Delete"))
		{
			UI::OpenModal("Delete Selected Files?");
		}

		UI::EndPopup();
	}

	return removed;
}

void AssetBrowserPanel::RenderFileInfo(const AssetData& data)
{
	if (!ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenBlockedByPopup))
	{
		return;
	}

	ImGui::OpenPopup("fileInfo");

	if (ImGui::BeginPopup("fileInfo"))
	{
		ImGui::TextUnformatted("test");

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
			for (const auto& asset : myCurrentDirectory->assets)
			{
				if (asset.selected)
				{
					Volt::AssetManager::Get().RemoveAsset(asset.handle);
				}
			}

			for (const auto& dir : myCurrentDirectory->subDirectories)
			{
				if (dir->selected)
				{
					RecursiveRemoveFolderContents(dir.get());
					FileSystem::Remove(dir->path);
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
	const std::filesystem::path currentPath = myCurrentDirectory ? myCurrentDirectory->path : FileSystem::GetAssetsPath();

	myCurrentDirectory = nullptr;
	myNextDirectory = nullptr;

	myDirectories[FileSystem::GetAssetsPath().string()] = ProcessDirectory(FileSystem::GetAssetsPath().string(), nullptr);
	myDirectories[FileSystem::GetEnginePath().string()] = ProcessDirectory(FileSystem::GetEnginePath().string(), nullptr);

	myEngineDirectory = myDirectories[FileSystem::GetEnginePath().string()].get();
	myAssetsDirectory = myDirectories[FileSystem::GetAssetsPath().string()].get();

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

void AssetBrowserPanel::RenderShaderPopup(const AssetData& data)
{
	if (ImGui::MenuItem("Recompile shaders"))
	{
		Ref<Volt::Shader> shader = Volt::AssetManager::GetAsset<Volt::Shader>(data.path);
		if (shader->Reload(true))
		{
			UI::Notify(NotificationType::Success, "Shader Compiled!", std::format("Shader {} compiled succesfully!", data.path.string()));
		}
		else
		{
			UI::Notify(NotificationType::Error, "Shader Compilation Failed", std::format("Shader {} failed to compile!", data.path.string()));
		}
	}
}

void AssetBrowserPanel::RenderMeshSourcePopup(const AssetData& data)
{
	if (ImGui::MenuItem("Import"))
	{
		myMeshImportData = {};
		myMeshToImport = data;
		myMeshImportData.destination = myMeshToImport.path.parent_path().string() + "\\" + myMeshToImport.path.stem().string() + ".vtmesh";
		UI::OpenModal("Import Mesh##assetBrowser");
	}
}

void AssetBrowserPanel::RenderMeshPopup(const AssetData& data)
{
	if (ImGui::MenuItem("Reimport"))
	{
		const std::filesystem::path fbxPath = data.path.parent_path() / (data.path.stem().string() + ".fbx");
		if (FileSystem::Exists(fbxPath))
		{
			Ref<Volt::Mesh> currentMesh = Volt::AssetManager::GetAsset<Volt::Mesh>(data.handle);
			if (currentMesh)
			{
				if (!FileSystem::IsWriteable(currentMesh->path))
				{
					UI::Notify(NotificationType::Error, "Unable to Compile Mesh!", std::format("Mesh {0} is not writeable!", data.path.string()), 5000);
					return;
				}

				Ref<Volt::Material> material;
				if (currentMesh->GetMaterial() && currentMesh->GetMaterial()->IsValid())
				{
					material = currentMesh->GetMaterial();
				}
				else
				{
					VT_CORE_WARN("Material for mesh {0} was invalid! Creating a new one!", currentMesh->path);
				}

				Volt::AssetHandle materialHandle = material ? material->handle : Volt::Asset::Null();
				bool recreatedMaterial = false;

				auto newFbxMesh = Volt::AssetManager::GetAsset<Volt::Mesh>(fbxPath);
				if (newFbxMesh && newFbxMesh->IsValid())
				{
					if (newFbxMesh->GetMaterial()->GetSubMaterials().size() != material->GetSubMaterials().size())
					{
						materialHandle = Volt::Asset::Null();
						recreatedMaterial = true;
					}

					if (Volt::MeshCompiler::TryCompile(newFbxMesh, data.path, materialHandle))
					{
						UI::Notify(NotificationType::Success, "Mesh Compiled!", std::format("Mesh {0} compiled successfully!", fbxPath.string()));
						Volt::AssetManager::Get().ReloadAsset(data.path);

						if (recreatedMaterial && material)
						{
							auto newMesh = Volt::AssetManager::GetAsset<Volt::Mesh>(data.path);
							size_t subMaterialCount = gem::min(newMesh->GetMaterial()->GetSubMaterials().size(), material->GetSubMaterials().size());

							// Set textures from old material
							for (size_t i = 0; i < subMaterialCount; i++)
							{
								for (const auto& [binding, tex] : material->GetSubMaterials().at(i)->GetTextures())
								{
									newMesh->GetMaterial()->GetSubMaterials().at(i)->SetTexture(binding, tex);
								}
							}

							Volt::AssetManager::Get().SaveAsset(newMesh->GetMaterial());
							Volt::AssetManager::Get().ReloadAsset(newMesh->GetMaterial()->path);
						}
					}
					else
					{
						UI::Notify(NotificationType::Error, "Mesh Failed to Compile!", std::format("Mesh {0} failed to compile!", fbxPath.string()));
					}
				}
				else
				{
					UI::Notify(NotificationType::Error, "Mesh Failed to Compile!", std::format("Mesh {0} failed to compile!", fbxPath.string()));
				}

				if (newFbxMesh)
				{
					Volt::AssetManager::Get().Unload(newFbxMesh->handle);
				}
			}
			else
			{
				UI::Notify(NotificationType::Error, "Unable to Compile Mesh!", std::format("Mesh {0} is invalid or cannot be found!", data.path.string()));
			}
		}
	}
}

void AssetBrowserPanel::RenderAnimationPopup(const AssetData& data)
{
	if (ImGui::MenuItem("Reimport"))
	{
		const std::filesystem::path fbxPath = data.path.parent_path() / (data.path.stem().string() + ".fbx");
		if (FileSystem::Exists(fbxPath))
		{
			Ref<Volt::Animation> originalAnimation = Volt::AssetManager::GetAsset<Volt::Animation>(data.handle);
			if (originalAnimation)
			{
				if (!FileSystem::IsWriteable(originalAnimation->path))
				{
					UI::Notify(NotificationType::Error, "Unable to Reimport Animation!", std::format("Animation {0} is not writeable!", data.path.string()), 5000);
					return;
				}

				Ref<Volt::Animation> newAnimation = Volt::MeshTypeImporter::ImportAnimation(fbxPath);
				if (newAnimation && newAnimation->IsValid())
				{
					newAnimation->handle = originalAnimation->handle;
					newAnimation->path = originalAnimation->path;

					Volt::AssetManager::Get().Unload(data.handle);
					Volt::AssetManager::Get().SaveAsset(newAnimation);

					UI::Notify(NotificationType::Success, "Animation Re imported!", std::format("Animation {0} was re imported successfully!", fbxPath.string()));
				}
				else
				{
					UI::Notify(NotificationType::Error, "Failed to Reimport Animation!", std::format("Animation {0} failed to import!", fbxPath.string()));
				}
			}
			else
			{
				UI::Notify(NotificationType::Error, "Failed to Reimport Animation!", std::format("Animation {0} failed was not found or is invalid!", data.path.string()));
			}
		}
		else
		{
			UI::Notify(NotificationType::Error, "Failed to Reimport Animation!", std::format("Animation {0} failed was not found!", fbxPath.string()));
		}
	}
}

void AssetBrowserPanel::RenderSkeletonPopup(const AssetData& data)
{
	if (ImGui::MenuItem("Reimport"))
	{
		const std::filesystem::path fbxPath = data.path.parent_path() / (data.path.stem().string() + ".fbx");
		if (FileSystem::Exists(fbxPath))
		{
			Ref<Volt::Skeleton> originalSkeleton = Volt::AssetManager::GetAsset<Volt::Skeleton>(data.handle);
			if (originalSkeleton)
			{
				if (!FileSystem::IsWriteable(originalSkeleton->path))
				{
					UI::Notify(NotificationType::Error, "Unable to Reimport Skeleton!", std::format("Skeleton {0} is not writeable!", data.path.string()), 5000);
					return;
				}

				Ref<Volt::Skeleton> newSkeleton = Volt::MeshTypeImporter::ImportSkeleton(fbxPath);
				if (newSkeleton && newSkeleton->IsValid())
				{
					newSkeleton->handle = originalSkeleton->handle;
					newSkeleton->path = originalSkeleton->path;

					Volt::AssetManager::Get().Unload(data.handle);
					Volt::AssetManager::Get().SaveAsset(newSkeleton);

					UI::Notify(NotificationType::Success, "Skeleton Re imported!", std::format("Skeleton {0} was reimported successfully!", fbxPath.string()));
				}
				else
				{
					UI::Notify(NotificationType::Error, "Failed to Reimport Skeleton!", std::format("Skeleton {0} failed to import!", fbxPath.string()));
				}
			}
			else
			{
				UI::Notify(NotificationType::Error, "Failed to Reimport Skeleton!", std::format("Skeleton {0} failed was not found or is invalid!", data.path.string()));
			}
		}
		else
		{
			UI::Notify(NotificationType::Error, "Failed to Reimport Skeleton!", std::format("Skeleton {0} failed was not found or is invalid!", fbxPath.string()));
		}
	}
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

void AssetBrowserPanel::FindFoldersAndFilesWithQuery(const std::vector<Ref<DirectoryData>>& dirList, std::vector<Ref<DirectoryData>>& directories, std::vector<AssetData>& assets, const std::string& query)
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
			std::string assetFilename = asset.path.filename().string();
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

DirectoryData* AssetBrowserPanel::FindDirectoryWithPath(const std::filesystem::path& path)
{
	std::vector<Ref<DirectoryData>> dirList;
	for (const auto& dir : myDirectories)
	{
		dirList.emplace_back(dir.second);
	}

	return FindDirectoryWithPathRecursivly(dirList, path);
}

DirectoryData* AssetBrowserPanel::FindDirectoryWithPathRecursivly(const std::vector<Ref<DirectoryData>> dirList, const std::filesystem::path& path)
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

void AssetBrowserPanel::DeselectAllDirectories(DirectoryData* rootDir)
{
	for (auto& dir : rootDir->subDirectories)
	{
		dir->selected = false;
	}
}

void AssetBrowserPanel::DeselectAllAssets(DirectoryData* rootDir)
{
	for (auto& asset : rootDir->assets)
	{
		asset.selected = false;
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
	myAssetPreviews.clear();

	for (const auto& asset : myCurrentDirectory->assets)
	{
		switch (asset.type)
		{
			case Volt::AssetType::Mesh:
				Ref<AssetPreview> preview = CreateRef<AssetPreview>(asset.path);

				myAssetPreviews.emplace(asset.handle, preview);
				myPreviewsToUpdate.emplace_back(preview);
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

	while (FileSystem::Exists(myCurrentDirectory->path / (tempName + extension)))
	{
		tempName = originalName + " (" + std::to_string(i) + ")";
		i++;
	}

	switch (type)
	{
		case Volt::AssetType::Material:
		{
			Ref<Volt::Material> material = Volt::AssetManager::CreateAsset<Volt::Material>(myCurrentDirectory->path, tempName + extension);
			material->SetName(std::filesystem::path(tempName).stem().string());
			material->CreateSubMaterial(Volt::ShaderRegistry::Get("Deferred"));
			Volt::AssetManager::Get().SaveAsset(material);

			newAssetHandle = material->handle;
			break;
		}

		case Volt::AssetType::AnimatedCharacter:
		{
			myNewCharacterData.destination = myCurrentDirectory->path;
			myNewCharacterData.name = tempName;

			UI::OpenModal("New Character##assetBrowser");

			break;
		}

		case Volt::AssetType::Shader:
		{
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
			Ref<Volt::ParticlePreset> particlePreset = Volt::AssetManager::CreateAsset<Volt::ParticlePreset>(myCurrentDirectory->path, tempName + extension);
			Volt::AssetManager::Get().SaveAsset(particlePreset);

			newAssetHandle = particlePreset->handle;
			break;
		}
	}

	myCurrentlyRenamingAssetName = tempName;
	myCurrentlyRenamingAsset = newAssetHandle;

	Reload();

	for (auto& asset : myCurrentDirectory->assets)
	{
		if (asset.handle == newAssetHandle)
		{
			asset.selected = true;
			break;
		}
	}
}