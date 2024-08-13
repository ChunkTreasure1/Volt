#include "sbpch.h"
#include "DirectoryItem.h"

#include "Sandbox/Window/AssetBrowser/AssetBrowserSelectionManager.h"
#include "Sandbox/Window/AssetBrowser/AssetItem.h"

#include "Sandbox/Utility/AssetBrowserUtilities.h"
#include "Sandbox/Utility/EditorResources.h"
#include "Sandbox/VersionControl/VersionControl.h"

#include <Volt/Utility/UIUtility.h>
#include <InputModule/KeyCodes.h>
#include <Volt/Rendering/Texture/Texture2D.h>

namespace AssetBrowser
{
	DirectoryItem::DirectoryItem(SelectionManager* selectionManager, const std::filesystem::path& path)
		: Item(selectionManager, path)
	{
		isDirectory = true;
	}

	bool DirectoryItem::Render()
	{
		bool reload = Item::Render();

		if (void* ptr = UI::DragDropTarget("ASSET_BROWSER_ITEM"))// TODO: DIRECTORY UNIQUE CODE
		{
			for (const auto& item : mySelectionManager->GetSelectedItems())
			{
				if (item->isDirectory && item != this)
				{
					const std::filesystem::path newPath = path / item->path.stem();
					Volt::AssetManager::Get().MoveFullFolder(item->path, newPath);
					FileSystem::MoveFolder(Volt::ProjectManager::GetDirectory() / item->path, newPath);
				}
			}

			for (const auto& item : mySelectionManager->GetSelectedItems())
			{
				if (!item->isDirectory && item != this && FileSystem::Exists(Volt::ProjectManager::GetDirectory() / item->path))
				{

					Volt::AssetManager::Get().MoveAsset(Volt::AssetManager::GetAssetHandleFromFilePath(item->path), path);
				}
			}

			reload = true;
		}

		if (void* ptr = UI::DragDropTarget("ASSET_BROWSER_FOLDER"))// TODO: DIRECTORY UNIQUE CODE
		{
			for (const auto& item : mySelectionManager->GetSelectedItems())
			{
				if (item->isDirectory && item != this)
				{
					const std::filesystem::path newPath = path / item->path.stem();
					Volt::AssetManager::Get().MoveFullFolder(item->path, newPath);
					FileSystem::MoveFolder(Volt::ProjectManager::GetDirectory() / item->path, newPath);
				}
			}

			for (const auto& item : mySelectionManager->GetSelectedItems())
			{
				if (!item->isDirectory && item != this && FileSystem::Exists(Volt::ProjectManager::GetDirectory() / item->path))
				{
					// Check for thumbnail PNG
					if (Volt::AssetManager::Get().GetAssetTypeFromPath(item->path) == Volt::AssetType::Texture)
					{
						const std::filesystem::path thumbnailPath = item->path.parent_path() / (item->path.stem().string() + ".vtthumb.png");
						if (FileSystem::Exists(thumbnailPath))
						{
							FileSystem::Move(Volt::ProjectManager::GetDirectory() / thumbnailPath, path);
						}
					}

					Volt::AssetManager::Get().MoveAsset(Volt::AssetManager::GetAssetHandleFromFilePath(item->path), path);
				}
			}

			reload = true;
		}

		return reload;
	}

	void DirectoryItem::PushID()
	{
		ImGui::PushID(path.string().c_str());
	}

	RefPtr<Volt::RHI::Image2D> DirectoryItem::GetIcon() const
	{
		return EditorResources::GetEditorIcon(EditorIcon::Directory)->GetImage();
	}

	ImVec4 DirectoryItem::GetBackgroundColor() const
	{
		return { 0.2f, 0.2f, 0.2f, 1.f };
	}

	std::string DirectoryItem::GetTypeName() const
	{
		return "Directory";
	}

	void DirectoryItem::SetDragDropPayload()
	{
		//Data being copied
		ImGui::SetDragDropPayload("ASSET_BROWSER_FOLDER", path.wstring().c_str(), path.wstring().size() * sizeof(wchar_t), ImGuiCond_Once);
	}

	bool DirectoryItem::RenderRightClickPopup()
	{
		bool removed = false;


		if (!mySelectionManager->IsSelected(this))
		{
			mySelectionManager->DeselectAll();
			mySelectionManager->Select(this);
		}

		if (ImGui::MenuItem("Show in Explorer"))
		{
			FileSystem::ShowFileInExplorer(Volt::ProjectManager::GetDirectory() / path);
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
			VersionControl::Edit(path);
		}


		return removed;
	}

	bool DirectoryItem::Rename(const std::string& newName)
	{
		if (newName.empty()) { return false; }

		const std::filesystem::path newDir = path.parent_path() / myCurrentRenamingName;
		RecursivlyRenameAssets(this, newDir);

		FileSystem::Rename(Volt::ProjectManager::GetDirectory() / path, myCurrentRenamingName);
		return true;
	}

	void DirectoryItem::Open()
	{
		isNext = true;
	}

	void DirectoryItem::RecursivlyRenameAssets(DirectoryItem* directory, const std::filesystem::path& targetDirectory)
	{
		for (const auto& asset : directory->assets)
		{
			const std::filesystem::path newPath = targetDirectory / asset->path.filename();
			Volt::AssetManager::Get().RenameAssetFolder(asset->handle, newPath);
		}

		for (const auto& dir : directory->subDirectories)
		{
			const std::filesystem::path newPath = targetDirectory / dir->path.stem();
			RecursivlyRenameAssets(dir.get(), newPath);
		}
	}
}
