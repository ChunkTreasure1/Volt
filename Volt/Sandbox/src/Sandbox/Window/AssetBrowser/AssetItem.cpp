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

#include <Volt/Asset/AssetManager.h>
#include <Volt/Asset/Mesh/Material.h>
#include <Volt/Input/KeyCodes.h>
#include <Volt/Utility/PremadeCommands.h>

namespace AssetBrowser
{
	AssetItem::AssetItem(SelectionManager* selectionManager, const std::filesystem::path& path, float& thumbnailSize, MeshImportData& aMeshImportData, AssetData& aMeshToImportData)
		: Item(selectionManager, path), myThumbnailSize(thumbnailSize), meshImportData(aMeshImportData), meshToImportData(aMeshToImportData)
	{
		type = Volt::AssetManager::GetAssetTypeFromPath(path);
		handle = Volt::AssetManager::GetAssetHandleFromPath(path);
		if (handle == Volt::Asset::Null() && !Volt::AssetManager::Get().HasAssetMetaFile(path))
		{
			handle = Volt::AssetManager::Get().AddAssetToRegistry(path);
		}
	}

	bool AssetItem::Render()
	{
		bool reload = false;

		ImGui::PushID((uint32_t)handle);

		const Ref<Volt::Image2D> icon = GetIcon();
		const bool selected = mySelectionManager->IsSelected(this);

		const ImVec2 itemSize = AssetBrowserUtilities::GetBrowserItemSize(myThumbnailSize);
		const float itemPadding = AssetBrowserUtilities::GetBrowserItemPadding();
		const ImVec4 assetTypeColor = GetBackgroundColorFromType(type);

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

				constexpr float colorModifier = 0.6f;
				UI::ScopedColor innerChild{ ImGuiCol_ChildBg, { assetTypeColor.x * colorModifier, assetTypeColor.y * colorModifier, assetTypeColor.z * colorModifier, assetTypeColor.w } };
				ImGui::BeginChild("Image", { myThumbnailSize, myThumbnailSize + 4.f }, false, ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse);
				{
					// Icon
					{
						ImGui::Image(UI::GetTextureID(icon), { myThumbnailSize, myThumbnailSize });

						if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_SourceAllowNullID))
						{
							//Data being copied
							ImGui::SetDragDropPayload("ASSET_BROWSER_ITEM", &handle, sizeof(Volt::AssetHandle), ImGuiCond_Once);
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

							// Viewport Drag drop
							GlobalEditorStates::dragStartedInAssetBrowser = true;
							GlobalEditorStates::dragAsset = handle;
							GlobalEditorStates::isDragging = true;

							ImGui::EndDragDropSource();
						}
					}

					const ImVec2 maxChildPos = AssetBrowserUtilities::GetBrowserItemPos();

					// Bottom bar
					{
						const ImVec2 barMin = maxChildPos;
						const ImVec2 barMax = barMin + ImVec2{ 10.f, 4.f };

						ImVec4 color = assetTypeColor;
						color.w = 1.f;

						ImGui::GetWindowDrawList()->AddRectFilled(barMin, barMax, ImColor(assetTypeColor));
					}
				}

				ImGui::EndChild();

				if (RenderRightClickPopup())
				{
					reload = true;
					goto renderEnd;
				}

				UI::ShiftCursor(itemPadding / 2.f, 0.f);

				if (isRenaming)
				{
					const std::string renameId = "###renameId" + std::to_string(handle);
					ImGui::PushItemWidth(ImGui::GetContentRegionAvail().x);

					UI::ScopedColor background{ ImGuiCol_FrameBg, { 0.1f, 0.1f, 0.1f, 0.1f } };
					if (ImGui::InputTextString(renameId.c_str(), &currentRenamingName, ImGuiInputTextFlags_EnterReturnsTrue))
					{
						Volt::AssetManager::Get().RenameAsset(handle, currentRenamingName);
						switch (type)
						{
							case Volt::AssetType::Material:
								Volt::AssetManager::GetAsset<Volt::Material>(handle)->SetName(currentRenamingName);
								break;
						}

						reload = true;
						ImGui::PopItemWidth();

						goto renderEnd;
					}

					if (isRenaming != myLastRenaming)
					{
						const ImGuiID widgetId = ImGui::GetCurrentWindow()->GetID(renameId.c_str());
						ImGui::SetFocusID(widgetId, ImGui::GetCurrentWindow());
						ImGui::SetKeyboardFocusHere(-1);
					}

					if (!ImGui::IsItemFocused())
					{
						Volt::AssetManager::Get().RenameAsset(handle, currentRenamingName);

						isRenaming = false;
						reload = true;

						ImGui::PopItemWidth();

						goto renderEnd;
					}

					if (!ImGui::IsItemHovered() && ImGui::IsMouseClicked(ImGuiMouseButton_Left))
					{
						Volt::AssetManager::Get().RenameAsset(handle, currentRenamingName);
						isRenaming = false;
						reload = true;

						ImGui::PopItemWidth();
						goto renderEnd;
					}

					myLastRenaming = true;
					ImGui::PopItemWidth();
				}
				else
				{
					myLastRenaming = false;
					ImGui::TextWrapped("%s", path.stem().string().c_str());
				}

				static Volt::AssetHandle sceneToOpen = Volt::Asset::Null();

				if (ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left) && hovered)
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
							case Volt::AssetType::Shader:
								break;
							case Volt::AssetType::ShaderSource:
								break;
							case Volt::AssetType::Scene:
							{
								UI::OpenModal("Do you want to save scene?##OpenSceneAssetBrowser");
								sceneToOpen = handle;
								break;
							}
							case Volt::AssetType::Font:
								break;
							case Volt::AssetType::PhysicsMaterial:
								break;
							case Volt::AssetType::Video:
								break;
							case Volt::AssetType::NavMesh:
								break;
							case Volt::AssetType::GraphKey:
								break;
							case Volt::AssetType::MonoScript:
							{
								if (!Volt::PremadeCommands::RunOpenVSFileCommand(UserSettingsManager::GetSettings().externalToolsSettings.customExternalScriptEditor, Volt::AssetManager::GetPathFromAssetHandle(handle)))
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

				if (SaveReturnState returnState = EditorUtils::SaveFilePopup("Do you want to save scene?##OpenSceneAssetBrowser"); returnState != SaveReturnState::None)
				{
					if (returnState == SaveReturnState::Save)
					{
						Sandbox::Get().SaveScene();
					}

					Sandbox::Get().OpenScene(Volt::AssetManager::GetPathFromAssetHandle(sceneToOpen));
					sceneToOpen = Volt::Asset::Null();
				}

				if (ImGui::IsMouseDragging(ImGuiMouseButton_Left) && hovered && !mySelectionManager->IsSelected(this))
				{
					mySelectionManager->DeselectAll();
					mySelectionManager->Select(this);
				}

				const bool mouseDown = mySelectionManager->IsAnySelected() ? ImGui::IsMouseReleased(ImGuiMouseButton_Left) : ImGui::IsMouseClicked(ImGuiMouseButton_Left);
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

	bool AssetItem::RenderRightClickPopup()
	{
		bool removed = false;

		if (UI::BeginPopupItem(path.string(), ImGuiPopupFlags_MouseButtonRight))
		{
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

			if (extraItemsRendered)
			{
				ImGui::Separator();
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
				VersionControl::Edit(Volt::AssetManager::Get().GetFilesystemPath(handle));
			}

			UI::EndPopup();
		}

		return removed;
	}

	Ref<Volt::Image2D> AssetItem::GetIcon() const
	{
		Ref<Volt::Image2D> icon = previewImage ? previewImage : nullptr;
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

	const ImVec4 AssetItem::GetBackgroundColorFromType(Volt::AssetType type)
	{
		switch (type)
		{
			case Volt::AssetType::Mesh: return { 0.73f, 0.9f, 0.26f, 1.f };
			case Volt::AssetType::MeshSource: return { 0.43f, 0.9f, 0.26f, 1.f };
			case Volt::AssetType::NavMesh: return { 0.f, 0.80f, 0.98f, 1.f };
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
			case Volt::AssetType::MonoScript: return { 0.f, 0.6f, 0.f, 1.f };
			case Volt::AssetType::BehaviorGraph: return { 0.75f, 0.04f, 0.83f, 1.f };
			case Volt::AssetType::AnimationGraph: return { 0.82f, 0.72f, 0.2f, 1.f };
			default: return { 0.f, 0.f, 0.f, 1.f };
		}

		return { 0.f, 0.f, 0.f, 1.f };
	}
}
