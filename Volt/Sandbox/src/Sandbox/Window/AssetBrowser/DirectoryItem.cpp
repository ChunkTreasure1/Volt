#include "sbpch.h"
#include "DirectoryItem.h"

#include "Sandbox/Window/AssetBrowser/AssetBrowserSelectionManager.h"
#include "Sandbox/Window/AssetBrowser/AssetItem.h"

#include "Sandbox/Utility/AssetBrowserUtilities.h"
#include "Sandbox/Utility/EditorResources.h"
#include "Sandbox/VersionControl/VersionControl.h"

#include <Volt/Utility/UIUtility.h>
#include <Volt/Input/KeyCodes.h>

namespace AssetBrowser
{
	DirectoryItem::DirectoryItem(SelectionManager* selectionManager, const std::filesystem::path& path, float& thumbnailSize)
		: Item(selectionManager, path), myThumbnailSize(thumbnailSize)
	{
		isDirectory = true;
	}

	bool DirectoryItem::Render()
	{
		bool reload = false;

		ImGui::PushID(path.string().c_str());

		const bool selected = mySelectionManager->IsSelected(this);

		const ImVec2 itemSize = AssetBrowserUtilities::GetBrowserItemSize(myThumbnailSize);
		const float itemPadding = AssetBrowserUtilities::GetBrowserItemPadding();

		UI::ScopedColor outerChild{ ImGuiCol_ChildBg, { 0.f } };
		ImGui::BeginChild("hoverWindow", itemSize, false, ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse);
		{
			const bool hovered = ImGui::IsWindowHovered(ImGuiHoveredFlags_ChildWindows);
			const ImVec4 bgColor = AssetBrowserUtilities::GetBackgroundColor(hovered, selected);

			UI::ScopedColor middleChild{ ImGuiCol_ChildBg, { bgColor.x, bgColor.y, bgColor.z, bgColor.w } };
			UI::ScopedStyleFloat rounding{ ImGuiStyleVar_ChildRounding, 2.f };
			ImGui::BeginChild("item", itemSize, false, ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse);
			{
				UI::ShiftCursor(itemPadding / 2.f, itemPadding / 2.f);
				UI::ScopedColor innerChild{ ImGuiCol_ChildBg, { 0.2f, 0.2f, 0.2f, 1.f } };
				ImGui::BeginChild("Image", { myThumbnailSize, myThumbnailSize }, false, ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse);
				{
					// Icon
					{
						UI::ShiftCursor(itemPadding / 2.f, itemPadding / 2.f);
						ImGui::Image(UI::GetTextureID(EditorResources::GetEditorIcon(EditorIcon::Directory)), { myThumbnailSize - itemPadding, myThumbnailSize - itemPadding });

						if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_SourceAllowNullID))
						{
							//Data being copied
							ImGui::SetDragDropPayload("ASSET_BROWSER_FOLDER", path.wstring().c_str(), path.wstring().size() * sizeof(wchar_t), ImGuiCond_Once);
							ImGui::TextUnformatted("Move:");

							constexpr uint32_t maxShownPaths = 3;
							for (uint32_t i = 0; const auto & selected : mySelectionManager->GetSelectedItems())
							{
								if (i == maxShownPaths)
								{
									ImGui::TextUnformatted("...");
									break;
								}
								ImGui::TextUnformatted(selected->path.string().c_str());
								i++;
							}

							ImGui::EndDragDropSource();
						}

						const ImVec2 maxChildPos = AssetBrowserUtilities::GetBrowserItemPos();

						// Bottom bar
						{
							const ImVec4 typeColor = { 0.07f, 0.07f, 0.07f, 1.f };
							const ImVec2 barMin = maxChildPos;
							const ImVec2 barMax = barMin + ImVec2{ 10.f, 4.f };

							ImVec4 color = ImColor(typeColor);
							color.w = 1.f;

							ImGui::GetWindowDrawList()->AddRectFilled(barMin, barMax, ImColor(typeColor));
						}

						if (void* ptr = UI::DragDropTarget("ASSET_BROWSER_ITEM"))
						{
							for (const auto& item : mySelectionManager->GetSelectedItems())
							{
								if (item->isDirectory && item != this)
								{
									const std::filesystem::path newPath = path / item->path.stem();
									Volt::AssetManager::Get().MoveFolder(item->path, newPath);
									FileSystem::MoveFolder(Volt::ProjectManager::GetDirectory() / item->path, newPath);
								}
							}

							for (const auto& item : mySelectionManager->GetSelectedItems())
							{
								if (!item->isDirectory && item != this && FileSystem::Exists(Volt::ProjectManager::GetDirectory() / item->path))
								{
									Volt::AssetManager::Get().MoveAsset(Volt::AssetManager::Get().GetAssetHandleFromPath(item->path), path);
								}
							}

							reload = true;
							ImGui::EndChild();
							goto renderEnd;
						}

						if (void* ptr = UI::DragDropTarget("ASSET_BROWSER_FOLDER"))
						{
							for (const auto& item : mySelectionManager->GetSelectedItems())
							{
								if (item->isDirectory && item != this)
								{
									const std::filesystem::path newPath = path / item->path.stem();
									Volt::AssetManager::Get().MoveFolder(item->path, newPath);
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

									Volt::AssetManager::Get().MoveAsset(Volt::AssetManager::Get().GetAssetHandleFromPath(item->path), path);
								}
							}

							reload = true;
							ImGui::EndChild();
							goto renderEnd;
						}

						if (RenderRightClickPopup())
						{
							reload = true;
							ImGui::EndChild();
							goto renderEnd;
						}
					}
				}
				ImGui::EndChild();

				UI::ShiftCursor(itemPadding / 2.f, 0.f);
				if (isRenaming)
				{
					const std::string renameId = "###renameId" + path.stem().string();
					ImGui::PushItemWidth(ImGui::GetContentRegionAvail().x);

					UI::ScopedColor background{ ImGuiCol_FrameBg, { 0.1f, 0.1f, 0.1f, 0.1f } };
					if (ImGui::InputTextString(renameId.c_str(), &currentRenamingName, ImGuiInputTextFlags_EnterReturnsTrue))
					{
						if (Rename(currentRenamingName))
						{
							isRenaming = false;
							reload = true;
							ImGui::PopItemWidth();
							goto renderEnd;
						}
					}

					if (isRenaming != myLastRenaming)
					{
						const ImGuiID widgetId = ImGui::GetCurrentWindow()->GetID(renameId.c_str());
						ImGui::SetFocusID(widgetId, ImGui::GetCurrentWindow());
						ImGui::SetKeyboardFocusHere(-1);
					}

					if (!ImGui::IsItemFocused())
					{
						if (Rename(currentRenamingName))
						{
							isRenaming = false;
							reload = true;

							ImGui::PopItemWidth();

							goto renderEnd;
						}
					}

					if (!ImGui::IsItemHovered() && ImGui::IsMouseClicked(ImGuiMouseButton_Left))
					{
						if (Rename(currentRenamingName))
						{
							isRenaming = false;
							reload = true;

							ImGui::PopItemWidth();
							goto renderEnd;
						}
					}

					myLastRenaming = true;
				}
				else
				{
					myLastRenaming = false;
					ImGui::TextWrapped("%s", path.stem().string().c_str());
				}

				if (ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left) && hovered)
				{
					isNext = true;
				}

				if (ImGui::IsMouseDragging(ImGuiMouseButton_Left) && hovered && !mySelectionManager->IsSelected(this))
				{
					mySelectionManager->DeselectAll();
					mySelectionManager->Select(this);
				}

				const bool mouseDown = (!mySelectionManager->IsAnySelected()) ? ImGui::IsMouseClicked(ImGuiMouseButton_Left) : ImGui::IsMouseReleased(ImGuiMouseButton_Left);
				if (mouseDown && hovered)
				{
					if (!Volt::Input::IsKeyDown(VT_KEY_LEFT_CONTROL))
					{
						mySelectionManager->DeselectAll();
					}

					if (mySelectionManager->IsSelected(this))
					{
						mySelectionManager->Deselect(this);
					}
					else
					{
						mySelectionManager->Select(this);
					}
				}
			}

		renderEnd:
			ImGui::EndChild();
		}
		ImGui::EndChild();
		ImGui::PopID();

		return reload;
	}

	bool DirectoryItem::RenderRightClickPopup()
	{
		bool removed = false;

		if (UI::BeginPopupItem(path.string(), ImGuiPopupFlags_MouseButtonRight))
		{
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
				isRenaming = true;
				currentRenamingName = path.stem().string();
			}

			if (ImGui::MenuItem("Delete"))
			{
				UI::OpenModal("Delete Selected Files?");
			}

			if (ImGui::MenuItem("Checkout"))
			{
				VersionControl::Edit(path);
			}

			UI::EndPopup();
		}

		return removed;
	}

	bool DirectoryItem::Rename(const std::string& newName)
	{
		if (newName.empty()) { return false; }

		const std::filesystem::path newDir = path.parent_path() / currentRenamingName;
		RecursivlyRenameAssets(this, newDir);

		FileSystem::Rename(Volt::ProjectManager::GetDirectory() / path, currentRenamingName);
		return true;
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
