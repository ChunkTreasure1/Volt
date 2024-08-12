#include "sbpch.h"
#include "AssetItem.h"

#include "Sandbox/Sandbox.h"
#include "Sandbox/Window/AssetBrowser/AssetBrowserSelectionManager.h"

#include "Sandbox/Utility/EditorResources.h"
#include "Sandbox/Utility/EditorUtilities.h"
#include "Sandbox/Utility/AssetBrowserUtilities.h"

#include "Sandbox/Utility/GlobalEditorStates.h"
#include "Sandbox/Utility/EditorLibrary.h"
#include "Sandbox/VersionControl/VersionControl.h"

#include "Sandbox/UserSettingsManager.h"

#include "Sandbox/Window/AssetBrowser/EditorAssetRegistry.h"

#include <Volt/Asset/AssetManager.h>
#include <Volt/Input/KeyCodes.h>
#include <Volt/Utility/PremadeCommands.h>
#include <Volt/Rendering/Texture/Texture2D.h>

namespace AssetBrowser
{
	AssetItem::AssetItem(SelectionManager* selectionManager, const std::filesystem::path& path, MeshImportData& aMeshImportData, AssetData& aMeshToImportData)
		: Item(selectionManager, path), meshImportData(aMeshImportData), meshToImportData(aMeshToImportData)
	{
		type = Volt::AssetManager::GetAssetTypeFromPath(path);
		handle = Volt::AssetManager::Get().AddAssetToRegistry(path);
	}

	bool AssetItem::Render()
	{
		bool reload = Item::Render();

		if (SaveReturnState returnState = EditorUtils::SaveFilePopup("Do you want to save scene?##OpenSceneAssetBrowser"); returnState != SaveReturnState::None)
		{
			if (returnState == SaveReturnState::Save)
			{
				Sandbox::Get().SaveScene();
			}

			Sandbox::Get().OpenScene(Volt::AssetManager::GetFilePathFromAssetHandle(mySceneToOpen));
			mySceneToOpen = Volt::Asset::Null();
		}

		return reload;
	}

	void AssetItem::PushID()
	{
		ImGui::PushID(static_cast<int32_t>(handle));
	}

	bool AssetItem::RenderRightClickPopup()
	{
		bool removed = false;
		if (!mySelectionManager->IsSelected(this))
		{
			mySelectionManager->DeselectAll();
			mySelectionManager->Select(this);
		}

		if (ImGui::MenuItem("Open Externally"))
		{
			FileSystem::OpenFileExternally(Volt::ProjectManager::GetDirectory() / path);
		}

		if (ImGui::MenuItem("Show In Explorer"))
		{
			FileSystem::ShowFileInExplorer(Volt::ProjectManager::GetDirectory() / path);
		}

		if (ImGui::MenuItem("Reload"))
		{
			Volt::AssetManager::Get().ReloadAsset(handle);
		}

		ImGui::Separator();

		bool extraItemsRendered = AssetBrowserUtilities::RenderAssetTypePopup(this);

		if (type == Volt::AssetType::MeshSource)
		{

		}

		if (extraItemsRendered)
		{
			ImGui::Separator();
		}

		if (ImGui::MenuItem("Rename"))
		{
			StartRename();
		}

		if (ImGui::MenuItem("Delete"))
		{
			UI::OpenModal("Delete Selected Files?");
		}

		if (ImGui::MenuItem("Checkout"))
		{
			VersionControl::Edit(Volt::AssetManager::Get().GetFilesystemPath(handle));
		}

		return removed;
	}

	bool AssetItem::Rename(const std::string& aNewName)
	{
		if (aNewName.empty()) { return false; }

		Volt::AssetManager::Get().RenameAsset(handle, aNewName);

		return true;
	}

	void AssetItem::Open()
	{
		if (!EditorLibrary::OpenAsset(Volt::AssetManager::Get().GetAssetRaw(handle)))
		{
			switch (type)
			{
				case Volt::AssetType::Animation:
					break;
				case Volt::AssetType::Skeleton:
					break;
				case Volt::AssetType::Texture:
					break;
				case Volt::AssetType::ShaderDefinition:
					break;
				case Volt::AssetType::ShaderSource:
					break;
				case Volt::AssetType::Scene:
				{
					UI::OpenModal("Do you want to save scene?##OpenSceneAssetBrowser");
					mySceneToOpen = handle;
					break;
				}
				case Volt::AssetType::Font:
					break;
				case Volt::AssetType::PhysicsMaterial:
					break;
				case Volt::AssetType::NavMesh:
					break;
				case Volt::AssetType::MonoScript:
				{
					if (!Volt::PremadeCommands::RunOpenVSFileCommand(UserSettingsManager::GetSettings().externalToolsSettings.customExternalScriptEditor, Volt::AssetManager::GetFilePathFromAssetHandle(handle)))
					{
						UI::Notify(NotificationType::Error, "Open file failed!", "External script editor is not valid!");
					}
					break;
				}
				default:
					break;
			}
		}
	}

	void AssetItem::DrawAdditionalHoverInfo()
	{
		auto extraData = EditorAssetRegistry::GetAssetBrowserPopupData(type, handle);
		for (auto& data : extraData)
		{
			DrawHoverInfo(data.first, data.second);
		}
		//file size
		{
			const auto fullPath = Volt::ProjectManager::GetAssetsDirectory() / std::filesystem::relative(path, "Assets\\");
			const uintmax_t fileSize = std::filesystem::file_size(fullPath);
			const std::string sizeStringWithMetricPrefix = Utility::ToStringWithMetricPrefixCharacterForBytes(fileSize);
			const std::string sizeStringWithSeparator = Utility::ToStringWithThousandSeparator(fileSize);

			DrawHoverInfo("Size", sizeStringWithMetricPrefix + " (" + sizeStringWithSeparator +" bytes)");
		}
	}

	RefPtr<Volt::RHI::Image> AssetItem::GetIcon() const
	{
		RefPtr<Volt::RHI::Image> icon = previewImage ? previewImage : nullptr;
		if (!icon && EditorResources::GetAssetIcon(type))
		{
			icon = EditorResources::GetAssetIcon(type)->GetImage();
		}

		if (type == Volt::AssetType::Texture)
		{
			if (EditorUtils::HasThumbnail(path))
			{
				auto image = Volt::AssetManager::GetAsset<Volt::Texture2D>(EditorUtils::GetThumbnailPathFromPath(path));
				if (image && image->IsValid())
				{
					icon = image->GetImage();
				}
			}
			else
			{
				//icon = EditorUtils::GenerateThumbnail(path)->GetImage();
			}
		}

		if (!icon)
		{
			icon = EditorResources::GetEditorIcon(EditorIcon::GenericFile)->GetImage();
		}

		return icon;
	}

	ImVec4 AssetItem::GetBackgroundColor() const
	{
		return GetBackgroundColorFromType(type);
	}

	std::string AssetItem::GetTypeName() const
	{
		return Volt::GetAssetTypeName(type);
	}

	void AssetItem::SetDragDropPayload()
	{
		//Data being copied
		ImGui::SetDragDropPayload("ASSET_BROWSER_ITEM", &handle, sizeof(Volt::AssetHandle), ImGuiCond_Once);

		// Viewport Drag drop
		GlobalEditorStates::dragStartedInAssetBrowser = true;
		GlobalEditorStates::dragAsset = handle;
		GlobalEditorStates::isDragging = true;
	}

	const ImVec4 AssetItem::GetBackgroundColorFromType(Volt::AssetType assetType) const
	{
		switch (assetType)
		{
			case Volt::AssetType::Mesh: return { 0.73f, 0.9f, 0.26f, 1.f };
			case Volt::AssetType::MeshSource: return { 0.43f, 0.9f, 0.26f, 1.f };
			case Volt::AssetType::NavMesh: return { 0.f, 0.80f, 0.98f, 1.f };
			case Volt::AssetType::Animation: return { 0.65f, 0.18f, 0.69f, 1.f };
			case Volt::AssetType::Skeleton: return { 1.f, 0.49f, 0.8f, 1.f };
			case Volt::AssetType::Texture: return { 0.9f, 0.26f, 0.27f, 1.f };
			case Volt::AssetType::Material: return { 0.26f, 0.35f, 0.9f, 1.f };
			case Volt::AssetType::ShaderDefinition: return { 0.26f, 0.6f, 0.9f, 1.f };
			case Volt::AssetType::ShaderSource: return { 0.26f, 0.72f, 0.9f, 1.f };
			case Volt::AssetType::Scene: return { 0.9f, 0.54f, 0.26f, 1.f };
			case Volt::AssetType::AnimatedCharacter: return { 0.9f, 0.25f, 0.49f, 1.f };
			case Volt::AssetType::Prefab: return { 0.25f, 0.93f, 0.92f, 1.f };
			case Volt::AssetType::ParticlePreset: return { 1.f, 0.62f, 0.f, 1.f };
			case Volt::AssetType::MonoScript: return { 0.f, 0.6f, 0.f, 1.f };
			case Volt::AssetType::BehaviorGraph: return { 0.75f, 0.04f, 0.83f, 1.f };
			case Volt::AssetType::MotionWeave: return { 0.74f, 0, 0.32f, 1.f };
		}

		return { 0.f, 0.f, 0.f, 1.f };
	}
}
