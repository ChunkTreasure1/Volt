#include "vtpch.h"
#include "Prefab.h"

#include "Volt/Asset/AssetManager.h"

#include "Volt/Components/Components.h"
#include "Volt/Components/PhysicsComponents.h"

#include "Volt/Animation/AnimationController.h"

#include "Volt/Asset/Animation/AnimationGraphAsset.h"
#include "Volt/Scene/Entity.h"

#include "Volt/Scripting/Mono/MonoScriptEngine.h"
#include "Volt/Net/SceneInteraction/NetActorComponent.h"

#include "Volt/Utility/Random.h"

#include <GraphKey/Graph.h>
#include <GraphKey/Node.h>

namespace Volt
{
	//Prefab::Prefab(Wire::Registry& aParentRegistry, Wire::EntityId topEntity)
	//{
	//	uint32_t count = 0;
	//	AddToPrefab(aParentRegistry, topEntity, count);
	//}

	//Wire::EntityId Prefab::Instantiate(Scene* targetScene, Wire::EntityId specifiedTargetId)
	//{
	//	auto& targetRegistry = targetScene->GetRegistry();

	//	Wire::EntityId rootEntity = GetRootId();
	//	Wire::EntityId newRoot = InstantiateEntity(targetScene, rootEntity, Wire::NullID, specifiedTargetId);

	//	auto& rootTransform = targetRegistry.GetComponent<TransformComponent>(newRoot);
	//	rootTransform.position = { 0.f, 0.f, 0.f };
	//	rootTransform.rotation = myRegistry.GetComponent<TransformComponent>(rootEntity).rotation;
	//	rootTransform.scale = myRegistry.GetComponent<TransformComponent>(rootEntity).scale;

	//	auto& tagComp = targetRegistry.GetComponent<TagComponent>(newRoot);
	//	tagComp.tag = path.stem().string();

	//	Entity ent{ newRoot, targetScene };
	//	targetScene->MoveToLayer(ent, targetScene->GetActiveLayer());

	//	return newRoot;
	//}

	//Wire::EntityId Prefab::GetRootId()
	//{
	//	Wire::EntityId rootEntity = Wire::NullID;
	//	for (const auto& ent : myRegistry.GetAllEntities())
	//	{
	//		if (myRegistry.HasComponent<RelationshipComponent>(ent))
	//		{
	//			auto& relComp = myRegistry.GetComponent<RelationshipComponent>(ent);
	//			if (relComp.Parent == Wire::NullID)
	//			{
	//				rootEntity = ent;
	//				break;
	//			}
	//		}
	//	}
	//	return rootEntity;
	//}

	//void Prefab::OverridePrefabInRegistry(Wire::Registry& aTargetRegistry, Wire::EntityId aRootEntity, AssetHandle prefabId)
	//{
	//	Ref<Prefab> prefabAsset = AssetManager::GetAsset<Prefab>(prefabId);
	//	if (!prefabAsset || !prefabAsset->IsValid())
	//	{
	//		VT_CORE_WARN("[Prefab]: Trying to override prefabs with ID {0}, but the prefab is not valid!", prefabId);
	//		return;
	//	}

	//	if (!aTargetRegistry.HasComponent<PrefabComponent>(aRootEntity))
	//	{
	//		VT_CORE_WARN("[Prefab]: Root entity is not a prefab!");
	//		return;
	//	}

	//	OverrideEntityInPrefab(aTargetRegistry, aRootEntity, prefabAsset);
	//}

	//void Prefab::OverridePrefabsInRegistry(Wire::Registry& aTargetRegistry, AssetHandle aPrefabId)
	//{
	//	for (const auto& entity : aTargetRegistry.GetAllEntities())
	//	{
	//		if (!aTargetRegistry.HasComponent<PrefabComponent>(entity))
	//		{
	//			continue;
	//		}

	//		auto& prefabComp = aTargetRegistry.GetComponent<PrefabComponent>(entity);
	//		if (prefabComp.prefabAsset != aPrefabId)
	//		{
	//			continue;
	//		}

	//		if (aTargetRegistry.HasComponent<RelationshipComponent>(entity))
	//		{
	//			auto& relComp = aTargetRegistry.GetComponent<RelationshipComponent>(entity);
	//			if (relComp.Parent != Wire::NullID)
	//			{
	//				if (aTargetRegistry.HasComponent<PrefabComponent>(relComp.Parent))
	//				{
	//					auto& parentPrefabComp = aTargetRegistry.GetComponent<PrefabComponent>(relComp.Parent);
	//					if (parentPrefabComp.prefabAsset == prefabComp.prefabAsset)
	//					{
	//						continue;
	//					}
	//				}
	//			}
	//		}

	//		OverridePrefabInRegistry(aTargetRegistry, entity, aPrefabId);
	//	}
	//}

	//void RecursiveDeleteChildren(Wire::Registry& aTargetRegistry, Wire::EntityId aTargetEntity)
	//{
	//	auto& relComp = aTargetRegistry.GetComponent<RelationshipComponent>(aTargetEntity);
	//	if (relComp.Parent)
	//	{
	//		auto& parentRelComp = aTargetRegistry.GetComponent<RelationshipComponent>(relComp.Parent);
	//		parentRelComp.Children.erase(std::find(parentRelComp.Children.begin(), parentRelComp.Children.end(), aTargetEntity));
	//	}
	//	for (const auto& id : relComp.Children)
	//	{
	//		RecursiveDeleteChildren(aTargetRegistry, id);
	//	}
	//	aTargetRegistry.RemoveEntity(aTargetEntity);
	//}

	//void Prefab::OverridePrefabAsset(Wire::Registry& aSrcRegistry, Wire::EntityId aSrcEntity, AssetHandle aPrefabId)
	//{
	//	Ref<Prefab> prefabAsset = AssetManager::GetAsset<Prefab>(aPrefabId);
	//	if (!prefabAsset || !prefabAsset->IsValid())
	//	{
	//		VT_CORE_WARN("[Prefab]: Trying to override prefab with ID {0}, but the prefab is not valid!", aPrefabId);
	//		return;
	//	}

	//	if (!aSrcRegistry.HasComponent<PrefabComponent>(aSrcEntity))
	//	{
	//		VT_CORE_WARN("[Prefab]: Root entity is not a prefab!");
	//		return;
	//	}


	//	prefabAsset->myVersion++;
	//	prefabAsset->OverrideEntity(aSrcRegistry, aSrcEntity);
	//	for (const auto& entityId : prefabAsset->myRegistry.GetAllEntities())
	//	{
	//		if (std::find(prefabAsset->myKeepEntitiesQueue.begin(), prefabAsset->myKeepEntitiesQueue.end(), entityId) == prefabAsset->myKeepEntitiesQueue.end())
	//		{
	//			RecursiveDeleteChildren(prefabAsset->myRegistry, entityId);
	//		}
	//	}
	//	prefabAsset->myKeepEntitiesQueue.clear();
	//	OverridePrefabsInRegistry(aSrcRegistry, aPrefabId);
	//}

	//bool Prefab::IsRootInPrefab(Wire::EntityId aEntityId, AssetHandle aPrefabId)
	//{
	//	Ref<Prefab> prefabAsset = AssetManager::GetAsset<Prefab>(aPrefabId);
	//	if (!prefabAsset || !prefabAsset->IsValid())
	//	{
	//		return false;
	//	}

	//	Wire::EntityId rootEntity = prefabAsset->GetRootId();

	//	if (aEntityId == rootEntity)
	//	{
	//		return true;
	//	}

	//	return false;
	//}

	//bool Prefab::IsValidInPrefab(Wire::EntityId aEntityId, AssetHandle aPrefabId)
	//{
	//	Ref<Prefab> prefabAsset = AssetManager::GetAsset<Prefab>(aPrefabId);
	//	if (!prefabAsset || !prefabAsset->IsValid())
	//	{
	//		return false;
	//	}

	//	return prefabAsset->myRegistry.Exists(aEntityId);
	//}

	//uint32_t Prefab::GetPrefabVersion(AssetHandle aPrefabId)
	//{
	//	Ref<Prefab> prefabAsset = AssetManager::GetAsset<Prefab>(aPrefabId);
	//	if (!prefabAsset || !prefabAsset->IsValid())
	//	{
	//		return 0;
	//	}

	//	return prefabAsset->myVersion;
	//}

	//Volt::AssetHandle Prefab::GetCorrectAssethandle(Wire::EntityId aRootId, AssetHandle aPrefabId)
	//{
	//	Ref<Prefab> prefabAsset = AssetManager::GetAsset<Prefab>(aPrefabId);
	//	if (!prefabAsset || !prefabAsset->IsValid() || !prefabAsset->myRegistry.Exists(aRootId))
	//	{
	//		return 0;
	//	}

	//	return prefabAsset->myRegistry.GetComponent<PrefabComponent>(aRootId).prefabAsset;
	//}

	//void Prefab::OverrideEntityInPrefab(Wire::Registry& aTargetRegistry, Wire::EntityId aEntity, Ref<Prefab> aPrefab)
	//{
	//	if (!aTargetRegistry.HasComponent<PrefabComponent>(aEntity))
	//	{
	//		return;
	//	}

	//	auto& srcRegistry = aPrefab->myRegistry;
	//	auto& targetPrefabComp = aTargetRegistry.GetComponent<PrefabComponent>(aEntity);

	//	if (!srcRegistry.Exists(targetPrefabComp.prefabEntity) || targetPrefabComp.prefabAsset != aPrefab->handle)
	//	{
	//		return;
	//	}

	//	std::vector<WireGUID> excludedComponents = { RelationshipComponent::comp_guid, PrefabComponent::comp_guid, EntityDataComponent::comp_guid };

	//	auto& targetRelationshipComponent = srcRegistry.GetComponent<RelationshipComponent>(targetPrefabComp.prefabEntity);
	//	if (targetRelationshipComponent.Parent == Wire::NullID)
	//	{
	//		excludedComponents.emplace_back(Wire::ComponentRegistry::GetRegistryDataFromName("TransformComponent").guid);
	//	}

	//	Entity::Copy(srcRegistry, aTargetRegistry, targetPrefabComp.prefabEntity, aEntity, excludedComponents, true);
	//	targetPrefabComp.version = aPrefab->myVersion;

	//	for (const auto& child : targetRelationshipComponent.Children)
	//	{
	//		OverrideEntityInPrefab(aTargetRegistry, child, aPrefab);
	//	}

	//	// #TODO(Ivar): Check that parenting is correct in all entities
	//}

	//Wire::EntityId Prefab::InstantiateEntity(Scene* targetScene, Wire::EntityId prefabEntity, Wire::EntityId parentEntity, Wire::EntityId specifiedTargetId)
	//{
	//	auto& targetRegistry = targetScene->GetRegistry();

	//	Wire::EntityId newEntity = (specifiedTargetId) ? targetRegistry.AddEntity(specifiedTargetId) : targetRegistry.CreateEntity();
	//	Entity::Copy(myRegistry, targetRegistry, prefabEntity, newEntity, { Volt::RigidbodyComponent::comp_guid, Volt::CharacterControllerComponent::comp_guid }, true);

	//	if (myRegistry.HasComponent<Volt::RigidbodyComponent>(prefabEntity))
	//	{
	//		const auto srcComp = myRegistry.GetComponent<Volt::RigidbodyComponent>(prefabEntity);
	//		targetRegistry.AddComponent<Volt::RigidbodyComponent>(newEntity, srcComp.bodyType, srcComp.layerId, srcComp.mass, srcComp.linearDrag, srcComp.lockFlags, srcComp.angularDrag, srcComp.disableGravity, srcComp.isKinematic, srcComp.collisionType);
	//	}

	//	if (myRegistry.HasComponent<Volt::CharacterControllerComponent>(prefabEntity))
	//	{
	//		const auto srcComp = myRegistry.GetComponent<Volt::CharacterControllerComponent>(prefabEntity);
	//		targetRegistry.AddComponent<Volt::CharacterControllerComponent>(newEntity, srcComp.climbingMode, srcComp.slopeLimit, srcComp.invisibleWallHeight, srcComp.maxJumpHeight, srcComp.contactOffset, srcComp.stepOffset, srcComp.density, srcComp.layer, srcComp.hasGravity);
	//	}

	//	if (myRegistry.HasComponent<RelationshipComponent>(prefabEntity))
	//	{
	//		auto& relComp = myRegistry.GetComponent<RelationshipComponent>(prefabEntity);
	//		if (!targetRegistry.HasComponent<RelationshipComponent>(newEntity))
	//		{
	//			targetRegistry.AddComponent<RelationshipComponent>(newEntity);
	//		}

	//		{
	//			auto& relCompNew = targetRegistry.GetComponent<RelationshipComponent>(newEntity);
	//			relCompNew.Children.clear();
	//			relCompNew.Parent = parentEntity;
	//		}

	//		for (const auto& child : relComp.Children)
	//		{
	//			Wire::EntityId entity = InstantiateEntity(targetScene, child, newEntity);
	//			auto& relCompNew = targetRegistry.GetComponent<RelationshipComponent>(newEntity);
	//			relCompNew.Children.emplace_back(entity);
	//		}
	//	}

	//	if (targetRegistry.HasComponent<AnimationControllerComponent>(newEntity))
	//	{
	//		auto& comp = targetRegistry.GetComponent<AnimationControllerComponent>(newEntity);

	//		if (comp.animationGraph != Asset::Null())
	//		{
	//			auto graph = AssetManager::GetAsset<AnimationGraphAsset>(comp.animationGraph);
	//			if (graph && graph->IsValid())
	//			{
	//				comp.controller = CreateRef<AnimationController>(graph, Volt::Entity{ newEntity, targetScene });
	//			}
	//		}
	//	}

	//	if (targetRegistry.HasComponent<VisualScriptingComponent>(newEntity))
	//	{
	//		auto& vsComp = targetRegistry.GetComponent<VisualScriptingComponent>(newEntity);
	//		if (vsComp.graph)
	//		{
	//			// Update entityId if the entity ID is of the prefab entity. Otherwise we set it to Null
	//			for (const auto& n : vsComp.graph->GetNodes())
	//			{
	//				for (auto& a : n->inputs)
	//				{
	//					if (a.data.has_value() && a.data.type() == typeid(Volt::Entity))
	//					{
	//						if (std::any_cast<Volt::Entity>(a.data).GetId() == prefabEntity)
	//						{
	//							Volt::Entity ent = std::any_cast<Volt::Entity>(a.data);
	//							a.data = Volt::Entity{ newEntity, targetScene };
	//						}
	//						else
	//						{
	//							a.data = Volt::Entity{ Wire::NullID, targetScene };
	//						}
	//					}
	//				}

	//				for (auto& a : n->outputs)
	//				{
	//					if (a.data.has_value() && a.data.type() == typeid(Volt::Entity))
	//					{
	//						if (std::any_cast<Volt::Entity>(a.data).GetId() == prefabEntity)
	//						{
	//							Volt::Entity ent = std::any_cast<Volt::Entity>(a.data);
	//							a.data = Volt::Entity{ newEntity, targetScene };
	//						}
	//						else
	//						{
	//							a.data = Volt::Entity{ Wire::NullID, targetScene };
	//						}
	//					}
	//				}
	//			}
	//		}
	//	}

	//	return newEntity;
	//}

	//Wire::EntityId Prefab::AddToPrefab(Wire::Registry& aParentRegistry, Wire::EntityId entity, uint32_t& count)
	//{
	//	auto newEntityId = entity;
	//	while (myRegistry.Exists(newEntityId))
	//	{
	//		newEntityId++;
	//	}
	//	myRegistry.AddEntity(newEntityId);
	//	auto& prefabComp = myRegistry.AddComponent<PrefabComponent>(newEntityId);
	//	prefabComp.prefabAsset = handle;
	//	prefabComp.prefabEntity = newEntityId;

	//	Entity::Copy(aParentRegistry, myRegistry, entity, newEntityId, { Wire::ComponentRegistry::GetRegistryDataFromName("RelationshipComponent").guid });

	//	// Add children
	//	if (!myRegistry.HasComponent<RelationshipComponent>(newEntityId))
	//	{
	//		myRegistry.AddComponent<RelationshipComponent>(newEntityId);
	//	}

	//	if (aParentRegistry.HasComponent<RelationshipComponent>(entity))
	//	{
	//		auto& relComp = aParentRegistry.GetComponent<RelationshipComponent>(entity);

	//		uint32_t tempCount = count;

	//		for (const auto& child : relComp.Children)
	//		{
	//			AddToPrefab(aParentRegistry, child, ++count);

	//			auto& newEntRelComp = myRegistry.GetComponent<RelationshipComponent>(newEntityId);
	//			newEntRelComp.Children.emplace_back(child);
	//		}

	//		auto& newEntRelComp = myRegistry.GetComponent<RelationshipComponent>(newEntityId);

	//		if (tempCount == 0)
	//		{
	//			newEntRelComp.Parent = Wire::NullID;
	//		}
	//		else
	//		{
	//			if (aParentRegistry.HasComponent<PrefabComponent>(relComp.Parent))
	//			{
	//				auto parentId = aParentRegistry.GetComponent<PrefabComponent>(relComp.Parent).prefabEntity;
	//				newEntRelComp.Parent = parentId;
	//				auto& parentChildren = myRegistry.GetComponent<RelationshipComponent>(parentId).Children;
	//				if (std::find(parentChildren.begin(), parentChildren.end(), newEntityId) == parentChildren.end())
	//				{
	//					parentChildren.emplace_back(newEntityId);
	//				}
	//			}
	//			else
	//			{
	//				newEntRelComp.Parent = relComp.Parent;
	//			}
	//		}
	//	}

	//	return newEntityId;
	//}

	//void Prefab::OverrideEntity(Wire::Registry& aSrcRegistry, Wire::EntityId aSrcEntity)
	//{
	//	if (!aSrcRegistry.HasComponent<PrefabComponent>(aSrcEntity))
	//	{
	//		auto& srcPrefabComp = aSrcRegistry.AddComponent<PrefabComponent>(aSrcEntity);

	//		uint32_t count = 1;
	//		auto newEnt = AddToPrefab(aSrcRegistry, aSrcEntity, count);

	//		srcPrefabComp.prefabAsset = handle;
	//		srcPrefabComp.prefabEntity = newEnt;
	//	}

	//	auto& srcPrefabComp = aSrcRegistry.GetComponent<PrefabComponent>(aSrcEntity);
	//	srcPrefabComp.version = myVersion;

	//	if (!myRegistry.Exists(srcPrefabComp.prefabEntity) || srcPrefabComp.prefabAsset != this->handle)
	//	{
	//		return;
	//	}

	//	myKeepEntitiesQueue.emplace_back(srcPrefabComp.prefabEntity);
	//	std::vector<WireGUID> excludedComponents = { Wire::ComponentRegistry::GetRegistryDataFromName("RelationshipComponent").guid };

	//	auto& targetRelationshipComponent = myRegistry.GetComponent<RelationshipComponent>(srcPrefabComp.prefabEntity);
	//	if (targetRelationshipComponent.Parent == Wire::NullID)
	//	{
	//		excludedComponents.emplace_back(Wire::ComponentRegistry::GetRegistryDataFromName("TransformComponent").guid);
	//	}

	//	Entity::Copy(aSrcRegistry, myRegistry, aSrcEntity, srcPrefabComp.prefabEntity, excludedComponents, true);

	//	auto& srcRelationshipComponent = aSrcRegistry.GetComponent<RelationshipComponent>(aSrcEntity);
	//	for (const auto& child : srcRelationshipComponent.Children)
	//	{
	//		OverrideEntity(aSrcRegistry, child);
	//	}

	//	// #TODO(Ivar): Check that parenting is correct in all entities
	//}

	Prefab::Prefab(Scene* scene, Wire::EntityId rootEntity)
	{
		CreatePrefab(scene, rootEntity);
	}

	bool Prefab::IsValidInPrefab(Wire::EntityId aEntityId, AssetHandle aPrefabId)
	{
		Ref<Prefab> prefabAsset = AssetManager::GetAssetLocking<Prefab>(aPrefabId);
		if (!prefabAsset || !prefabAsset->IsValid())
		{
			return false;
		}

		return prefabAsset->myRegistry.Exists(aEntityId);
	}

	bool Prefab::IsRootInPrefab(Wire::EntityId aEntityId, AssetHandle aPrefabId)
	{
		Ref<Prefab> prefabAsset = AssetManager::GetAssetLocking<Prefab>(aPrefabId);
		if (!prefabAsset || !prefabAsset->IsValid())
		{
			return false;
		}

		return aEntityId == prefabAsset->myRootId;
	}

	uint32_t Prefab::GetPrefabVersion(AssetHandle aPrefabId)
	{
		Ref<Prefab> prefabAsset = AssetManager::GetAssetLocking<Prefab>(aPrefabId);
		if (!prefabAsset || !prefabAsset->IsValid())
		{
			return 0;
		}

		return prefabAsset->myVersion;
	}

	void Prefab::PreloadAllPrefabs()
	{
		for (const auto& asset : AssetManager::GetAllAssetsOfType<Prefab>())
		{
			AssetManager::QueueAsset<Prefab>(asset);
		}
	}

	Wire::EntityId Prefab::Instantiate(Scene* targetScene, Wire::EntityId aTargetEntity)
	{
		if (myRootId == Wire::NullID) { return Wire::NullID; }
		auto newEnt = Entity::Duplicate(myRegistry, targetScene->GetRegistry(), myScriptFieldCache, targetScene->GetScriptFieldCache(), myRootId, aTargetEntity, { RelationshipComponent::comp_guid, RigidbodyComponent::comp_guid, CharacterControllerComponent::comp_guid }, true);

		UpdateComponents(targetScene, newEnt);
		CorrectEntityReferences(targetScene, newEnt);

		targetScene->InvalidateEntityTransform(newEnt);

		return newEnt;
	}

	void Prefab::UpdateComponents(Scene* targetScene, Wire::EntityId aTargetEntity)
	{
		auto& targetRegistry = targetScene->GetRegistry();
		Wire::EntityId prefabEntity = targetRegistry.GetComponent<PrefabComponent>(aTargetEntity).prefabEntity;

		if (myRegistry.HasComponent<RigidbodyComponent>(prefabEntity))
		{
			const auto srcComp = myRegistry.GetComponent<RigidbodyComponent>(prefabEntity);
			if (targetRegistry.HasComponent<RigidbodyComponent>(aTargetEntity))
			{
				targetRegistry.RemoveComponent<RigidbodyComponent>(aTargetEntity);
			}

			targetRegistry.AddComponent<RigidbodyComponent>(aTargetEntity, srcComp.bodyType, srcComp.layerId, srcComp.mass, srcComp.linearDrag, srcComp.lockFlags, srcComp.angularDrag, srcComp.disableGravity, srcComp.isKinematic, srcComp.collisionType);
		}

		if (targetRegistry.HasComponent<NetActorComponent>(aTargetEntity))
		{
			auto& srcComp = targetRegistry.GetComponent<NetActorComponent>(aTargetEntity);
			srcComp.repId = Nexus::RandRepID();
		}

		if (myRegistry.HasComponent<CharacterControllerComponent>(prefabEntity))
		{
			const auto srcComp = myRegistry.GetComponent<CharacterControllerComponent>(prefabEntity);
			if (targetRegistry.HasComponent<CharacterControllerComponent>(aTargetEntity))
			{
				targetRegistry.RemoveComponent<CharacterControllerComponent>(aTargetEntity);
			}

			targetRegistry.AddComponent<CharacterControllerComponent>(aTargetEntity, srcComp.climbingMode, srcComp.slopeLimit, srcComp.invisibleWallHeight, srcComp.maxJumpHeight, srcComp.contactOffset, srcComp.stepOffset, srcComp.density, srcComp.layer, srcComp.hasGravity);
		}

		if (targetRegistry.HasComponent<AnimationControllerComponent>(aTargetEntity))
		{
			auto& comp = targetRegistry.GetComponent<AnimationControllerComponent>(aTargetEntity);

			if (comp.animationGraph != Asset::Null())
			{
				auto graph = AssetManager::GetAsset<AnimationGraphAsset>(comp.animationGraph);
				if (graph && graph->IsValid())
				{
					comp.controller = CreateRef<AnimationController>(graph, Entity{ aTargetEntity, targetScene });
				}
			}
		}

		if (targetRegistry.HasComponent<VisualScriptingComponent>(aTargetEntity))
		{
			auto& vsComp = targetRegistry.GetComponent<VisualScriptingComponent>(aTargetEntity);
			if (vsComp.graph)
			{
				// Update entityId if the entity ID is of the prefab entity. Otherwise we set it to Null
				for (const auto& n : vsComp.graph->GetNodes())
				{
					for (auto& a : n->inputs)
					{
						if (a.data.has_value() && a.data.type() == typeid(Volt::Entity))
						{
							if (std::any_cast<Volt::Entity>(a.data).GetId() == prefabEntity)
							{
								Volt::Entity ent = std::any_cast<Volt::Entity>(a.data);
								a.data = Volt::Entity{ aTargetEntity, targetScene };
							}
							else
							{
								a.data = Volt::Entity{ Wire::NullID, targetScene };
							}
						}
					}

					for (auto& a : n->outputs)
					{
						if (a.data.has_value() && a.data.type() == typeid(Volt::Entity))
						{
							if (std::any_cast<Volt::Entity>(a.data).GetId() == prefabEntity)
							{
								Volt::Entity ent = std::any_cast<Volt::Entity>(a.data);
								a.data = Volt::Entity{ aTargetEntity, targetScene };
							}
							else
							{
								a.data = Volt::Entity{ Wire::NullID, targetScene };
							}
						}
					}
				}
			}
		}

		if (targetScene->GetRegistry().HasComponent<EntityDataComponent>(aTargetEntity))
		{
			auto& comp = targetScene->GetRegistry().GetComponent<EntityDataComponent>(aTargetEntity);
			comp.layerId = targetScene->GetActiveLayer();
			comp.randomValue = Random::Float(0.f, 1.f);
			comp.timeSinceCreation = 0.f;
		}
		else
		{
			auto& comp = targetScene->GetRegistry().AddComponent<EntityDataComponent>(aTargetEntity);
			comp.layerId = targetScene->GetActiveLayer();
			comp.randomValue = Random::Float(0.f, 1.f);
			comp.timeSinceCreation = 0.f;
		}

		if (targetRegistry.HasComponent<RelationshipComponent>(aTargetEntity))
		{
			auto& relationshipComp = targetRegistry.GetComponent<RelationshipComponent>(aTargetEntity);

			for (auto child : relationshipComp.Children)
			{
				UpdateComponents(targetScene, child);
			}
		}
	}

	void Prefab::CorrectEntityReferences(Scene* scene, Wire::EntityId targetEntity)
	{
		CorrectEntityReferencesRecursive(scene, targetEntity, targetEntity);
	}

	void Prefab::CorrectEntityReferencesRecursive(Scene* scene, Wire::EntityId targetEntity, Wire::EntityId startEntity)
	{
		auto& targetRegistry = scene->GetRegistry();

		for (const auto& [guid, pool] : targetRegistry.GetPools())
		{
			if (!targetRegistry.HasComponent(guid, targetEntity))
			{
				continue;
			}

			if (guid == RelationshipComponent::comp_guid || guid == PrefabComponent::comp_guid)
			{
				continue;
			}

			const auto& compInfo = Wire::ComponentRegistry::GetRegistryDataFromGUID(guid);
			uint8_t* data = (uint8_t*)targetRegistry.GetComponentPtr(guid, targetEntity);
			for (const auto& prop : compInfo.properties)
			{
				if (!prop.visible)
				{
					continue;
				}

				if (prop.type == Wire::ComponentRegistry::PropertyType::EntityId)
				{
					Wire::EntityId currentEntityId = *(Wire::EntityId*)&data[prop.offset];
					Wire::EntityId correspondingEntity = FindCorrespondingEntity(targetRegistry, startEntity, currentEntityId);

					*(Wire::EntityId*)&data[prop.offset] = correspondingEntity;
				}
			}
		}

		if (targetRegistry.HasComponent<MonoScriptComponent>(targetEntity))
		{
			for (const auto& script : targetRegistry.GetComponent<MonoScriptComponent>(targetEntity).scriptIds)
			{
				const auto& fieldMap = scene->GetScriptFieldCache().GetCache().at(script);

				for (const auto& [name, fieldInstance] : fieldMap)
				{
					if (fieldInstance->field.type == MonoFieldType::Entity)
					{
						Wire::EntityId currentEntityId = *fieldInstance->data.As<Wire::EntityId>();
						Wire::EntityId correspondingEntity = FindCorrespondingEntity(targetRegistry, startEntity, currentEntityId);

						fieldInstance->SetValue(correspondingEntity, sizeof(Wire::EntityId), MonoFieldType::Entity);
					}
				}
			}
		}

		if (targetRegistry.HasComponent<RelationshipComponent>(targetEntity))
		{
			for (const auto& child : targetRegistry.GetComponent<RelationshipComponent>(targetEntity).Children)
			{
				CorrectEntityReferencesRecursive(scene, child, startEntity);
			}
		}
	}

	Wire::EntityId Prefab::FindCorrespondingEntity(Wire::Registry& registry, Wire::EntityId currentEntity, Wire::EntityId wantedPrefabEntity)
	{
		if (!registry.HasComponent<PrefabComponent>(currentEntity))
		{
			return 0;
		}

		if (registry.GetComponent<PrefabComponent>(currentEntity).prefabEntity == wantedPrefabEntity)
		{
			return currentEntity;
		}

		if (registry.HasComponent<RelationshipComponent>(currentEntity))
		{
			for (const auto& child : registry.GetComponent<RelationshipComponent>(currentEntity).Children)
			{
				Wire::EntityId result = FindCorrespondingEntity(registry, child, wantedPrefabEntity);
				if (result != 0)
				{
					return result;
				}
			}
		}

		return 0;
	}

	void Prefab::OverridePrefabAsset(Scene* scene, Wire::EntityId aSrcEntity, AssetHandle aPrefabId)
	{
		const Ref<Prefab> prefabAsset = AssetManager::GetAssetLocking<Prefab>(aPrefabId);
		if (!prefabAsset || !prefabAsset->IsValid())
		{
			return;
		}

		prefabAsset->CreatePrefab(scene, aSrcEntity);
		prefabAsset->myVersion++;
	}

	bool Prefab::CreatePrefab(Scene* scene, Wire::EntityId rootEntity)
	{
		if (!scene->GetRegistry().Exists(rootEntity)) { return false; }

		myRegistry.Clear();
		myRootId = RecursiveAddToPrefab(scene, rootEntity);

		if (myRegistry.HasComponent<RelationshipComponent>(myRootId))
		{
			auto& [Children, Parent] = myRegistry.GetComponent<RelationshipComponent>(myRootId);
			Parent = Wire::NullID;
		}

		return true;
	}

	Wire::EntityId Prefab::RecursiveAddToPrefab(Scene* scene, Wire::EntityId aSrcEntity)
	{
		auto& srcRegistry = scene->GetRegistry();

		Wire::EntityId newEntity = aSrcEntity;
		if (srcRegistry.HasComponent<PrefabComponent>(aSrcEntity))
		{
			newEntity = srcRegistry.GetComponent<PrefabComponent>(aSrcEntity).prefabEntity;
		}
		else
		{
			srcRegistry.AddComponent<PrefabComponent>(aSrcEntity);
		}

		auto& srcPrefabComp = srcRegistry.GetComponent<PrefabComponent>(aSrcEntity);
		srcPrefabComp.prefabAsset = handle;
		srcPrefabComp.prefabEntity = newEntity;
		srcPrefabComp.version = myVersion;

		myRegistry.AddEntity(newEntity);
		Entity::Copy(srcRegistry, myRegistry, scene->GetScriptFieldCache(), myScriptFieldCache, aSrcEntity, newEntity);

		if (srcRegistry.HasComponent<RelationshipComponent>(aSrcEntity))
		{
			auto& srcRelationshipComp = srcRegistry.GetComponent<RelationshipComponent>(aSrcEntity);
			auto& targetRelationshipComp = myRegistry.GetComponent<RelationshipComponent>(newEntity);

			auto srcParent = srcRelationshipComp.Parent;

			if (srcParent != Wire::NullID)
			{
				if (srcRegistry.HasComponent<PrefabComponent>(srcParent))
				{
					targetRelationshipComp.Parent = srcRegistry.GetComponent<PrefabComponent>(srcParent).prefabEntity;
				}
			}

			auto& children = srcRelationshipComp.Children;

			targetRelationshipComp.Children.clear();
			for (auto child : children)
			{
				Wire::EntityId newChild = child;

				if (srcRegistry.HasComponent<PrefabComponent>(child))
				{
					newChild = srcRegistry.GetComponent<PrefabComponent>(child).prefabEntity;
				}

				targetRelationshipComp.Children.emplace_back(newChild);
			}

			for (auto child : children)
			{
				RecursiveAddToPrefab(scene, child);
			}
		}

		return newEntity;
	}

	Wire::EntityId Prefab::UpdateEntity(Scene* targetScene, Wire::EntityId aTargetEntity, AssetHandle prefabHandle)
	{
		if (auto asset = AssetManager::GetAssetLocking<Prefab>(prefabHandle))
		{
			Volt::Entity ent(aTargetEntity, targetScene);

			auto& registry = targetScene->GetRegistry();

			auto oldTransformComp = ent.GetComponent<Volt::TransformComponent>();
			auto oldEntityDataComp = ent.GetComponent<Volt::EntityDataComponent>();

			auto parent = ent.GetComponent<Volt::RelationshipComponent>().Parent;
			targetScene->UnparentEntity(ent);
			targetScene->RemoveEntity(ent);

			targetScene->RemoveEntity({ aTargetEntity, targetScene });
			ent = { asset->Instantiate(targetScene, aTargetEntity), targetScene };

			targetScene->ParentEntity(Volt::Entity(parent, targetScene), ent);

			if (registry.HasComponent<Volt::TransformComponent>(ent.GetId()))
			{
				auto& newTransformComp = registry.GetComponent<Volt::TransformComponent>(ent.GetId());
				newTransformComp = oldTransformComp;
			}

			targetScene->MoveToLayer(ent, oldEntityDataComp.layerId);
		}

		return Wire::NullID;
	}
}
