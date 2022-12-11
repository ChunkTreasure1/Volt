#include "sbpch.h"
#include "DirectoryItem.h"

#include "Sandbox/Window/AssetBrowser/AssetBrowserSelectionManager.h"

#include "Sandbox/Utility/AssetBrowserUtilities.h"
#include "Sandbox/Utility/EditorIconLibrary.h"

#include <Volt/Utility/UIUtility.h>
#include <Volt/Input/KeyCodes.h>

namespace AssetBrowser
{
	DirectoryItem::DirectoryItem(SelectionManager* selectionManager, const std::filesystem::path& path, float& thumbnailSize)
		: Item(selectionManager, path), myThumbnailSize(thumbnailSize)
	{}

	bool DirectoryItem::Render()
	{
		bool reload = false;
		const bool wasRenamingAtStart = isRenaming;

		ImGui::PushID(path.string().c_str());

		const bool selected = mySelectionManager->IsSelected(this);

		const ImVec2 itemSize = AssetBrowserUtilities::GetBrowserItemSize();
		const ImVec2 minChildPos = AssetBrowserUtilities::GetBrowserItemMinPos();
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
				UI::ShiftCursor(itemPadding / 2.f, 10.f);
				UI::ScopedColor innerChild{ ImGuiCol_ChildBg, { 0.2f, 0.2f, 0.2f, 1.f } };
				ImGui::BeginChild("Image", { myThumbnailSize, myThumbnailSize }, false, ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse);
				{
					// Right bar
					{
						const ImVec4 typeColor = { 0.1f, 0.1f, 0.1f, 1.f };
						const ImVec2 barMin = minChildPos;
						const ImVec2 barMax = barMin + ImVec2{ 10.f, myThumbnailSize + itemPadding };

						ImGui::GetWindowDrawList()->AddRectFilled(barMin, barMax, ImColor(typeColor));
					}

					// Icon
					{
						UI::ShiftCursor(itemPadding / 2.f, itemPadding / 2.f);
						ImGui::Image(UI::GetTextureID(EditorIconLibrary::GetIcon(EditorIcon::Directory)), { myThumbnailSize - itemPadding, myThumbnailSize - itemPadding });

						if (void* ptr = UI::DragDropTarget("ASSET_BROWSER_ITEM"))
						{
							Volt::AssetHandle handle = *(Volt::AssetHandle*)ptr;
							Volt::AssetManager::Get().MoveAsset(handle, path);

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
						const std::filesystem::path newDir = path.parent_path() / currentRenamingName;

						isRenaming = false;
						reload = true;
						ImGui::PopItemWidth();
						goto renderEnd;
					}

					if (isRenaming != wasRenamingAtStart)
					{
						const ImGuiID widgetId = ImGui::GetCurrentWindow()->GetID(renameId.c_str());
						ImGui::SetFocusID(widgetId, ImGui::GetCurrentWindow());
						ImGui::SetKeyboardFocusHere(-1);
					}

					if (!ImGui::IsItemFocused())
					{
						isRenaming = false;
						reload = true;

						ImGui::PopItemWidth();

						goto renderEnd;
					}

					if (!ImGui::IsItemHovered() && ImGui::IsMouseClicked(ImGuiMouseButton_Left))
					{
						const std::filesystem::path newDir = path.parent_path() / currentRenamingName;

						isRenaming = false;
						reload = true;

						ImGui::PopItemWidth();
						goto renderEnd;
					}
				}
				else
				{
					ImGui::TextWrapped("%s", path.stem().string().c_str());
				}

				if (ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left) && hovered)
				{
					isNext = true;
				}

				if (ImGui::IsMouseClicked(ImGuiMouseButton_Left) && hovered)
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
				FileSystem::ShowDirectoryInExplorer(path);
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

			UI::EndPopup();
		}

		return removed;
	}
}