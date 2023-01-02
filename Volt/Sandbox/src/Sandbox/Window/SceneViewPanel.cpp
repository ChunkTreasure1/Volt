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

SceneViewPanel::SceneViewPanel(Ref<Volt::Scene>& scene)
	: EditorWindow("Scene View"), myScene(scene)
{
	myWindowFlags = ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse;
	myIsOpen = true;
}

void SceneViewPanel::UpdateMainContent()
{
	VT_PROFILE_FUNCTION();

	if (EditorUtils::SearchBar(mySearchQuery, myHasSearchQuery))
	{
		Search(mySearchQuery);
	}

	ImGui::BeginChild("MainView", ImGui::GetContentRegionAvail());
	{
		UI::ScopedColor button(ImGuiCol_Button, { 0.f, 0.f, 0.f, 0.f });
		UI::ScopedColor hovered(ImGuiCol_ButtonHovered, { 0.3f, 0.305f, 0.31f, 0.5f });
		UI::ScopedColor active(ImGuiCol_ButtonActive, { 0.5f, 0.505f, 0.51f, 0.5f });
		UI::ScopedColor tableRow(ImGuiCol_TableRowBg, { 0.18f, 0.18f, 0.18f, 1.f });
		UI::ScopedStyleFloat2 padd{ ImGuiStyleVar_FramePadding, { 4.f, 2.f } };
		UI::ScopedStyleFloat2 padd1{ ImGuiStyleVar_CellPadding, { 4.f, 0.f } };

		if (ImGui::BeginTable("entitiesTable", 2, ImGuiTableFlags_SizingStretchProp | ImGuiTableFlags_Resizable))
		{
			ImGui::TableSetupColumn("Name", ImGuiTableColumnFlags_WidthStretch);
			ImGui::TableSetupColumn("Modifiers", ImGuiTableColumnFlags_WidthFixed, 40.f);
			ImGui::TableHeadersRow();

			std::vector<Wire::EntityId> usedEntities;
			if (myHasSearchQuery)
			{
				usedEntities = mySearchEntities;
			}
			else
			{
				usedEntities = myScene->GetRegistry().GetAllEntities();
			}

			ImGui::PushStyleColor(ImGuiCol_Header, { 0.f, 0.44f, 1.f, 1.f });

			// Draw Entities
			{
				VT_PROFILE_SCOPE("Draw Entities")
				for (const auto& entity : usedEntities)
				{
					if (myScene->GetRegistry().HasComponent<Volt::RelationshipComponent>(entity))
					{
						if (myScene->GetRegistry().GetComponent<Volt::RelationshipComponent>(entity).Parent == Wire::NullID)
						{
							DrawEntity(entity, usedEntities);
						}
					}
				}
			}

			ImGui::PopStyleColor();

			ImGui::EndTable();

			// Right-click on blank space
			if (ImGui::BeginPopupContextWindow("RightClickPanel", ImGuiPopupFlags_MouseButtonRight | ImGuiPopupFlags_NoOpenOverExistingPopup))
			{
				if (ImGui::MenuItem("Create Empty Entity"))
				{
					auto ent = myScene->CreateEntity();

					Ref<ObjectStateCommand> command = CreateRef<ObjectStateCommand>(ent, ObjectStateAction::Create);
					EditorCommandStack::GetInstance().PushUndo(command);
					SelectionManager::DeselectAll();
					SelectionManager::Select(ent.GetId());
				}

				ImGui::EndPopup();
			}

			ImRect windowRect = { ImGui::GetWindowContentRegionMin(), ImGui::GetWindowContentRegionMax() };

			if (ImGui::BeginDragDropTargetCustom(windowRect, ImGui::GetCurrentWindow()->ID))
			{
				const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("scene_entity_hierarchy", ImGuiDragDropFlags_AcceptNoDrawDefaultRect);

				// If there is a payload, assume it's all the selected entities
				if (payload)
				{
					std::vector<Ref<ParentChildData>> undoData;

					//for (const auto& ent : SelectionManager::GetSelectedEntities())
					//{
					//	Volt::Entity entity{ ent, myScene.get() };
					//	auto& relComp = entity.GetComponent<Volt::RelationshipComponent>();

					//	if (relComp.Parent != 0)
					//	{
					//		Ref<ParentChildData> data = CreateRef<ParentChildData>();
					//		data->myParent = Volt::Entity(relComp.Parent, myScene.get());
					//		data->myChild = Volt::Entity(ent, myScene.get());
					//		undoData.push_back(data);
					//	}

					//	myScene->UnparentEntity(Volt::Entity(ent, myScene.get()));
					//}


					Wire::EntityId droppedEntity = *(Wire::EntityId*)payload->Data;

					Volt::Entity entity{ droppedEntity, myScene.get() };

					bool dataAlreadyAdded = false;
					for (const auto& i : undoData)
					{
						if (entity == i->myChild)
						{
							dataAlreadyAdded = true;
						}
					}

					if (!dataAlreadyAdded)
					{
						auto& relComp = entity.GetComponent<Volt::RelationshipComponent>();

						Ref<ParentChildData> data = CreateRef<ParentChildData>();
						data->myParent = Volt::Entity(relComp.Parent, myScene.get());
						data->myChild = entity;
						undoData.push_back(data);
					}

					Ref<ParentingCommand> command = CreateRef<ParentingCommand>(undoData, ParentingAction::Unparent);
					EditorCommandStack::PushUndo(command);

					myScene->UnparentEntity(Volt::Entity(droppedEntity, myScene.get()));
				}

				ImGui::EndDragDropTarget();
			}
		}
	}

	ImGui::EndChild();

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
}

VT_OPTIMIZE_OFF

void SceneViewPanel::DrawEntity(Wire::EntityId entity, const std::vector<Wire::EntityId>& usedEntities)
{
	VT_PROFILE_FUNCTION();

	const float imageSize = 20.f;
	const float imagePadding = 5.f;
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

	ImGui::TableNextRow();
	ImGui::TableNextColumn();

	std::string entityId = entityName + "###" + std::to_string(entity);
	bool open = false;
	bool isPrefab = myScene->GetRegistry().HasComponent<Volt::PrefabComponent>(entity);

	if (isPrefab)
	{
		ImGui::PushStyleColor(ImGuiCol_Text, { 56.f / 255.f, 156.f / 255.f, 1.f, 1.f });
	}

	if (!children.empty())
	{
		ImGuiTreeNodeFlags flags = SelectionManager::IsSelected(entity) ? ImGuiTreeNodeFlags_Selected : 0;
		flags |= ImGuiTreeNodeFlags_SpanAllColumns | ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_FramePadding;

		open = ImGui::TreeNodeEx(entityId.c_str(), flags);
	}
	else
	{
		ImGuiTreeNodeFlags flags = SelectionManager::IsSelected(entity) ? ImGuiTreeNodeFlags_Selected : 0;
		flags |= ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen | ImGuiTreeNodeFlags_SpanAllColumns | ImGuiTreeNodeFlags_FramePadding;

		if (parentId == Wire::NullID)
		{
			UI::ShiftCursor(-20.f, 0.f);
		}
		ImGui::TreeNodeEx(entityId.c_str(), flags);
	}

	if (isPrefab)
	{
		ImGui::PopStyleColor();
	}

	if (ImGui::IsItemHovered() && ImGui::IsMouseReleased(ImGuiMouseButton_Left))
	{
		if (Volt::Input::IsKeyDown(VT_KEY_LEFT_CONTROL))
		{
			if (SelectionManager::IsSelected(entity))
			{
				SelectionManager::Deselect(entity);
			}
			else
			{
				SelectionManager::Select(entity);
			}
		}
		else if (Volt::Input::IsKeyDown(VT_KEY_LEFT_SHIFT))
		{
			if (SelectionManager::GetSelectedCount() == 1)
			{
				SelectEntitiesBetweenClosest(entity, usedEntities);
			}
			else
			{
				SelectionManager::Select(entity);
			}
		}
		else
		{
			if (SelectionManager::IsSelected(entity) && SelectionManager::GetSelectedCount() == 1)
			{
				SelectionManager::Deselect(entity);
			}
			else
			{
				SelectionManager::DeselectAll();
				SelectionManager::Select(entity);
			}
		}
	}

	// Drag & Drop
	//------------

	if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_SourceAllowNullID))
	{
		ImGui::SetDragDropPayload("scene_entity_hierarchy", &entity, sizeof(Wire::EntityId));
		ImGui::EndDragDropSource();
	}

	if (ImGui::BeginDragDropTarget())
	{
		const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("scene_entity_hierarchy");

		// If there is a payload, assume it's all the selected entities
		if (payload)
		{
			std::vector<Ref<ParentChildData>> undoData;

			//for (const auto& ent : SelectionManager::GetSelectedEntities())
			//{
			//	Ref<ParentChildData> data = CreateRef<ParentChildData>();
			//	data->myParent = Volt::Entity(entity, myScene.get());
			//	data->myChild = Volt::Entity(ent, myScene.get());
			//	undoData.push_back(data);

			//	myScene->ParentEntity(Volt::Entity(entity, myScene.get()), Volt::Entity(ent, myScene.get()));
			//}

			Wire::EntityId& droppedEntity = *(Wire::EntityId*)payload->Data;

			Volt::Entity ent(droppedEntity, myScene.get());
			bool dataAlreadyAdded = false;
			for (const auto& i : undoData)
			{
				if (ent == i->myChild)
				{
					dataAlreadyAdded = true;
				}
			}

			if (!dataAlreadyAdded)
			{
				Ref<ParentChildData> data = CreateRef<ParentChildData>();
				data->myParent = Volt::Entity(entity, myScene.get());
				data->myChild = ent;
				undoData.push_back(data);
			}

			Ref<ParentingCommand> command = CreateRef<ParentingCommand>(undoData, ParentingAction::Parent);
			EditorCommandStack::PushUndo(command);

			myScene->ParentEntity(Volt::Entity(entity, myScene.get()), Volt::Entity(droppedEntity, myScene.get()));
		}

		ImGui::EndDragDropTarget();
	}

	std::string entityMenuId = "RightClickEntity" + std::to_string(entity);
	if (ImGui::BeginPopupContextItem(entityMenuId.c_str(), ImGuiPopupFlags_MouseButtonRight))
	{
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
		if (myScene->GetRegistry().HasComponent<Volt::TransformComponent>(entity))
		{
			auto& transformComponent = myScene->GetRegistry().GetComponent<Volt::TransformComponent>(entity);

			Ref<Volt::Texture2D> visibleIcon = transformComponent.visible ? EditorResources::GetEditorIcon(EditorIcon::Visible) : EditorResources::GetEditorIcon(EditorIcon::Hidden);
			std::string visibleId = "##visible" + std::to_string(entity);
			if (UI::ImageButton(visibleId, UI::GetTextureID(visibleIcon), { imageSize, imageSize }, { 0.f, 0.f }, { 1.f, 1.f }, 0))
			{
				auto newVal = !transformComponent.visible;
				for (const auto& e : SelectionManager::GetSelectedEntities())
				{
					auto& eTransformComponent = myScene->GetRegistry().GetComponent<Volt::TransformComponent>(e);
					eTransformComponent.visible = newVal;
				}
			}

			ImGui::SameLine();

			Ref<Volt::Texture2D> lockedIcon = transformComponent.locked ? EditorResources::GetEditorIcon(EditorIcon::Locked) : EditorResources::GetEditorIcon(EditorIcon::Unlocked);
			std::string lockedId = "##locked" + std::to_string(entity);
			if (UI::ImageButton(lockedId, UI::GetTextureID(lockedIcon), { imageSize, imageSize }, { 0.f, 0.f }, { 1.f, 1.f }, 0))
			{
				auto newVal = !transformComponent.locked;
				for (const auto& e : SelectionManager::GetSelectedEntities())
				{
					auto& eTransformComponent = myScene->GetRegistry().GetComponent<Volt::TransformComponent>(e);
					eTransformComponent.locked = newVal;
				}
			}
		}

	}

	ImGui::TableNextColumn();

	if (open)
	{
		for (const auto& child : children)
		{
			DrawEntity(child, usedEntities);
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

void SceneViewPanel::SelectEntitiesBetweenClosest(Wire::EntityId entity, const std::vector<Wire::EntityId>& usedEntities)
{
	// Find closest selected entity ("up" or "down")

	int32_t shortestDistance = INT32_MAX;
	std::vector<Wire::EntityId>::const_iterator closestIt;
	const auto newIt = std::find(usedEntities.begin(), usedEntities.end(), entity);

	for (const auto& selectedEnt : SelectionManager::GetSelectedEntities())
	{
		const auto selectedIt = std::find(usedEntities.begin(), usedEntities.end(), selectedEnt);

		const int32_t distBetweenItems = (int32_t)std::distance(selectedIt, newIt);

		if (std::abs(distBetweenItems) < std::abs(shortestDistance))
		{
			shortestDistance = distBetweenItems;
			closestIt = selectedIt;
		}
	}

	if (shortestDistance < 0)
	{
		for (auto it = closestIt; it != newIt; --it)
		{
			SelectionManager::Select(*it);
			RecursivlySelectChildren(*it);
		}
	}
	else
	{
		for (auto it = closestIt; it != newIt; ++it)
		{
			SelectionManager::Select(*it);
			RecursivlySelectChildren(*it);
		}
	}

	SelectionManager::Select(entity);

	// Select siblings "up" or "down"
	if (myScene->GetRegistry().HasComponent<Volt::RelationshipComponent>(entity))
	{
		auto& currEntRelComp = myScene->GetRegistry().GetComponent<Volt::RelationshipComponent>(entity);
		if (currEntRelComp.Parent != Wire::NullID)
		{
			if (myScene->GetRegistry().HasComponent<Volt::RelationshipComponent>(currEntRelComp.Parent))
			{
				auto& parentEntRelComp = myScene->GetRegistry().GetComponent<Volt::RelationshipComponent>(currEntRelComp.Parent);

				auto startSiblingIt = std::find(parentEntRelComp.Children.begin(), parentEntRelComp.Children.end(), entity);

				if (shortestDistance < 0)
				{
					for (auto it = startSiblingIt; it != parentEntRelComp.Children.end(); ++it)
					{
						SelectionManager::Select(*it);
						RecursivlySelectChildren(*it);
					}
				}
				else
				{
					for (auto it = startSiblingIt; it != parentEntRelComp.Children.begin(); --it)
					{
						SelectionManager::Select(*it);
						RecursivlySelectChildren(*it);
					}
				}
			}
		}
	}
}

void SceneViewPanel::RecursivlySelectChildren(Wire::EntityId entity)
{
	if (myScene->GetRegistry().HasComponent<Volt::RelationshipComponent>(entity))
	{
		auto& relComp = myScene->GetRegistry().GetComponent<Volt::RelationshipComponent>(entity);
		for (const auto& child : relComp.Children)
		{
			SelectionManager::Select(child);
			RecursivlySelectChildren(child);
		}
	}
}

void SceneViewPanel::Search(const std::string query)
{
	const std::string lowerQuery = Utils::ToLower(query);

	mySearchEntities.clear();

	for (auto entity : myScene->GetRegistry().GetAllEntities())
	{
		if (myScene->GetRegistry().HasComponent<Volt::TagComponent>(entity))
		{
			auto tagComp = myScene->GetRegistry().GetComponent<Volt::TagComponent>(entity);
			const std::string lowerName = Utils::ToLower(tagComp.tag);

			if (lowerName.find(lowerQuery) != std::string::npos)
			{
				mySearchEntities.emplace_back(entity);
			}
		}
	}
}