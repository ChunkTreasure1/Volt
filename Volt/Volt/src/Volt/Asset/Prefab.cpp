#include "vtpch.h"
#include "Prefab.h"

#include "Volt/Asset/AssetManager.h"

#include "Volt/Components/PhysicsComponents.h"
#include "Volt/Components/CoreComponents.h"
#include "Volt/Components/RenderingComponents.h"

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
	Prefab::Prefab(Scene* scene, entt::entity rootEntity)
	{
		CreatePrefab(scene, rootEntity);
	}

	bool Prefab::IsValidInPrefab(entt::entity aEntityId, AssetHandle aPrefabId)
	{
		Ref<Prefab> prefabAsset = AssetManager::GetAssetLocking<Prefab>(aPrefabId);
		if (!prefabAsset || !prefabAsset->IsValid())
		{
			return false;
		}

		return prefabAsset->myRegistry.valid(aEntityId);
	}

	bool Prefab::IsRootInPrefab(entt::entity aEntityId, AssetHandle aPrefabId)
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

	entt::entity Prefab::Instantiate(Scene* targetScene, entt::entity aTargetEntity)
	{
		if (myRootId == entt::null) { return entt::null; }
		//auto newEnt = Entity::Duplicate(myRegistry, targetScene->GetRegistry(), myScriptFieldCache, targetScene->GetScriptFieldCache(), myRootId, aTargetEntity, { RelationshipComponent::comp_guid, RigidbodyComponent::comp_guid, CharacterControllerComponent::comp_guid }, true);

		//UpdateComponents(targetScene, newEnt);
		//CorrectEntityReferences(targetScene, newEnt);

		//targetScene->InvalidateEntityTransform(newEnt);

		return entt::null;
	}

	void Prefab::UpdateComponents(Scene* targetScene, entt::entity aTargetEntity)
	{
		Entity targetEntity = { aTargetEntity, targetScene };
		Entity prefabEntity = { targetEntity.GetComponent<PrefabComponent>().prefabEntity, targetScene };

		if (prefabEntity.HasComponent<RigidbodyComponent>())
		{
			const auto srcComp = prefabEntity.GetComponent<RigidbodyComponent>();
			if (targetEntity.HasComponent<RigidbodyComponent>())
			{
				targetEntity.RemoveComponent<RigidbodyComponent>();
			}

			targetEntity.AddComponent<RigidbodyComponent>(srcComp.bodyType, srcComp.layerId, srcComp.mass, srcComp.linearDrag, srcComp.lockFlags, srcComp.angularDrag, srcComp.disableGravity, srcComp.isKinematic, srcComp.collisionType);
		}

		if (targetEntity.HasComponent<NetActorComponent>())
		{
			auto& srcComp = targetEntity.GetComponent<NetActorComponent>();
			srcComp.repId = Nexus::RandRepID();
		}

		if (prefabEntity.HasComponent<CharacterControllerComponent>())
		{
			const auto srcComp = prefabEntity.GetComponent<CharacterControllerComponent>();
			if (targetEntity.HasComponent<CharacterControllerComponent>())
			{
				targetEntity.RemoveComponent<CharacterControllerComponent>();
			}

			targetEntity.AddComponent<CharacterControllerComponent>(srcComp.climbingMode, srcComp.slopeLimit, srcComp.invisibleWallHeight, srcComp.maxJumpHeight, srcComp.contactOffset, srcComp.stepOffset, srcComp.density, srcComp.layer, srcComp.hasGravity);
		}

		if (targetEntity.HasComponent<AnimationControllerComponent>())
		{
			auto& comp = targetEntity.GetComponent<AnimationControllerComponent>();

			if (comp.animationGraph != Asset::Null())
			{
				auto graph = AssetManager::GetAsset<AnimationGraphAsset>(comp.animationGraph);
				if (graph && graph->IsValid())
				{
					comp.controller = CreateRef<AnimationController>(graph, Entity{ aTargetEntity, targetScene });
				}
			}
		}

		if (targetEntity.HasComponent<CommonComponent>())
		{
			auto& comp = targetEntity.GetComponent<CommonComponent>();
			comp.layerId = targetScene->GetActiveLayer();
			comp.randomValue = Random::Float(0.f, 1.f);
			comp.timeSinceCreation = 0.f;
		}
		else
		{
			auto& comp = targetEntity.AddComponent<CommonComponent>();
			comp.layerId = targetScene->GetActiveLayer();
			comp.randomValue = Random::Float(0.f, 1.f);
			comp.timeSinceCreation = 0.f;
		}

		if (targetEntity.HasComponent<RelationshipComponent>())
		{
			auto& relationshipComp = targetEntity.GetComponent<RelationshipComponent>();

			for (auto child : relationshipComp.children)
			{
				UpdateComponents(targetScene, child);
			}
		}
	}

	void Prefab::CorrectEntityReferences(Scene* scene, entt::entity targetEntity)
	{
		CorrectEntityReferencesRecursive(scene, targetEntity, targetEntity);
	}

	void Prefab::CorrectEntityReferencesRecursive(Scene* scene, entt::entity targetEntityId, entt::entity startEntityId)
	{
		Entity targetEntity = { targetEntityId, scene };
		Entity startEntity = { startEntityId, scene };

		/*for (const auto& [guid, pool] : targetRegistry.GetPools())
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
					entt::entity currentEntityId = *(entt::entity*)&data[prop.offset];
					entt::entity correspondingEntity = FindCorrespondingEntity(targetRegistry, startEntity, currentEntityId);

					*(entt::entity*)&data[prop.offset] = correspondingEntity;
				}
			}
		}*/

		if (targetEntity.HasComponent<MonoScriptComponent>())
		{
			for (const auto& script : targetEntity.GetComponent<MonoScriptComponent>().scriptIds)
			{
				const auto& fieldMap = scene->GetScriptFieldCache().GetCache().at(script);

				for (const auto& [fieldName, fieldInstance] : fieldMap)
				{
					if (fieldInstance->field.type == MonoFieldType::Entity)
					{
						entt::entity currentEntityId = *fieldInstance->data.As<entt::entity>();
						entt::entity correspondingEntity = FindCorrespondingEntity(scene, startEntity.GetID(), currentEntityId);

						fieldInstance->SetValue(correspondingEntity, sizeof(entt::entity), MonoFieldType::Entity);
					}
				}
			}
		}

		if (targetEntity.HasComponent<RelationshipComponent>())
		{
			for (const auto& child : targetEntity.GetComponent<RelationshipComponent>().children)
			{
				CorrectEntityReferencesRecursive(scene, child, startEntity.GetID());
			}
		}
	}

	entt::entity Prefab::FindCorrespondingEntity(Scene* scene, entt::entity currentEntityId, entt::entity wantedPrefabEntityId)
	{
		Entity currentEntity = { currentEntityId, scene };
		Entity wantedPrefabEntity = { wantedPrefabEntityId, scene };

		if (!currentEntity.HasComponent<PrefabComponent>())
		{
			return entt::null;
		}

		if (currentEntity.GetComponent<PrefabComponent>().prefabEntity == wantedPrefabEntityId)
		{
			return currentEntity.GetID();
		}

		if (currentEntity.HasComponent<RelationshipComponent>())
		{
			for (const auto& child : currentEntity.GetComponent<RelationshipComponent>().children)
			{
				entt::entity result = FindCorrespondingEntity(scene, child, wantedPrefabEntity.GetID());
				if (result != entt::null)
				{
					return result;
				}
			}
		}

		return entt::null;
	}

	void Prefab::OverridePrefabAsset(Scene* scene, entt::entity aSrcEntity, AssetHandle aPrefabId)
	{
		const Ref<Prefab> prefabAsset = AssetManager::GetAssetLocking<Prefab>(aPrefabId);
		if (!prefabAsset || !prefabAsset->IsValid())
		{
			return;
		}

		prefabAsset->CreatePrefab(scene, aSrcEntity);
		prefabAsset->myVersion++;
	}

	bool Prefab::CreatePrefab(Scene* scene, entt::entity rootEntityId)
	{
		if (!scene->GetRegistry().valid(rootEntityId)) { return false; }

		myRegistry.clear();
		myRootId = RecursiveAddToPrefab(scene, rootEntityId);

		Entity rootEntity = { myRootId, scene };

		if (rootEntity.HasComponent<RelationshipComponent>())
		{
			auto& [parent, children] = rootEntity.GetComponent<RelationshipComponent>();
			parent = entt::null;
		}

		return true;
	}

	entt::entity Prefab::RecursiveAddToPrefab(Scene* scene, entt::entity aSrcEntity)
	{
		//auto& srcRegistry = scene->GetRegistry();

		//Entity srcEntity = { aSrcEntity, scene };
		//entt::entity newEntity = srcEntity.GetID();
		//
		//if (srcEntity.HasComponent<PrefabComponent>())
		//{
		//	newEntity = srcEntity.GetComponent<PrefabComponent>().prefabEntity;
		//}
		//else
		//{
		//	srcEntity.AddComponent<PrefabComponent>();
		//}

		//auto& srcPrefabComp = srcEntity.GetComponent<PrefabComponent>();
		//srcPrefabComp.prefabAsset = handle;
		//srcPrefabComp.prefabEntity = newEntity;
		//srcPrefabComp.version = myVersion;

		//myRegistry.create(newEntity);
		//Entity::Copy(srcRegistry, myRegistry, scene->GetScriptFieldCache(), myScriptFieldCache, aSrcEntity, newEntity);

	/*	if (srcEntity.HasComponent<RelationshipComponent>())
		{
			auto& srcRelationshipComp = srcEntity.GetComponent<RelationshipComponent>();
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
				entt::entity newChild = child;

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
		}*/

		return entt::null;
	}

	entt::entity Prefab::UpdateEntity(Scene* targetScene, entt::entity aTargetEntity, AssetHandle prefabHandle)
	{
		/*if (auto asset = AssetManager::GetAssetLocking<Prefab>(prefabHandle))
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
		}*/

		return entt::null;
	}
}
