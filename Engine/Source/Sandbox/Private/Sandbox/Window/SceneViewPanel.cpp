#include "sbpch.h"

#include "Sandbox/Window/SceneViewPanel.h"
#include "Sandbox/Utility/SelectionManager.h"
#include "Sandbox/Utility/EditorResources.h"
#include "Sandbox/Utility/EditorUtilities.h"
#include "Sandbox/Utility/Theme.h"
#include "Sandbox/Sandbox.h"

#include <Volt/Asset/Prefab.h>
#include <Volt/Asset/ParticlePreset.h>
#include <Volt/Asset/Mesh/Mesh.h>
#include <Volt/Utility/UIUtility.h>
#include <Volt/Components/CoreComponents.h>
#include <Volt/Components/RenderingComponents.h>
#include <Volt/Components/LightComponents.h>
#include <Volt/Vision/VisionComponents.h>
#include <Volt/ImGui/FontAwesome.h>

#include <InputModule/Input.h>
#include <InputModule/InputCodes.h>

#include <AssetSystem/AssetManager.h>
#include <WindowModule/WindowManager.h>
#include <WindowModule/Window.h>

#include <CoreUtilities/FileSystem.h>

namespace Utility
{
	inline static void SetRowColor(const glm::vec4& color)
	{
		for (int32_t i = 0; i < ImGui::TableGetColumnCount(); i++)
		{
			ImGui::TableSetBgColor(ImGuiTableBgTarget_CellBg, ImColor{ color.x, color.y, color.z, color.w }, i);
		}
	}
}

SceneViewPanel::SceneViewPanel(Ref<Volt::Scene>& scene, const std::string& id)
	: EditorWindow("Scene View", false, id), m_scene(scene)
{
	Open();
	m_windowFlags = ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse;

	RegisterListener<Volt::KeyPressedEvent>(VT_BIND_EVENT_FN(SceneViewPanel::OnKeyPressedEvent));
}

void SceneViewPanel::UpdateMainContent()
{
	VT_PROFILE_FUNCTION();

	//if (myTitle.contains("#"))
	//{
	//	SelectionManager::SetSelectionKey(myId);
	//}

	EditorUtils::SearchBar(m_searchQuery, m_hasSearchQuery);

	ImGui::PushStyleColor(ImGuiCol_ChildBg, { 0.16f, 0.16f, 0.16f, 1.f });

	ImGui::BeginChild("MainView", ImGui::GetContentRegionAvail());
	{
		UI::ScopedColor button(ImGuiCol_Button, { 0.f, 0.f, 0.f, 0.f });
		UI::ScopedColor hovered(ImGuiCol_ButtonHovered, { 0.3f, 0.305f, 0.31f, 0.5f });
		UI::ScopedColor active(ImGuiCol_ButtonActive, { 0.5f, 0.505f, 0.51f, 0.5f });
		UI::ScopedColor tableRow(ImGuiCol_TableRowBg, { 0.18f, 0.18f, 0.18f, 1.f });
		UI::ScopedStyleFloat2 padd{ ImGuiStyleVar_FramePadding, { 4.f, 4.f } };
		UI::ScopedStyleFloat2 padd1{ ImGuiStyleVar_CellPadding, { 4.f, 0.f } };

		const auto flags = ImGuiTableFlags_Reorderable | ImGuiTableFlags_ScrollY | ImGuiTableFlags_NoPadInnerX;

		const uint32_t columnCount = m_showEntityUUIDs ? 3 : 2;
		if (ImGui::BeginTable("entitiesTable", columnCount, flags, ImGui::GetContentRegionAvail()))
		{
			ImGui::TableSetupColumn("Name", ImGuiTableColumnFlags_WidthStretch);

			if (m_showEntityUUIDs)
			{
				ImGui::TableSetupColumn("UUID", ImGuiTableColumnFlags_WidthStretch);
			}

			ImGui::TableSetupColumn("Modifiers", ImGuiTableColumnFlags_WidthFixed, 70.f);

			// Headers
			{
				ImGui::PushStyleColor(ImGuiCol_HeaderHovered, ImVec4(0.4f, 0.67f, 1.000f, 1.000f));
				ImGui::PushStyleColor(ImGuiCol_HeaderActive, ImVec4(0.4f, 0.67f, 1.000f, 1.000f));
				ImGui::PushStyleColor(ImGuiCol_TableRowBg, ImVec4(0.2f, 0.2f, 0.2f, 1.000f));

				ImGui::TableSetupScrollFreeze(ImGui::TableGetColumnCount(), 1);
				ImGui::TableNextRow(ImGuiTableRowFlags_Headers, 22.f);

				for (uint32_t i = 0; i < columnCount; i++)
				{
					ImGui::TableSetColumnIndex(i);

					UI::ShiftCursor(3.f, 3.f);
					ImGui::TableSetBgColor(ImGuiTableBgTarget_CellBg, ImColor{ 0.2f, 0.2f, 0.2f, 1.000f }, i);
					ImGui::TableHeader(ImGui::TableGetColumnName());
					UI::ShiftCursor(3.f, -3.f);
				}

				ImGui::PopStyleColor(3);
			}

			if (m_rebuildDrawList)
			{
				RebuildEntityDrawList();
			}

			// Draw Entities
			{
				VT_PROFILE_SCOPE("Draw Entities");

				for (const auto& id : m_entityDrawList)
				{
					Volt::Entity entity = m_scene->GetEntityFromID(id);
					DrawEntity(entity, m_searchQuery);
				}
			}
			ImGui::EndTable();

			m_rebuildDrawList = true;

			const ImRect windowRect = ImGui::GetCurrentWindow()->Rect();

			if (ImGui::IsMouseHoveringRect(windowRect.Min, windowRect.Max, false) && !ImGui::IsAnyItemHovered())
			{
				if (ImGui::IsMouseReleased(ImGuiMouseButton_Left))
				{
					SelectionManager::DeselectAll();
				}
				else if (ImGui::IsMouseClicked(ImGuiMouseButton_Right))
				{
					UI::OpenPopup("MainRightClickMenu");
				}
			}

			if (ImGui::BeginDragDropTargetCustom(windowRect, ImGui::GetCurrentWindow()->ID))
			{
				const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("scene_entity_hierarchy", ImGuiDragDropFlags_AcceptNoDrawDefaultRect);

				// If there is a payload, assume it's all the selected entities
				if (payload)
				{
					const size_t count = payload->DataSize / sizeof(Volt::EntityID);
					Vector<Ref<ParentChildData>> undoData;

					for (size_t i = 0; i < count; i++)
					{
						Volt::EntityID id = *(((Volt::EntityID*)payload->Data) + i);
						Volt::Entity child = m_scene->GetEntityFromID(id);

						Ref<ParentChildData> data = CreateRef<ParentChildData>();
						data->myParent = child.GetParent();
						data->myChild = child;
						undoData.push_back(data);

						EditorUtils::MarkEntityAsEdited(child);
						EditorUtils::MarkEntityAsEdited(child.GetParent());

						m_scene->UnparentEntity(child);
					}

					Ref<ParentingCommand> command = CreateRef<ParentingCommand>(undoData, ParentingAction::Unparent);
					EditorCommandStack::PushUndo(command);
				}

				ImGui::EndDragDropTarget();
			}
		}
	}

	ImGui::EndChild();

	ImGui::PopStyleColor();

	if (void* ptr = UI::DragDropTarget({ "ASSET_BROWSER_ITEM" }))
	{
		const Volt::AssetHandle handle = *(const Volt::AssetHandle*)ptr;
		const AssetType type = Volt::AssetManager::GetAssetTypeFromHandle(handle);

		if (type == AssetTypes::Mesh)
		{
			Volt::Entity newEntity = m_scene->CreateEntity();

			auto& meshComp = newEntity.AddComponent<Volt::MeshComponent>();
			auto mesh = Volt::AssetManager::GetAsset<Volt::Mesh>(handle);
			if (mesh)
			{
				meshComp.handle = mesh->handle;
			}

			newEntity.GetComponent<Volt::TagComponent>().tag = Volt::AssetManager::Get().GetFilePathFromAssetHandle(handle).stem().string();
		}
		else if (type == AssetTypes::ParticlePreset)
		{
			Volt::Entity newEntity = m_scene->CreateEntity();

			auto& particleEmitter = newEntity.AddComponent<Volt::ParticleEmitterComponent>();
			auto preset = Volt::AssetManager::GetAsset<Volt::ParticlePreset>(handle);
			if (preset)
			{
				particleEmitter.preset = preset->handle;
			}

			newEntity.GetComponent<Volt::TagComponent>().tag = Volt::AssetManager::Get().GetFilePathFromAssetHandle(handle).stem().string();
		}
		else if (type == AssetTypes::Prefab)
		{
			auto prefab = Volt::AssetManager::GetAsset<Volt::Prefab>(handle);
			if (prefab && prefab->IsValid())
			{
				Volt::Entity prefabEntity = prefab->Instantiate(m_scene);

				Ref<ObjectStateCommand> command = CreateRef<ObjectStateCommand>(prefabEntity, ObjectStateAction::Create);
				EditorCommandStack::GetInstance().PushUndo(command);
			}
		}
	}

	DrawMainRightClickPopup();
	//SelectionManager::ResetSelectionKey();
}

void SceneViewPanel::HighlightEntity(Volt::Entity entity)
{
	m_scrollToEntity = entity.GetID();
}

void RecursiveUnpackPrefab(Ref<Volt::Scene> scene, Volt::EntityID id)
{
	Volt::Entity entity = scene->GetEntityFromID(id);
	EditorUtils::MarkEntityAsEdited(entity);

	if (entity.HasComponent<Volt::PrefabComponent>())
	{
		entity.RemoveComponent<Volt::PrefabComponent>();
	}

	for (auto& childId : entity.GetComponent<Volt::RelationshipComponent>().children)
	{
		Volt::Entity child = scene->GetEntityFromID(childId);

		if (child.HasComponent<Volt::PrefabComponent>())
		{
			child.RemoveComponent<Volt::PrefabComponent>();
		}

		RecursiveUnpackPrefab(scene, childId);
	}
};

bool SceneViewPanel::OnKeyPressedEvent(Volt::KeyPressedEvent& e)
{
	if (!IsHovered() || ImGui::IsAnyItemActive())
	{
		return false;
	}

	switch (e.GetKeyCode())
	{
		case Volt::InputCode::Backspace:
		case Volt::InputCode::Delete:
		{
			Vector<Volt::Entity> entitiesToRemove;

			auto selection = SelectionManager::GetSelectedEntities();
			for (const auto& selectedEntity : selection)
			{
				Volt::Entity tempEnt = m_scene->GetEntityFromID(selectedEntity);
				entitiesToRemove.push_back(tempEnt);

				SelectionManager::Deselect(tempEnt.GetID());
				SelectionManager::GetFirstSelectedRow() = -1;
				SelectionManager::GetLastSelectedRow() = -1;
			}

			Ref<ObjectStateCommand> command = CreateRef<ObjectStateCommand>(entitiesToRemove, ObjectStateAction::Delete);
			EditorCommandStack::GetInstance().PushUndo(command);

			bool shouldUpdateNavMesh = false;
			for (const auto& i : entitiesToRemove)
			{
				if (!shouldUpdateNavMesh && Sandbox::Get().CheckForUpdateNavMesh(i))
				{
					shouldUpdateNavMesh = true;
				}
				m_scene->DestroyEntity(i);
			}

			if (shouldUpdateNavMesh)
			{
				Sandbox::Get().BakeNavMesh();
			}

			break;
		}
	}

	return false;
}

void SceneViewPanel::DrawEntity(Volt::Entity entity, const std::string& filter)
{
	bool entityDeleted = false;

	std::string entityName = "Null";

	Volt::Entity parent = Volt::Entity::Null();
	Vector<Volt::EntityID> children;

	if (entity.HasComponent<Volt::RelationshipComponent>())
	{
		auto& relComp = entity.GetComponent<Volt::RelationshipComponent>();

		parent = m_scene->GetEntityFromID(relComp.parent);
		children = relComp.children;
	}

	if (entity.HasComponent<Volt::TagComponent>())
	{
		entityName = entity.GetComponent<Volt::TagComponent>().tag;
	}

	const bool hasMatchingParent = SearchRecursivelyParent(entity, filter, 10);
	const bool hasMatchingChild = SearchRecursively(entity, filter, 10);
	const bool matchesQuery = MatchesQuery(entityName, filter);
	const bool hasId = std::to_string(static_cast<uint32_t>(entity.GetID())) == filter;
	const bool hasComponent = HasComponent(entity, filter);
	const bool isVisionCamera = entity.HasComponent<Volt::VisionCameraComponent>();

	if (!matchesQuery && !hasId && !hasMatchingChild && !hasMatchingParent && !hasComponent)
	{
		return;
	}

	if (isVisionCamera)
	{
		entityName = VT_ICON_FA_CAMERA + std::string(" ") + entityName;
	}

	const float rowHeight = 17.f;
	const float rowPadding = 4.f;

	auto* window = ImGui::GetCurrentWindow();
	window->DC.CurrLineSize.y = rowHeight;

	ImGui::TableNextRow(0, rowHeight);
	ImGui::TableNextColumn();
	window->DC.CurrLineTextBaseOffset = 3.f;

	const ImVec2 rowAreaMin = ImGui::TableGetCellBgRect(ImGui::GetCurrentTable(), 0).Min;
	const ImVec2 rowAreaMax = { ImGui::TableGetCellBgRect(ImGui::GetCurrentTable(), ImGui::TableGetColumnCount() - 2).Max.x - 20.f, rowAreaMin.y + rowHeight + rowPadding * 2.f };

	const bool isSelected = SelectionManager::IsSelected(entity.GetID());

	const std::string entityStrId = entityName + "###" + std::to_string(static_cast<uint32_t>(entity.GetID()));

	ImGui::PushClipRect(rowAreaMin, rowAreaMax, false);
	bool isRowClicked = false;
	bool isRowHovered = ImGui::IsMouseHoveringRect(rowAreaMin, rowAreaMax, true);

	if (isRowHovered)
	{
		if (ImGui::IsMouseReleased(ImGuiMouseButton_Left))
		{
			isRowClicked = true;
		}
	}

	const bool wasRowRightClicked = ImGui::IsMouseReleased(ImGuiMouseButton_Right);
	ImGui::PopClipRect();

	bool open = false;
	const bool isPrefab = entity.HasComponent<Volt::PrefabComponent>();

	if (isPrefab)
	{
		auto& prefabComp = entity.GetComponent<Volt::PrefabComponent>();
		Ref<Volt::Prefab> prefabAsset = Volt::AssetManager::QueueAsset<Volt::Prefab>(prefabComp.prefabAsset);

		bool hasValidLink = false;

		if (prefabAsset && prefabAsset->IsValid())
		{
			hasValidLink = prefabAsset->IsEntityValidInPrefab(entity);
		}

		if (hasValidLink)
		{
			ImGui::PushStyleColor(ImGuiCol_Text, { 56.f / 255.f, 156.f / 255.f, 1.f, 1.f });
		}
		else
		{
			ImGui::PushStyleColor(ImGuiCol_Text, { 255.f / 255.f, 255.f / 255.f, 0.f, 1.f });
		}
	}

	ImGuiTreeNodeFlags treeFlags = isSelected ? ImGuiTreeNodeFlags_Selected : 0;
	treeFlags |= ImGuiTreeNodeFlags_SpanAllColumns | ImGuiTreeNodeFlags_FramePadding | ImGuiTreeNodeFlags_AllowItemOverlap | ImGuiTreeNodeFlags_SpanAvailWidth;

	if (hasMatchingChild)
	{
		ImGui::SetNextItemOpen(true);
		treeFlags |= ImGuiTreeNodeFlags_DefaultOpen;
	}

	auto isAnyDescendantSelected = [&](Volt::Entity ent, auto isAnyDescendantSelected)
	{
		if (!ent)
		{
			return false;
		}

		if (SelectionManager::IsSelected(ent.GetID()))
		{
			return true;
		}

		if (!ent.GetChildren().empty())
		{
			for (auto& child : ent.GetChildren())
			{
				if (isAnyDescendantSelected(child, isAnyDescendantSelected))
				{
					return true;
				}
			}
		}

		return false;
	};

	const bool descendantSelected = isAnyDescendantSelected(entity, isAnyDescendantSelected);
	const glm::vec4 selectedColor = IsFocused() ? EditorTheme::ItemSelectedFocused : EditorTheme::ItemSelected;

	if (isRowHovered)
	{
		if (isSelected)
		{
			Utility::SetRowColor(selectedColor);
		}
		else
		{
			Utility::SetRowColor(EditorTheme::ItemHovered);
		}
	}
	else if (!isSelected && descendantSelected)
	{
		ImGui::SetNextItemOpen(true);
		Utility::SetRowColor(EditorTheme::ItemChildActive);
	}
	else if (isSelected)
	{
		Utility::SetRowColor(selectedColor);
	}
	else if (hasMatchingChild)
	{
		Utility::SetRowColor(EditorTheme::ItemChildActive);
	}

	glm::vec2 offset = { 25.f, 6.f };
	if (!parent.IsValid())
	{
		offset.x -= 20.f;
	}

	if (matchesQuery)
	{
		UI::RenderMatchingTextBackground(filter, entityName, EditorTheme::MatchingTextBackground, offset);
	}

	if (hasId)
	{
		UI::RenderMatchingTextBackground(entityName, entityName, EditorTheme::MatchingTextBackground, offset);
	}

	if (m_scrollToEntity != Volt::Entity::NullID() && m_scrollToEntity == entity.GetID())
	{
		m_scrollToEntity = Volt::Entity::NullID();
		ImGui::SetScrollHereY();
	}

	ImGui::PushStyleColor(ImGuiCol_Header, ImVec4{ 0.f, 0.f, 0.f, 0.f });
	ImGui::PushStyleColor(ImGuiCol_HeaderHovered, ImVec4{ 0.f, 0.f, 0.f, 0.f });
	ImGui::PushStyleColor(ImGuiCol_HeaderActive, ImVec4{ 0.f, 0.f, 0.f, 0.f });

	UI::PushFont(FontType::Regular_16);

	if (!children.empty())
	{
		treeFlags |= ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_FramePadding;
		open = ImGui::TreeNodeEx(entityStrId.c_str(), treeFlags);
	}
	else
	{
		treeFlags |= ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen;
		if (!parent.IsValid())
		{
			UI::ShiftCursor(-20.f, 0.f);
		}
		ImGui::TreeNodeEx(entityStrId.c_str(), treeFlags);
	}

	UI::PopFont();

	ImGui::PopStyleColor(3);

	ImGui::SetItemAllowOverlap();

	const int32_t rowIndex = ImGui::TableGetRowIndex();

	if (rowIndex >= SelectionManager::GetFirstSelectedRow() && rowIndex <= SelectionManager::GetLastSelectedRow() && !SelectionManager::IsSelected(entity.GetID()))
	{
		SelectionManager::Select(entity.GetID());
	}

	if (isPrefab)
	{
		ImGui::PopStyleColor();
	}

	if (isRowClicked)
	{
		if (!wasRowRightClicked)
		{
			if (!Volt::Input::IsKeyDown(Volt::InputCode::LeftControl) && !Volt::Input::IsKeyDown(Volt::InputCode::LeftShift))
			{
				SelectionManager::DeselectAll();
				SelectionManager::Select(entity.GetID());

				SelectionManager::GetFirstSelectedRow() = rowIndex;
				SelectionManager::GetLastSelectedRow() = -1;
			}
			else if (Volt::Input::IsKeyDown(Volt::InputCode::LeftShift) && SelectionManager::GetSelectedCount() > 0)
			{
				if (rowIndex < SelectionManager::GetFirstSelectedRow())
				{
					SelectionManager::GetLastSelectedRow() = SelectionManager::GetFirstSelectedRow();
					SelectionManager::GetFirstSelectedRow() = rowIndex;
				}
				else
				{
					SelectionManager::GetLastSelectedRow() = rowIndex;
				}
			}
			else
			{
				if (isSelected)
				{
					SelectionManager::Deselect(entity.GetID());
				}
				else
				{
					SelectionManager::Select(entity.GetID());
				}
			}
		}

	}

	// Drag & Drop
	//------------

	if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_SourceAllowNullID))
	{
		if (!SelectionManager::IsSelected(entity.GetID()))
		{
			ImGui::TextUnformatted(entity.GetTag().c_str());

			const Volt::EntityID entityId = entity.GetID();
			ImGui::SetDragDropPayload("scene_entity_hierarchy", &entityId, sizeof(Volt::EntityID));
			ImGui::EndDragDropSource();
		}
		else
		{
			Vector<Volt::EntityID> selectedEntities = SelectionManager::GetSelectedEntities();

			for (uint32_t i = 0; const auto & id : selectedEntities)
			{
				if (i >= 5)
				{
					break;
				}

				Volt::Entity ent = m_scene->GetEntityFromID(id);
				ImGui::TextUnformatted(ent.GetTag().c_str());
				i++;
			}

			ImGui::SetDragDropPayload("scene_entity_hierarchy", selectedEntities.data(), selectedEntities.size() * sizeof(Volt::EntityID));
			ImGui::EndDragDropSource();
		}
	}

	if (ImGui::BeginDragDropTarget())
	{
		const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("scene_entity_hierarchy");

		// If there is a payload, assume it's all the selected entities
		if (payload)
		{
			const size_t count = payload->DataSize / sizeof(Volt::EntityID);
			Vector<Ref<ParentChildData>> undoData;

			for (size_t i = 0; i < count; i++)
			{
				Volt::EntityID id = *(((Volt::EntityID*)payload->Data) + i);
				Volt::Entity newParent = entity;
				Volt::Entity child = m_scene->GetEntityFromID(id);

				Ref<ParentChildData> data = CreateRef<ParentChildData>();
				data->myParent = newParent;
				data->myChild = child;
				undoData.push_back(data);

				m_scene->ParentEntity(newParent, child);

				EditorUtils::MarkEntityAsEdited(child);
				EditorUtils::MarkEntityAsEdited(newParent);
			}

			Ref<ParentingCommand> command = CreateRef<ParentingCommand>(undoData, ParentingAction::Parent);
			EditorCommandStack::PushUndo(command);
		}

		ImGui::EndDragDropTarget();
	}

	std::string entityMenuId = "RightClickEntity" + entity.ToString();
	if (ImGui::BeginPopupContextItem(entityMenuId.c_str(), ImGuiPopupFlags_MouseButtonRight))
	{
		if (!SelectionManager::IsSelected(entity.GetID()))
		{
			SelectionManager::DeselectAll();
			SelectionManager::Select(entity.GetID());
		}

		if (entity.HasComponent<Volt::PrefabComponent>())
		{
			const auto& prefabComp = entity.GetComponent<Volt::PrefabComponent>();

			Ref<Volt::Prefab> prefabAsset = Volt::AssetManager::QueueAsset<Volt::Prefab>(prefabComp.prefabAsset);
			if (prefabAsset && prefabAsset->IsValid())
			{
				const std::string menuId = "Update Prefab Entity##" + entity.ToString();
				if (ImGui::MenuItem(menuId.c_str()))
				{
					const auto prefabPath = Volt::AssetManager::GetFilePathFromAssetHandle(prefabComp.prefabAsset);
					if (!FileSystem::IsWriteable(prefabPath))
					{
						UI::Notify(NotificationType::Error, "Unable to update prefab!", std::format("The prefab file {0} is not writeable!", prefabPath.string()));
					}
					else
					{
						prefabAsset->UpdateEntityInPrefab(entity);
						UpdatePrefabsInScene(prefabAsset, entity);

						Volt::AssetManager::SaveAsset(prefabAsset);
						UI::Notify(NotificationType::Success, "Prefab updated!", std::format("The prefab file {0} has been updated!", prefabPath.string()));
					}
				}
			}
		}

		if (!entity.HasComponent<Volt::PrefabComponent>())
		{
			const std::string menuId = "Create Prefab##" + entity.ToString();
			if (ImGui::MenuItem(menuId.c_str()))
			{
				CreatePrefabAndSetupEntities(entity);
			}
		}
		else
		{
			const std::string unpackId = "Unpack Prefab##" + entity.ToString();
			if (ImGui::MenuItem(unpackId.c_str()))
			{
				RecursiveUnpackPrefab(m_scene, entity.GetID());
			}
		}

		const std::string deleteId = "Delete##" + entity.ToString();
		if (ImGui::MenuItem(deleteId.c_str()))
		{
			entityDeleted = true;
		}

		const std::string copyId = "Copy ID##" + entity.ToString();
		if (ImGui::MenuItem(copyId.c_str()))
		{
			Volt::WindowManager::Get().GetMainWindow().SetClipboard(entity.ToString());
		}

		ImGui::EndPopup();
	}

	if (entity.HasComponent<Volt::RelationshipComponent>())
	{
		parent = entity.GetParent();
	}

	if (entityDeleted)
	{
		Vector<Volt::Entity> entitiesToRemove;

		auto selection = SelectionManager::GetSelectedEntities();
		for (const auto& selectedEntity : selection)
		{
			Volt::Entity tempEnt = m_scene->GetEntityFromID(selectedEntity);
			entitiesToRemove.push_back(tempEnt);

			SelectionManager::Deselect(tempEnt.GetID());
		}

		Ref<ObjectStateCommand> command = CreateRef<ObjectStateCommand>(entitiesToRemove, ObjectStateAction::Delete);
		EditorCommandStack::GetInstance().PushUndo(command);

		for (const auto& i : entitiesToRemove)
		{
			m_scene->DestroyEntity(i);
		}
	}

	ImGui::TableNextColumn();

	// UUIDs
	if (m_showEntityUUIDs)
	{
		const std::string text = std::to_string(entity.GetID());
		ImGui::TextUnformatted(text.c_str());
		ImGui::TableNextColumn();
	}

	// Modifiers
	{
		const float imageSize = 21.f;
		UI::ShiftCursor(0.f, 2.f);

		if (entity.HasComponent<Volt::TransformComponent>())
		{
			auto& transformComponent = entity.GetComponent<Volt::TransformComponent>();

			Ref<Volt::Texture2D> visibleIcon = transformComponent.visible ? EditorResources::GetEditorIcon(EditorIcon::Visible) : EditorResources::GetEditorIcon(EditorIcon::Hidden);
			std::string visibleId = "##visible" + entity.ToString();
			if (UI::ImageButton(visibleId, UI::GetTextureID(visibleIcon), { imageSize, imageSize }, { 0.f, 0.f }, { 1.f, 1.f }, 0))
			{
				auto recursiveSetVisible = [scene = m_scene](Volt::Entity entity, bool visible, auto recursiveSetVisible) -> bool
				{
					if (entity.HasComponent<Volt::TransformComponent>())
					{
						entity.GetComponent<Volt::TransformComponent>().visible = visible;
					}

					for (const auto& e : entity.GetChildren())
					{
						recursiveSetVisible(e, visible, recursiveSetVisible);
					}

					EditorUtils::MarkEntityAsEdited(entity);
					return false;
				};

				const auto newVal = !transformComponent.visible;
				if (!SelectionManager::IsSelected(entity.GetID()))
				{
					recursiveSetVisible(entity, newVal, recursiveSetVisible);
				}
				else
				{
					for (const auto& e : SelectionManager::GetSelectedEntities())
					{
						recursiveSetVisible(m_scene->GetEntityFromID(e), newVal, recursiveSetVisible);
					}
				}
			}

			ImGui::SameLine();

			Ref<Volt::Texture2D> lockedIcon = transformComponent.locked ? EditorResources::GetEditorIcon(EditorIcon::Locked) : EditorResources::GetEditorIcon(EditorIcon::Unlocked);
			std::string lockedId = "##locked" + entity.ToString();
			if (UI::ImageButton(lockedId, UI::GetTextureID(lockedIcon), { imageSize, imageSize }, { 0.f, 0.f }, { 1.f, 1.f }, 0))
			{
				const auto newVal = !transformComponent.locked;
				if (!SelectionManager::IsSelected(entity.GetID()))
				{
					transformComponent.locked = newVal;
				}
				else
				{
					for (const auto& e : SelectionManager::GetSelectedEntities())
					{
						Volt::Entity tempEnt = m_scene->GetEntityFromID(e);

						auto& eTransformComponent = tempEnt.GetComponent<Volt::TransformComponent>();
						eTransformComponent.locked = newVal;
					}
				}

			}
		}
	}

	ImGui::TableNextColumn();

	if (open)
	{
		for (const auto& child : children)
		{
			auto childEnt = m_scene->GetEntityFromID(child);
			if (!childEnt)
			{
				// #TODO_Ivar: Maybe display invalid entity somehow?
				continue;
			}

			DrawEntity(childEnt, filter);
		}

		ImGui::TreePop();
	}
}

void SceneViewPanel::CreatePrefabAndSetupEntities(Volt::Entity entity)
{
	if (entity.HasComponent<Volt::PrefabComponent>())
	{
		UI::Notify(NotificationType::Error, "Unable to create prefab!", "Cannot create prefab of existing prefab!");
		return;
	}

	const auto& tagComp = entity.GetComponent<Volt::TagComponent>();

	Ref<Volt::Prefab> prefab = CreateRef<Volt::Prefab>(entity);

	const std::filesystem::path basePath = "Assets/Prefabs/";

	std::string path = basePath.string() + tagComp.tag + ".vtprefab";
	if (!std::filesystem::exists(Volt::ProjectManager::GetProjectDirectory() / basePath))
	{
		std::filesystem::create_directories(Volt::ProjectManager::GetProjectDirectory() / basePath);
	}

	path.erase(std::remove_if(path.begin(), path.end(), ::isspace), path.end());
	Volt::AssetManager::SaveAssetAs(prefab, path);

	EditorUtils::MarkEntityAndChildrenAsEdited(entity);
}

void SceneViewPanel::UpdatePrefabsInScene(Ref<Volt::Prefab> prefab, Volt::Entity srcEntity)
{
	Ref<Volt::Prefab> prefabAsset = Volt::AssetManager::GetAsset<Volt::Prefab>(srcEntity.GetComponent<Volt::PrefabComponent>().prefabAsset);
	if (!prefabAsset || !prefabAsset->IsValid())
	{
		return;
	}

	m_scene->ForEachWithComponents<const Volt::PrefabComponent, const Volt::IDComponent>([&](const entt::entity id, const Volt::PrefabComponent& prefabComp, const Volt::IDComponent& idComponent)
	{
		if (idComponent.id == srcEntity.GetID())
		{
			return;
		}

		if (prefabComp.prefabAsset != prefabAsset->handle)
		{
			return;
		}

		if (prefabComp.prefabEntity != srcEntity.GetComponent<Volt::PrefabComponent>().prefabEntity)
		{
			return;
		}

		auto entity = Volt::Entity{ id, m_scene };
		prefabAsset->UpdateEntityInScene(entity);

		EditorUtils::MarkEntityAsEdited(entity);
	});

	if (prefabAsset->IsReference(srcEntity))
	{
		m_scene->ForEachWithComponents<const Volt::PrefabComponent, const Volt::IDComponent>([&](const entt::entity id, const Volt::PrefabComponent& prefabComp, const Volt::IDComponent& idComponent)
		{
			if (idComponent.id == srcEntity.GetID())
			{
				return;
			}

			const auto& prefabRefData = prefabAsset->GetReferenceData(srcEntity);

			if (prefabComp.prefabAsset != prefabRefData.prefabAsset || prefabComp.prefabEntity != prefabRefData.prefabReferenceEntity)
			{
				return;
			}

			Ref<Volt::Prefab> prefabRefAsset = Volt::AssetManager::GetAsset<Volt::Prefab>(prefabRefData.prefabAsset);
			if (!prefabRefAsset || !prefabRefAsset->IsValid())
			{
				return;
			}

			auto entity = Volt::Entity{ id, m_scene };
			prefabRefAsset->UpdateEntityInScene(entity);

			EditorUtils::MarkEntityAsEdited(entity);
		});
	}
}

bool SceneViewPanel::SearchRecursively(Volt::Entity entity, const std::string& filter, uint32_t maxSearchDepth, uint32_t currentDepth)
{
	VT_PROFILE_FUNCTION();

	if (filter.empty())
	{
		return false;
	}

	for (auto child : entity.GetChildren())
	{
		if (child.HasComponent<Volt::TagComponent>())
		{
			std::string t = child.GetComponent<Volt::TagComponent>().tag;
			if (MatchesQuery(t, filter) || child.ToString() == filter || HasComponent(child, filter))
			{
				return true;
			}
		}

		const bool found = SearchRecursively(child, filter, maxSearchDepth, currentDepth + 1);
		if (found)
		{
			return true;
		}
	}

	return false;
}

bool SceneViewPanel::SearchRecursivelyParent(Volt::Entity entity, const std::string& filter, uint32_t maxSearchDepth, uint32_t currentDepth /*= 0*/)
{
	VT_PROFILE_FUNCTION();

	if (filter.empty())
	{
		return false;
	}

	if (entity.HasComponent<Volt::RelationshipComponent>())
	{
		auto parent = entity.GetParent();
		if (!parent)
		{
			return false;
		}

		if (parent.HasComponent<Volt::TagComponent>())
		{
			std::string t = parent.GetComponent<Volt::TagComponent>().tag;
			if (MatchesQuery(t, filter) || parent.ToString() == filter || HasComponent(parent, filter))
			{
				return true;
			}
		}

		const bool found = SearchRecursively(parent, filter, maxSearchDepth, currentDepth + 1);
		if (found)
		{
			return true;
		}
	}

	return false;
}

bool SceneViewPanel::MatchesQuery(const std::string& text, const std::string& filter)
{
	VT_PROFILE_FUNCTION();

	if (filter.empty())
	{
		return true;
	}

	std::string query = Utility::ToLower(filter);
	const std::string lowerText = Utility::ToLower(text);

	query.push_back(' ');
	Vector<std::string> queries;

	for (auto next = query.find_first_of(' '); next != std::string::npos; next = query.find_first_of(' '))
	{
		std::string split = query.substr(0, next);
		query = query.substr(next + 1);
		queries.emplace_back(split);
	}

	for (const auto& q : queries)
	{
		if (Utility::StringContains(lowerText, q))
		{
			return true;
		}
	}

	return false;
}

bool SceneViewPanel::HasComponent(Volt::Entity entity, const std::string& filter)
{
	if (filter.empty())
	{
		return false;
	}

	if (!(filter.at(0) == ';'))
	{
		return false;
	}

	const std::string compSearchString = filter.substr(1);
	return entity.HasComponent(compSearchString);
}

void SceneViewPanel::DrawMainRightClickPopup()
{
	if (UI::BeginPopup("MainRightClickMenu", ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse))
	{
		if (ImGui::MenuItem(VT_ICON_FA_PLUS " Create Empty Entity"))
		{
			auto ent = m_scene->CreateEntity();

			Ref<ObjectStateCommand> command = CreateRef<ObjectStateCommand>(ent, ObjectStateAction::Create);
			EditorCommandStack::GetInstance().PushUndo(command);
			SelectionManager::DeselectAll();
			SelectionManager::Select(ent.GetID());
		}

		if (ImGui::BeginMenu(VT_ICON_FA_PLUS " New"))
		{
			if (ImGui::BeginMenu(VT_ICON_FA_CUBES " Primitives"))
			{
				if (ImGui::MenuItem(VT_ICON_FA_CUBE " Cube"))
				{
					auto ent = m_scene->CreateEntity();
					auto& meshComp = ent.AddComponent<Volt::MeshComponent>();
					meshComp.handle = Volt::AssetManager::GetAssetHandleFromFilePath("Engine/Meshes/Primitives/SM_Cube.vtasset");
					Volt::MeshComponent::OnMemberChanged(Volt::MeshComponent::MeshEntity(m_scene->GetEntityHelperFromEntityID(ent.GetID())));

					ent.SetTag("New Cube");

					SelectionManager::DeselectAll();
					SelectionManager::Select(ent.GetID());
				}

				if (ImGui::MenuItem(VT_ICON_FA_CUBE " Capsule"))
				{
					auto ent = m_scene->CreateEntity();
					auto& meshComp = ent.AddComponent<Volt::MeshComponent>();
					meshComp.handle = Volt::AssetManager::GetAssetHandleFromFilePath("Engine/Meshes/Primitives/SM_Capsule.vtasset");
					Volt::MeshComponent::OnMemberChanged(Volt::MeshComponent::MeshEntity(m_scene->GetEntityHelperFromEntityID(ent.GetID())));

					ent.SetTag("New Capsule");

					SelectionManager::DeselectAll();
					SelectionManager::Select(ent.GetID());
				}

				if (ImGui::MenuItem(VT_ICON_FA_CUBE " Cone"))
				{
					auto ent = m_scene->CreateEntity();
					auto& meshComp = ent.AddComponent<Volt::MeshComponent>();
					meshComp.handle = Volt::AssetManager::GetAssetHandleFromFilePath("Engine/Meshes/Primitives/SM_Cone.vtasset");
					Volt::MeshComponent::OnMemberChanged(Volt::MeshComponent::MeshEntity(m_scene->GetEntityHelperFromEntityID(ent.GetID())));

					ent.SetTag("New Cone");

					SelectionManager::DeselectAll();
					SelectionManager::Select(ent.GetID());
				}

				if (ImGui::MenuItem(VT_ICON_FA_CUBE " Cylinder"))
				{
					auto ent = m_scene->CreateEntity();
					auto& meshComp = ent.AddComponent<Volt::MeshComponent>();
					meshComp.handle = Volt::AssetManager::GetAssetHandleFromFilePath("Engine/Meshes/Primitives/SM_Cylinder.vtasset");
					Volt::MeshComponent::OnMemberChanged(Volt::MeshComponent::MeshEntity(m_scene->GetEntityHelperFromEntityID(ent.GetID())));

					ent.SetTag("New Cylinder");

					SelectionManager::DeselectAll();
					SelectionManager::Select(ent.GetID());
				}

				if (ImGui::MenuItem(VT_ICON_FA_CUBE " Sphere"))
				{
					auto ent = m_scene->CreateEntity();
					auto& meshComp = ent.AddComponent<Volt::MeshComponent>();
					meshComp.handle = Volt::AssetManager::GetAssetHandleFromFilePath("Engine/Meshes/Primitives/SM_Sphere.vtasset");
					Volt::MeshComponent::OnMemberChanged(Volt::MeshComponent::MeshEntity(m_scene->GetEntityHelperFromEntityID(ent.GetID())));
					ent.SetTag("New Sphere");

					SelectionManager::DeselectAll();
					SelectionManager::Select(ent.GetID());
				}

				if (ImGui::MenuItem(VT_ICON_FA_CUBE " Plane"))
				{
					auto ent = m_scene->CreateEntity();
					auto& meshComp = ent.AddComponent<Volt::MeshComponent>();
					meshComp.handle = Volt::AssetManager::GetAssetHandleFromFilePath("Engine/Meshes/Primitives/SM_Plane.vtasset");
					Volt::MeshComponent::OnMemberChanged(Volt::MeshComponent::MeshEntity(m_scene->GetEntityHelperFromEntityID(ent.GetID())));

					ent.SetTag("New Plane");

					SelectionManager::DeselectAll();
					SelectionManager::Select(ent.GetID());
				}

				ImGui::EndMenu();
			}

			if (ImGui::BeginMenu(VT_ICON_FA_IMAGE " Environment"))
			{
				if (ImGui::MenuItem("Decal"))
				{
					auto ent = m_scene->CreateEntity();
					ent.AddComponent<Volt::DecalComponent>();
					ent.SetTag("New Decal");

					SelectionManager::DeselectAll();
					SelectionManager::Select(ent.GetID());
				}

				ImGui::EndMenu();
			}

			if (ImGui::BeginMenu(VT_ICON_FA_LIGHTBULB " Lights"))
			{
				if (ImGui::MenuItem(VT_ICON_FA_LIGHTBULB " Point Light"))
				{
					auto ent = m_scene->CreateEntity();
					ent.AddComponent<Volt::PointLightComponent>();
					ent.SetTag("New Point Light");

					SelectionManager::DeselectAll();
					SelectionManager::Select(ent.GetID());
				}

				if (ImGui::MenuItem(VT_ICON_FA_LIGHTBULB " Spot Light"))
				{
					auto ent = m_scene->CreateEntity();
					ent.AddComponent<Volt::SpotLightComponent>();
					ent.SetTag("New Spot Light");

					SelectionManager::DeselectAll();
					SelectionManager::Select(ent.GetID());
				}

				if (ImGui::MenuItem(VT_ICON_FA_SUN " Directional Light"))
				{
					auto ent = m_scene->CreateEntity();
					ent.AddComponent<Volt::DirectionalLightComponent>();
					ent.SetTag("New Directional Light");

					SelectionManager::DeselectAll();
					SelectionManager::Select(ent.GetID());
				}

				if (ImGui::MenuItem(VT_ICON_FA_LIGHTBULB " Skylight"))
				{
					auto ent = m_scene->CreateEntity();
					ent.AddComponent<Volt::SkylightComponent>();
					ent.SetTag("New Skylight");

					SelectionManager::DeselectAll();
					SelectionManager::Select(ent.GetID());
				}

				if (ImGui::MenuItem(VT_ICON_FA_LIGHTBULB " Sphere Light"))
				{
					auto ent = m_scene->CreateEntity();
					ent.AddComponent<Volt::SphereLightComponent>();
					ent.SetTag("New Sphere Light");

					SelectionManager::DeselectAll();
					SelectionManager::Select(ent.GetID());
				}

				if (ImGui::MenuItem(VT_ICON_FA_LIGHTBULB " Rectangle Light"))
				{
					auto ent = m_scene->CreateEntity();
					ent.AddComponent<Volt::RectangleLightComponent>();
					ent.SetTag("New Rectangle Light");

					SelectionManager::DeselectAll();
					SelectionManager::Select(ent.GetID());
				}

				ImGui::EndMenu();
			}

			if (ImGui::MenuItem(VT_ICON_FA_CAMERA " Camera"))
			{
				auto ent = m_scene->CreateEntity();
				ent.AddComponent<Volt::CameraComponent>();
				ent.SetTag("New Camera");

				SelectionManager::DeselectAll();
				SelectionManager::Select(ent.GetID());
			}

			ImGui::EndMenu();
		}

		ImGui::MenuItem("Show UUIDS", nullptr, &m_showEntityUUIDs);

		ImGui::EndPopup();
	}
}

void SceneViewPanel::RebuildEntityDrawList()
{
	m_entityDrawList.clear();
	m_entityToImGuiID.clear();
	m_rebuildDrawList = true;

	m_scene->ForEachWithComponents<const Volt::CommonComponent>([&](entt::entity id, const Volt::CommonComponent& dataComp)
	{
		Volt::Entity entity{ id, m_scene.get() };
		if (entity.GetParent())
		{
			return;
		}

		RebuildEntityDrawListRecursive(entity, m_searchQuery);
	});
}

void SceneViewPanel::RebuildEntityDrawListRecursive(Volt::Entity entity, const std::string& filter)
{
	const bool hasMatchingChild = SearchRecursively(entity, filter, 10);
	const bool matchesQuery = MatchesQuery(entity.GetTag(), filter);
	const bool hasComponent = HasComponent(entity, filter);
	const bool hasId = entity.ToString() == filter;

	if (!matchesQuery && !hasId && !hasComponent && !hasMatchingChild)
	{
		return;
	}

	m_entityDrawList.emplace_back(entity.GetID());

	if (entity.GetChildren().empty())
	{
		return;
	}

	const bool isVisionCamera = entity.HasComponent<Volt::VisionCameraComponent>();
	std::string entityName = entity.GetTag();

	if (isVisionCamera)
	{
		entityName = VT_ICON_FA_CAMERA + std::string(" ") + entityName;
	}

	const std::string entityStrId = entityName + "###" + entity.ToString();

	auto imGuiID = ImGui::GetID(entityStrId.c_str());
	m_entityToImGuiID[entity.GetID()] = imGuiID;

	bool isOpen = ImGui::TreeNodeBehaviorIsOpen(imGuiID);

	if (!isOpen)
	{
		return;
	}

	//for (const auto& child : entity.GetChilden())
	//{
	//	RebuildEntityDrawListRecursive(child.GetId(), filter);
	//}
}
