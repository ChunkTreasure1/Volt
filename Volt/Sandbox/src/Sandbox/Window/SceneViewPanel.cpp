#include "sbpch.h"
#include "SceneViewPanel.h"

#include "Sandbox/Utility/SelectionManager.h"
#include "Sandbox/Utility/EditorResources.h"
#include "Sandbox/Utility/EditorUtilities.h"

#include <Volt/Asset/Prefab.h>
#include <Volt/Input/Input.h>
#include <Volt/Input/KeyCodes.h>
#include <Volt/Asset/ParticlePreset.h>

#include <Volt/Utility/UIUtility.h>
#include <Volt/Utility/StringUtility.h>

#include <Volt/Components/Components.h>
#include <Volt/Components/LightComponents.h>

SceneViewPanel::SceneViewPanel(Ref<Volt::Scene>& scene)
	: EditorWindow("Scene View"), myScene(scene)
{
	myWindowFlags = ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse;
	myIsOpen = true;
}

void SceneViewPanel::UpdateMainContent()
{
	VT_PROFILE_FUNCTION();

	EditorUtils::SearchBar(mySearchQuery, myHasSearchQuery);

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

		constexpr uint32_t columnCount = 2;
		if (ImGui::BeginTable("entitiesTable", columnCount, flags, ImGui::GetContentRegionAvail()))
		{
			ImGui::TableSetupColumn("Name", ImGuiTableColumnFlags_WidthStretch);
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

			const ImVec4 selectedColor = myIsFocused ? ImVec4{ 0.f, 0.44f, 1.f, 1.f } : ImVec4{ 0.3f, 0.54f, 0.8f, 1.f };
			ImGui::PushStyleColor(ImGuiCol_Header, selectedColor);

			// Draw Entities
			{
				VT_PROFILE_SCOPE("Draw Entities")
					for (const auto& entity : myScene->GetRegistry().GetAllEntities())
					{
						if (myScene->GetRegistry().HasComponent<Volt::RelationshipComponent>(entity))
						{
							if (myScene->GetRegistry().GetComponent<Volt::RelationshipComponent>(entity).Parent == Wire::NullID)
							{
								DrawEntity(entity, mySearchQuery);
							}
						}
					}
			}

			ImGui::PopStyleColor();

			ImGui::EndTable();

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
					const size_t count = payload->DataSize / sizeof(Wire::EntityId);
					std::vector<Ref<ParentChildData>> undoData;

					for (size_t i = 0; i < count; i++)
					{
						Wire::EntityId id = *(((Wire::EntityId*)payload->Data) + i);
						Volt::Entity child(id, myScene.get());

						Ref<ParentChildData> data = CreateRef<ParentChildData>();
						data->myParent = child.GetParent();
						data->myChild = child;
						undoData.push_back(data);

						myScene->UnparentEntity(child);
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
		const Volt::AssetType type = Volt::AssetManager::Get().GetAssetTypeFromHandle(handle);

		switch (type)
		{
			case Volt::AssetType::Mesh:
			{
				Volt::Entity newEntity = myScene->CreateEntity();

				auto& meshComp = newEntity.AddComponent<Volt::MeshComponent>();
				auto mesh = Volt::AssetManager::GetAsset<Volt::Mesh>(handle);
				if (mesh)
				{
					meshComp.handle = mesh->handle;
				}

				newEntity.GetComponent<Volt::TagComponent>().tag = Volt::AssetManager::Get().GetPathFromAssetHandle(handle).stem().string();

				break;
			}

			case Volt::AssetType::ParticlePreset:
			{
				Volt::Entity newEntity = myScene->CreateEntity();

				auto& particleEmitter = newEntity.AddComponent<Volt::ParticleEmitterComponent>();
				auto preset = Volt::AssetManager::GetAsset<Volt::ParticlePreset>(handle);
				if (preset)
				{
					particleEmitter.preset = preset->handle;
				}

				newEntity.GetComponent<Volt::TagComponent>().tag = Volt::AssetManager::Get().GetPathFromAssetHandle(handle).stem().string();

				break;
			}

			case Volt::AssetType::Prefab:
			{
				auto prefab = Volt::AssetManager::GetAsset<Volt::Prefab>(handle);
				if (!prefab || !prefab->IsValid())
				{
					break;
				}

				prefab->Instantiate(myScene->GetRegistry());

				break;
			}
		}
	}

	DrawMainRightClickPopup();
}

void SceneViewPanel::DrawEntity(Wire::EntityId entity, const std::string& filter)
{
	VT_PROFILE_FUNCTION();

	bool entityDeleted = false;

	auto& registry = myScene->GetRegistry();
	std::string entityName = "Null";

	Wire::EntityId parentId = Wire::NullID;
	std::vector<Wire::EntityId> children;

	if (myScene->GetRegistry().HasComponent<Volt::RelationshipComponent>(entity))
	{
		parentId = myScene->GetRegistry().GetComponent<Volt::RelationshipComponent>(entity).Parent;
		children = myScene->GetRegistry().GetComponent<Volt::RelationshipComponent>(entity).Children;
	}

	if (registry.HasComponent<Volt::TagComponent>(entity))
	{
		entityName = registry.GetComponent<Volt::TagComponent>(entity).tag;
	}

	constexpr uint32_t maxSearchDepth = 10;
	const bool hasMatchingChild = SearchRecursivly(entity, filter, 10);
	const bool matchesQuery = MatchesQuery(entityName, filter) && !filter.empty();

	if (!MatchesQuery(entityName, filter) && !hasMatchingChild)
	{
		return;
	}

	const float edgeOffset = 4.f;
	const float rowHeight = 21.f;

	auto* window = ImGui::GetCurrentWindow();
	window->DC.CurrLineSize.y = rowHeight;

	ImGui::TableNextRow(0, 21.f);
	ImGui::TableNextColumn();
	window->DC.CurrLineTextBaseOffset = 3.f;

	const ImVec2 rowAreaMin = ImGui::TableGetCellBgRect(ImGui::GetCurrentTable(), 0).Min;
	const ImVec2 rowAreaMax = { ImGui::TableGetCellBgRect(ImGui::GetCurrentTable(), ImGui::TableGetColumnCount() - 2).Max.x - 20.f, rowAreaMin.y + rowHeight };

	const bool isSelected = SelectionManager::IsSelected(entity);

	ImGui::PushClipRect(rowAreaMin, rowAreaMax, false);
	bool isRowClicked = false;
	if (ImGui::IsMouseHoveringRect(rowAreaMin, rowAreaMax, false) && ImGui::IsMouseReleased(ImGuiMouseButton_Left))
	{
		isRowClicked = true;
	}

	const bool wasRowRightClicked = ImGui::IsMouseReleased(ImGuiMouseButton_Right);
	ImGui::PopClipRect();

	std::string entityId = entityName + "###" + std::to_string(entity);
	bool open = false;
	bool isPrefab = myScene->GetRegistry().HasComponent<Volt::PrefabComponent>(entity);

	if (isPrefab)
	{
		ImGui::PushStyleColor(ImGuiCol_Text, { 56.f / 255.f, 156.f / 255.f, 1.f, 1.f });
	}

	ImGuiTreeNodeFlags treeFlags = isSelected ? ImGuiTreeNodeFlags_Selected : 0;
	treeFlags |= ImGuiTreeNodeFlags_SpanAllColumns | ImGuiTreeNodeFlags_FramePadding | ImGuiTreeNodeFlags_AllowItemOverlap | ImGuiTreeNodeFlags_SpanAvailWidth;

	if (hasMatchingChild)
	{
		ImGui::SetNextItemOpen(true);
		treeFlags |= ImGuiTreeNodeFlags_DefaultOpen;
	}

	auto isAnyDescendantSelected = [&](Wire::EntityId id, auto isAnyDescendantSelected)
	{
		if (SelectionManager::IsSelected(id))
		{
			return true;
		}

		Volt::Entity ent{ id, myScene.get() };
		if (!ent.GetChilden().empty())
		{
			for (auto& child : ent.GetChilden())
			{
				if (isAnyDescendantSelected(child.GetId(), isAnyDescendantSelected))
				{
					return true;
				}
			}
		}

		return false;
	};

	const bool descendantSelected = isAnyDescendantSelected(entity, isAnyDescendantSelected);

	if (!isSelected && descendantSelected)
	{
		ImGui::SetNextItemOpen(true);
		ImGui::PushStyleColor(ImGuiCol_Header, { 0.17f, 0.196f, 0.227f, 1.f });

		treeFlags |= ImGuiTreeNodeFlags_Selected;
	}

	if (hasMatchingChild)
	{
		ImGui::PushStyleColor(ImGuiCol_Header, { 0.17f, 0.196f, 0.227f, 1.f });
		treeFlags |= ImGuiTreeNodeFlags_Selected;
	}

	if (matchesQuery)
	{
		ImGui::PushStyleColor(ImGuiCol_Header, ImVec4{ 0.3f, 0.54f, 0.8f, 1.f });
		treeFlags |= ImGuiTreeNodeFlags_Selected;
	}

	if (!children.empty())
	{
		treeFlags |= ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_FramePadding;
		open = ImGui::TreeNodeEx(entityId.c_str(), treeFlags);
	}
	else
	{
		treeFlags |= ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen;
		if (parentId == Wire::NullID)
		{
			UI::ShiftCursor(-20.f, 0.f);
		}
		ImGui::TreeNodeEx(entityId.c_str(), treeFlags);
	}

	ImGui::SetItemAllowOverlap();

	if (matchesQuery)
	{
		ImGui::PopStyleColor();
	}

	if (hasMatchingChild)
	{
		ImGui::PopStyleColor();
	}

	if (!isSelected && descendantSelected)
	{
		ImGui::PopStyleColor();
	}

	const int32_t rowIndex = ImGui::TableGetRowIndex();

	if (rowIndex >= SelectionManager::GetFirstSelectedRow() && rowIndex <= SelectionManager::GetLastSelectedRow() && !SelectionManager::IsSelected(entity))
	{
		SelectionManager::Select(entity);
	}

	if (isPrefab)
	{
		ImGui::PopStyleColor();
	}

	if (isRowClicked)
	{
		if (!wasRowRightClicked)
		{
			if (!Volt::Input::IsKeyDown(VT_KEY_LEFT_CONTROL) && !Volt::Input::IsKeyDown(VT_KEY_LEFT_SHIFT))
			{
				SelectionManager::DeselectAll();
				SelectionManager::Select(entity);

				SelectionManager::GetFirstSelectedRow() = rowIndex;
				SelectionManager::GetLastSelectedRow() = -1;
			}
			else if (Volt::Input::IsKeyDown(VT_KEY_LEFT_SHIFT) && SelectionManager::GetSelectedCount() > 0)
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
					SelectionManager::Deselect(entity);
				}
				else
				{
					SelectionManager::Select(entity);
				}
			}
		}

	}

	// Drag & Drop
	//------------

	if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_SourceAllowNullID))
	{
		if (!SelectionManager::IsSelected(entity))
		{
			Volt::Entity ent{ entity, myScene.get() };
			ImGui::TextUnformatted(ent.GetTag().c_str());

			ImGui::SetDragDropPayload("scene_entity_hierarchy", &entity, sizeof(Wire::EntityId));
			ImGui::EndDragDropSource();
		}
		else
		{
			std::vector<Wire::EntityId> selectedEntities = SelectionManager::GetSelectedEntities();

			constexpr uint32_t maxEntNames = 5;
			for (uint32_t i = 0; const auto & id : selectedEntities)
			{
				if (i >= 5)
				{
					break;
				}

				Volt::Entity ent{ id, myScene.get() };
				ImGui::TextUnformatted(ent.GetTag().c_str());
				i++;
			}

			ImGui::SetDragDropPayload("scene_entity_hierarchy", selectedEntities.data(), selectedEntities.size() * sizeof(Wire::EntityId));
			ImGui::EndDragDropSource();
		}
	}

	if (ImGui::BeginDragDropTarget())
	{
		const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("scene_entity_hierarchy");

		// If there is a payload, assume it's all the selected entities
		if (payload)
		{
			const size_t count = payload->DataSize / sizeof(Wire::EntityId);
			std::vector<Ref<ParentChildData>> undoData;

			for (size_t i = 0; i < count; i++)
			{
				Wire::EntityId id = *(((Wire::EntityId*)payload->Data) + i);
				Volt::Entity parent(entity, myScene.get());
				Volt::Entity child(id, myScene.get());

				Ref<ParentChildData> data = CreateRef<ParentChildData>();
				data->myParent = parent;
				data->myChild = child;
				undoData.push_back(data);

				myScene->ParentEntity(parent, child);
			}

			Ref<ParentingCommand> command = CreateRef<ParentingCommand>(undoData, ParentingAction::Parent);
			EditorCommandStack::PushUndo(command);
		}

		ImGui::EndDragDropTarget();
	}

	std::string entityMenuId = "RightClickEntity" + std::to_string(entity);
	if (ImGui::BeginPopupContextItem(entityMenuId.c_str(), ImGuiPopupFlags_MouseButtonRight))
	{
		if (!SelectionManager::IsSelected(entity))
		{
			SelectionManager::DeselectAll();
			SelectionManager::Select(entity);
		}

		if (myScene->GetRegistry().HasComponent<Volt::PrefabComponent>(entity))
		{
			const auto& prefabComp = myScene->GetRegistry().GetComponent<Volt::PrefabComponent>(entity);

			if (Volt::Prefab::IsParentInPrefab(prefabComp.prefabEntity, prefabComp.prefabAsset))
			{
				const std::string menuId = "Override Prefab Asset##" + std::to_string(entity);

				if (ImGui::MenuItem(menuId.c_str()))
				{
					const auto prefabPath = Volt::AssetManager::Get().GetPathFromAssetHandle(prefabComp.prefabAsset);
					if (!FileSystem::IsWriteable(prefabPath))
					{
						UI::Notify(NotificationType::Error, "Unable to override prefab!", std::format("The prefab file {0} is not writeable!", prefabPath.string()));
					}
					else
					{
						Volt::Prefab::OverridePrefabAsset(myScene->GetRegistry(), entity, prefabComp.prefabAsset);

						auto prefabAsset = Volt::AssetManager::GetAsset<Volt::Prefab>(prefabComp.prefabAsset);
						if (prefabAsset && prefabAsset->IsValid())
						{
							Volt::AssetManager::Get().SaveAsset(prefabAsset);
							UI::Notify(NotificationType::Success, "Prefab overridden!", std::format("The prefab file {0} has been overriden!", prefabPath.string()));
						}
						else
						{
							UI::Notify(NotificationType::Error, "Unable to override prefab!", std::format("The prefab with id {0} is not valid!", (uint64_t)prefabComp.prefabAsset));
						}

					}
				}
			}
		}

		if (!myScene->GetRegistry().HasComponent<Volt::PrefabComponent>(entity))
		{
			const std::string menuId = "Create Prefab##" + std::to_string(entity);
			if (ImGui::MenuItem(menuId.c_str()))
			{
				CreatePrefabAndSetupEntities(entity);
			}
		}
		else
		{
			const auto& prefabComp = myScene->GetRegistry().GetComponent<Volt::PrefabComponent>(entity);

			const std::string menuId = "Reset Prefab##" + std::to_string(entity);
			if (ImGui::MenuItem(menuId.c_str()))
			{
				Volt::Prefab::OverridePrefabInRegistry(myScene->GetRegistry(), entity, prefabComp.prefabAsset);
			}
		}

		const std::string deleteId = "Delete##" + std::to_string(entity);
		if (ImGui::MenuItem(deleteId.c_str()))
		{
			entityDeleted = true;
		}

		ImGui::EndPopup();
	}

	if (myScene->GetRegistry().HasComponent<Volt::RelationshipComponent>(entity))
	{
		parentId = myScene->GetRegistry().GetComponent<Volt::RelationshipComponent>(entity).Parent;
	}

	if (entityDeleted)
	{
		std::vector<Volt::Entity> entitiesToRemove;

		auto selection = SelectionManager::GetSelectedEntities();
		for (const auto& selectedEntity : selection)
		{
			Volt::Entity tempEnt = Volt::Entity(selectedEntity, myScene.get());
			entitiesToRemove.push_back(tempEnt);

			SelectionManager::Deselect(tempEnt.GetId());
		}

		Ref<ObjectStateCommand> command = CreateRef<ObjectStateCommand>(entitiesToRemove, ObjectStateAction::Delete);
		EditorCommandStack::GetInstance().PushUndo(command);

		for (const auto& i : entitiesToRemove)
		{
			myScene->RemoveEntity(i);
		}
	}

	ImGui::TableNextColumn();

	// Modifiers
	{
		const float imageSize = 21.f;
		UI::ShiftCursor(0.f, 2.f);

		if (myScene->GetRegistry().HasComponent<Volt::TransformComponent>(entity))
		{
			auto& transformComponent = myScene->GetRegistry().GetComponent<Volt::TransformComponent>(entity);

			Ref<Volt::Texture2D> visibleIcon = transformComponent.visible ? EditorResources::GetEditorIcon(EditorIcon::Visible) : EditorResources::GetEditorIcon(EditorIcon::Hidden);
			std::string visibleId = "##visible" + std::to_string(entity);
			if (UI::ImageButton(visibleId, UI::GetTextureID(visibleIcon), { imageSize, imageSize }, { 0.f, 0.f }, { 1.f, 1.f }, 0))
			{
				const auto newVal = !transformComponent.visible;
				if (!SelectionManager::IsSelected(entity))
				{
					transformComponent.visible = newVal;
				}
				else
				{
					for (const auto& e : SelectionManager::GetSelectedEntities())
					{
						auto& eTransformComponent = myScene->GetRegistry().GetComponent<Volt::TransformComponent>(e);
						eTransformComponent.visible = newVal;
					}
				}

			}

			ImGui::SameLine();

			Ref<Volt::Texture2D> lockedIcon = transformComponent.locked ? EditorResources::GetEditorIcon(EditorIcon::Locked) : EditorResources::GetEditorIcon(EditorIcon::Unlocked);
			std::string lockedId = "##locked" + std::to_string(entity);
			if (UI::ImageButton(lockedId, UI::GetTextureID(lockedIcon), { imageSize, imageSize }, { 0.f, 0.f }, { 1.f, 1.f }, 0))
			{
				const auto newVal = !transformComponent.locked;
				if (!SelectionManager::IsSelected(entity))
				{
					transformComponent.locked = newVal;
				}
				else
				{
					for (const auto& e : SelectionManager::GetSelectedEntities())
					{
						auto& eTransformComponent = myScene->GetRegistry().GetComponent<Volt::TransformComponent>(e);
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
			DrawEntity(child, filter);
		}

		ImGui::TreePop();
	}
}

void SceneViewPanel::CreatePrefabAndSetupEntities(Wire::EntityId entity)
{
	const auto& tagComp = myScene->GetRegistry().GetComponent<Volt::TagComponent>(entity);

	Ref<Volt::Prefab> prefab = CreateRef<Volt::Prefab>(myScene->GetRegistry(), entity);

	std::string path = "Assets/Prefabs/" + tagComp.tag + ".vtprefab";
	path.erase(std::remove_if(path.begin(), path.end(), ::isspace), path.end());

	prefab->path = path;
	Volt::AssetManager::Get().SaveAsset(prefab);

	SetupEntityAsPrefab(entity, prefab->handle);
}

void SceneViewPanel::SetupEntityAsPrefab(Wire::EntityId entity, Volt::AssetHandle prefabId)
{
	if (!myScene->GetRegistry().HasComponent<Volt::PrefabComponent>(entity))
	{
		myScene->GetRegistry().AddComponent<Volt::PrefabComponent>(entity);
	}

	auto& prefabComp = myScene->GetRegistry().GetComponent<Volt::PrefabComponent>(entity);
	prefabComp.prefabAsset = prefabId;
	prefabComp.prefabEntity = entity;

	auto& relComp = myScene->GetRegistry().GetComponent<Volt::RelationshipComponent>(entity);
	for (const auto& child : relComp.Children)
	{
		SetupEntityAsPrefab(child, prefabId);
	}
}

bool SceneViewPanel::SearchRecursivly(Wire::EntityId id, const std::string& filter, uint32_t maxSearchDepth, uint32_t currentDepth)
{
	if (filter.empty())
	{
		return false;
	}

	Volt::Entity ent{ id, myScene.get() };

	for (auto child : ent.GetChilden())
	{
		if (child.HasComponent<Volt::TagComponent>())
		{
			std::string t = child.GetComponent<Volt::TagComponent>().tag;
			if (MatchesQuery(t, filter))
			{
				return true;
			}
		}

		const bool found = SearchRecursivly(child.GetId(), filter, maxSearchDepth, currentDepth + 1);
		if (found)
		{
			return true;
		}
	}

	return false;
}

bool SceneViewPanel::MatchesQuery(const std::string& text, const std::string& filter)
{
	if (filter.empty())
	{
		return true;
	}

	std::string query = Utils::ToLower(filter);
	const std::string lowerText = Utils::ToLower(text);

	query.push_back(' ');
	std::vector<std::string> queries;

	for (auto next = query.find_first_of(' '); next != std::string::npos; next = query.find_first_of(' '))
	{
		std::string split = query.substr(0, next);
		query = query.substr(next + 1);
		queries.emplace_back(split);
	}

	for (const auto& q : queries)
	{
		if (lowerText.contains(q))
		{
			return true;
		}
	}

	return false;
}

void SceneViewPanel::DrawMainRightClickPopup()
{
	if (UI::BeginPopup("MainRightClickMenu", ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse))
	{
		if (ImGui::MenuItem("Create Empty Entity"))
		{
			auto ent = myScene->CreateEntity();

			Ref<ObjectStateCommand> command = CreateRef<ObjectStateCommand>(ent, ObjectStateAction::Create);
			EditorCommandStack::GetInstance().PushUndo(command);
			SelectionManager::DeselectAll();
			SelectionManager::Select(ent.GetId());
		}

		if (ImGui::BeginMenu("New"))
		{
			if (ImGui::BeginMenu("Primitives"))
			{
				if (ImGui::MenuItem("Cube"))
				{
					auto ent = myScene->CreateEntity();
					auto& meshComp = ent.AddComponent<Volt::MeshComponent>();
					meshComp.handle = Volt::AssetManager::GetAssetHandleFromPath("Assets/Meshes/Primitives/SM_Cube.vtmesh");
					ent.SetTag("New Cube");

					SelectionManager::DeselectAll();
					SelectionManager::Select(ent.GetId());
				}

				if (ImGui::MenuItem("Capsule"))
				{
					auto ent = myScene->CreateEntity();
					auto& meshComp = ent.AddComponent<Volt::MeshComponent>();
					meshComp.handle = Volt::AssetManager::GetAssetHandleFromPath("Assets/Meshes/Primitives/SM_Capsule.vtmesh");
					ent.SetTag("New Capsule");

					SelectionManager::DeselectAll();
					SelectionManager::Select(ent.GetId());
				}

				if (ImGui::MenuItem("Cone"))
				{
					auto ent = myScene->CreateEntity();
					auto& meshComp = ent.AddComponent<Volt::MeshComponent>();
					meshComp.handle = Volt::AssetManager::GetAssetHandleFromPath("Assets/Meshes/Primitives/SM_Cone.vtmesh");
					ent.SetTag("New Cone");

					SelectionManager::DeselectAll();
					SelectionManager::Select(ent.GetId());
				}

				if (ImGui::MenuItem("Cylinder"))
				{
					auto ent = myScene->CreateEntity();
					auto& meshComp = ent.AddComponent<Volt::MeshComponent>();
					meshComp.handle = Volt::AssetManager::GetAssetHandleFromPath("Assets/Meshes/Primitives/SM_Cylinder.vtmesh");
					ent.SetTag("New Cylinder");

					SelectionManager::DeselectAll();
					SelectionManager::Select(ent.GetId());
				}

				if (ImGui::MenuItem("Sphere"))
				{
					auto ent = myScene->CreateEntity();
					auto& meshComp = ent.AddComponent<Volt::MeshComponent>();
					meshComp.handle = Volt::AssetManager::GetAssetHandleFromPath("Assets/Meshes/Primitives/SM_Sphere.vtmesh");
					ent.SetTag("New Sphere");

					SelectionManager::DeselectAll();
					SelectionManager::Select(ent.GetId());
				}

				if (ImGui::MenuItem("Plane"))
				{
					auto ent = myScene->CreateEntity();
					auto& meshComp = ent.AddComponent<Volt::MeshComponent>();
					meshComp.handle = Volt::AssetManager::GetAssetHandleFromPath("Assets/Meshes/Primitives/SM_Plane.vtmesh");
					ent.SetTag("New Plane");

					SelectionManager::DeselectAll();
					SelectionManager::Select(ent.GetId());
				}

				ImGui::EndMenu();
			}

			if (ImGui::BeginMenu("Lights"))
			{
				if (ImGui::MenuItem("Point Light"))
				{
					auto ent = myScene->CreateEntity();
					ent.AddComponent<Volt::PointLightComponent>();
					ent.SetTag("New Point Light");

					SelectionManager::DeselectAll();
					SelectionManager::Select(ent.GetId());
				}

				if (ImGui::MenuItem("Directional Light"))
				{
					auto ent = myScene->CreateEntity();
					ent.AddComponent<Volt::DirectionalLightComponent>();
					ent.SetTag("New Directional Light");

					SelectionManager::DeselectAll();
					SelectionManager::Select(ent.GetId());
				}

				if (ImGui::MenuItem("Skylight"))
				{
					auto ent = myScene->CreateEntity();
					ent.AddComponent<Volt::SkylightComponent>();
					ent.SetTag("New Skylight");

					SelectionManager::DeselectAll();
					SelectionManager::Select(ent.GetId());
				}

				ImGui::EndMenu();
			}

			ImGui::EndMenu();
		}

		ImGui::EndPopup();
	}
}
