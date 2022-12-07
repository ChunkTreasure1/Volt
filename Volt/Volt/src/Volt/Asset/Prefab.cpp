#include "vtpch.h"
#include "Prefab.h"

#include "Volt/Asset/AssetManager.h"

#include "Volt/Components/Components.h"
#include "Volt/Scene/Entity.h"

namespace Volt
{
	Prefab::Prefab(Wire::Registry& aParentRegistry, Wire::EntityId topEntity)
	{
		uint32_t count = 0;
		AddToPrefab(aParentRegistry, topEntity, count);
	}

	Wire::EntityId Prefab::Instantiate(Wire::Registry& aTargetRegistry)
	{
		Wire::EntityId rootEntity = Wire::NullID;
		for (const auto& ent : myRegistry.GetAllEntities())
		{
			if (myRegistry.HasComponent<RelationshipComponent>(ent))
			{
				auto& relComp = myRegistry.GetComponent<RelationshipComponent>(ent);
				if (relComp.Parent == Wire::NullID)
				{
					rootEntity = ent;
					break;
				}
			}
		}

		Wire::EntityId newRoot = InstantiateEntity(aTargetRegistry, rootEntity, Wire::NullID);

		auto& rootTransform = aTargetRegistry.GetComponent<TransformComponent>(newRoot);
		rootTransform.position = { 0.f, 0.f, 0.f };
		rootTransform.rotation = { 0.f, 0.f, 0.f };
		rootTransform.scale = aTargetRegistry.GetComponent<TransformComponent>(newRoot).scale;

		auto& tagComp = aTargetRegistry.GetComponent<TagComponent>(newRoot);
		tagComp.tag = path.stem().string();

		return newRoot;
	}

	void Prefab::OverridePrefabInRegistry(Wire::Registry& aTargetRegistry, Wire::EntityId aRootEntity, AssetHandle prefabId)
	{
		Ref<Prefab> prefabAsset = AssetManager::GetAsset<Prefab>(prefabId);
		if (!prefabAsset || !prefabAsset->IsValid())
		{
			VT_CORE_WARN("[Prefab]: Trying to override prefabs with ID {0}, but the prefab is not valid!", prefabId);
			return;
		}

		if (!aTargetRegistry.HasComponent<PrefabComponent>(aRootEntity))
		{
			VT_CORE_WARN("[Prefab]: Root entity is not a prefab!");
			return;
		}

		OverrideEntityInPrefab(aTargetRegistry, aRootEntity, prefabAsset);
	}

	void Prefab::OverridePrefabsInRegistry(Wire::Registry& aTargetRegistry, AssetHandle aPrefabId)
	{
		for (const auto& entity : aTargetRegistry.GetAllEntities())
		{
			if (!aTargetRegistry.HasComponent<PrefabComponent>(entity))
			{
				continue;
			}

			auto& prefabComp = aTargetRegistry.GetComponent<PrefabComponent>(entity);
			if (prefabComp.prefabAsset != aPrefabId)
			{
				continue;
			}

			if (aTargetRegistry.HasComponent<RelationshipComponent>(entity))
			{
				auto& relComp = aTargetRegistry.GetComponent<RelationshipComponent>(entity);
				if (relComp.Parent != Wire::NullID)
				{
					if (aTargetRegistry.HasComponent<PrefabComponent>(relComp.Parent))
					{
						auto& parentPrefabComp = aTargetRegistry.GetComponent<PrefabComponent>(relComp.Parent);
						if (parentPrefabComp.prefabAsset == prefabComp.prefabAsset)
						{
							continue;
						}
					}
				}
			}

			OverridePrefabInRegistry(aTargetRegistry, entity, aPrefabId);
		}
	}

	void Prefab::OverridePrefabAsset(Wire::Registry& aSrcRegistry, Wire::EntityId aSrcEntity, AssetHandle aPrefabId)
	{
		Ref<Prefab> prefabAsset = AssetManager::GetAsset<Prefab>(aPrefabId);
		if (!prefabAsset || !prefabAsset->IsValid())
		{
			VT_CORE_WARN("[Prefab]: Trying to override prefab with ID {0}, but the prefab is not valid!", aPrefabId);
			return;
		}

		if (!aSrcRegistry.HasComponent<PrefabComponent>(aSrcEntity))
		{
			VT_CORE_WARN("[Prefab]: Root entity is not a prefab!");
			return;
		}

		prefabAsset->myVersion++;
		prefabAsset->OverrideEntity(aSrcRegistry, aSrcEntity);
		OverridePrefabsInRegistry(aSrcRegistry, aPrefabId);
	}

	bool Prefab::IsParentInPrefab(Wire::EntityId aEntityId, AssetHandle aPrefabId)
	{
		Ref<Prefab> prefabAsset = AssetManager::GetAsset<Prefab>(aPrefabId);
		if (!prefabAsset || !prefabAsset->IsValid())
		{
			return false;
		}

		Wire::EntityId rootEntity = Wire::NullID;
		for (const auto& ent : prefabAsset->myRegistry.GetAllEntities())
		{
			if (prefabAsset->myRegistry.HasComponent<RelationshipComponent>(ent))
			{
				auto& relComp = prefabAsset->myRegistry.GetComponent<RelationshipComponent>(ent);
				if (relComp.Parent == Wire::NullID)
				{
					rootEntity = ent;
					break;
				}
			}
		}

		if (aEntityId == rootEntity)
		{
			return true;
		}

		return false;
	}

	uint32_t Prefab::GetPrefabVersion(AssetHandle aPrefabId)
	{
		Ref<Prefab> prefabAsset = AssetManager::GetAsset<Prefab>(aPrefabId);
		if (!prefabAsset || !prefabAsset->IsValid())
		{
			return 0;
		}

		return prefabAsset->myVersion;
	}

	void Prefab::OverrideEntityInPrefab(Wire::Registry& aTargetRegistry, Wire::EntityId aEntity, Ref<Prefab> aPrefab)
	{
		if (!aTargetRegistry.HasComponent<PrefabComponent>(aEntity))
		{
			return;
		}

		auto& srcRegistry = aPrefab->myRegistry;
		auto& targetPrefabComp = aTargetRegistry.GetComponent<PrefabComponent>(aEntity);

		if (!srcRegistry.Exists(targetPrefabComp.prefabEntity) || targetPrefabComp.prefabAsset != aPrefab->handle)
		{
			return;
		}

		std::vector<WireGUID> excludedComponents = { Wire::ComponentRegistry::GetRegistryDataFromName("RelationshipComponent").guid, Wire::ComponentRegistry::GetRegistryDataFromName("PrefabComponent").guid };

		auto& targetRelationshipComponent = aTargetRegistry.GetComponent<RelationshipComponent>(aEntity);
		if (targetRelationshipComponent.Parent == Wire::NullID)
		{
			excludedComponents.emplace_back(Wire::ComponentRegistry::GetRegistryDataFromName("TransformComponent").guid);
		}

		Entity::Copy(srcRegistry, aTargetRegistry, targetPrefabComp.prefabEntity, aEntity, excludedComponents, true);

		for (const auto& child : targetRelationshipComponent.Children)
		{
			OverrideEntityInPrefab(aTargetRegistry, child, aPrefab);
		}

		// #TODO(Ivar): Check that parenting is correct in all entities
	}

	Wire::EntityId Prefab::InstantiateEntity(Wire::Registry& aTargetRegistry, Wire::EntityId prefabEntity, Wire::EntityId parentEntity)
	{
		Wire::EntityId newEntity = aTargetRegistry.CreateEntity();
		Entity::Copy(myRegistry, aTargetRegistry, prefabEntity, newEntity, {}, true);

		if (myRegistry.HasComponent<RelationshipComponent>(prefabEntity))
		{
			auto& relComp = myRegistry.GetComponent<RelationshipComponent>(prefabEntity);
			if (!aTargetRegistry.HasComponent<RelationshipComponent>(newEntity))
			{
				aTargetRegistry.AddComponent<RelationshipComponent>(newEntity);
			}

			{
				auto& relCompNew = aTargetRegistry.GetComponent<RelationshipComponent>(newEntity);
				relCompNew.Children.clear();
				relCompNew.Parent = parentEntity;
			}

			for (const auto& child : relComp.Children)
			{
				Wire::EntityId entity = InstantiateEntity(aTargetRegistry, child, newEntity);
				auto& relCompNew = aTargetRegistry.GetComponent<RelationshipComponent>(newEntity);
				relCompNew.Children.emplace_back(entity);
			}
		}

		return newEntity;
	}

	void Prefab::AddToPrefab(Wire::Registry& aParentRegistry, Wire::EntityId entity, uint32_t& count)
	{
		myRegistry.AddEntity(entity);
		auto& prefabComp = myRegistry.AddComponent<PrefabComponent>(entity);
		prefabComp.prefabAsset = handle;
		prefabComp.prefabEntity = entity;

		Entity::Copy(aParentRegistry, myRegistry, entity, entity, { Wire::ComponentRegistry::GetRegistryDataFromName("RelationshipComponent").guid });

		// Add children
		if (!myRegistry.HasComponent<RelationshipComponent>(entity))
		{
			myRegistry.AddComponent<RelationshipComponent>(entity);
		}

		if (aParentRegistry.HasComponent<RelationshipComponent>(entity))
		{
			auto& relComp = aParentRegistry.GetComponent<RelationshipComponent>(entity);

			uint32_t tempCount = count;

			for (const auto& child : relComp.Children)
			{
				AddToPrefab(aParentRegistry, child, ++count);

				auto& newEntRelComp = myRegistry.GetComponent<RelationshipComponent>(entity);
				newEntRelComp.Children.emplace_back(child);
			}

			auto& newEntRelComp = myRegistry.GetComponent<RelationshipComponent>(entity);

			if (tempCount == 0)
			{
				newEntRelComp.Parent = Wire::NullID;
			}
			else
			{
				newEntRelComp.Parent = relComp.Parent;
			}
		}
	}

	void Prefab::OverrideEntity(Wire::Registry& aSrcRegistry, Wire::EntityId aSrcEntity)
	{
		if (!aSrcRegistry.HasComponent<PrefabComponent>(aSrcEntity))
		{
			return;
		}

		auto& srcPrefabComp = aSrcRegistry.GetComponent<PrefabComponent>(aSrcEntity);
		srcPrefabComp.version = myVersion;

		if (!myRegistry.Exists(srcPrefabComp.prefabEntity) || srcPrefabComp.prefabAsset != this->handle)
		{
			return;
		}

		std::vector<WireGUID> excludedComponents = { Wire::ComponentRegistry::GetRegistryDataFromName("RelationshipComponent").guid };

		auto& targetRelationshipComponent = myRegistry.GetComponent<RelationshipComponent>(srcPrefabComp.prefabEntity);
		if (targetRelationshipComponent.Parent == Wire::NullID)
		{
			excludedComponents.emplace_back(Wire::ComponentRegistry::GetRegistryDataFromName("TransformComponent").guid);
		}

		Entity::Copy(aSrcRegistry, myRegistry, aSrcEntity, srcPrefabComp.prefabEntity, excludedComponents, true);

		auto& srcRelationshipComponent = aSrcRegistry.GetComponent<RelationshipComponent>(aSrcEntity);
		for (const auto& child : srcRelationshipComponent.Children)
		{
			OverrideEntity(aSrcRegistry, child);
		}

		// #TODO(Ivar): Check that parenting is correct in all entities
	}
}