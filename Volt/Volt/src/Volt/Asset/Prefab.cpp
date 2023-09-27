#include "vtpch.h"
//#include "Prefab.h"
//
//#include "Volt/Asset/AssetManager.h"
//
//#include "Volt/Components/PhysicsComponents.h"
//#include "Volt/Components/CoreComponents.h"
//#include "Volt/Components/RenderingComponents.h"
//
//#include "Volt/Animation/AnimationController.h"
//
//#include "Volt/Asset/Animation/AnimationGraphAsset.h"
//#include "Volt/Scene/Entity.h"
//
//#include "Volt/Scripting/Mono/MonoScriptEngine.h"
//#include "Volt/Net/SceneInteraction/NetActorComponent.h"
//
//#include "Volt/Utility/Random.h"
//
//#include <GraphKey/Graph.h>
//#include <GraphKey/Node.h>
//
//namespace Volt
//{
//	Prefab::Prefab(Scene* scene, entt::entity rootEntity)
//	{
//		CreatePrefab(scene, rootEntity);
//	}
//
//	bool Prefab::IsValidInPrefab(entt::entity aEntityId, AssetHandle aPrefabId)
//	{
//		Ref<Prefab> prefabAsset = AssetManager::GetAssetLocking<Prefab>(aPrefabId);
//		if (!prefabAsset || !prefabAsset->IsValid())
//		{
//			return false;
//		}
//
//		return prefabAsset->myRegistry.valid(aEntityId);
//	}
//
//	bool Prefab::IsRootInPrefab(entt::entity aEntityId, AssetHandle aPrefabId)
//	{
//		Ref<Prefab> prefabAsset = AssetManager::GetAssetLocking<Prefab>(aPrefabId);
//		if (!prefabAsset || !prefabAsset->IsValid())
//		{
//			return false;
//		}
//
//		return aEntityId == prefabAsset->myRootId;
//	}
//
//	uint32_t Prefab::GetPrefabVersion(AssetHandle aPrefabId)
//	{
//		Ref<Prefab> prefabAsset = AssetManager::GetAssetLocking<Prefab>(aPrefabId);
//		if (!prefabAsset || !prefabAsset->IsValid())
//		{
//			return 0;
//		}
//
//		return prefabAsset->myVersion;
//	}
//
//	void Prefab::PreloadAllPrefabs()
//	{
//		for (const auto& asset : AssetManager::GetAllAssetsOfType<Prefab>())
//		{
//			AssetManager::QueueAsset<Prefab>(asset);
//		}
//	}
//
//	entt::entity Prefab::Instantiate(Scene* targetScene, entt::entity aTargetEntity)
//	{
//		if (myRootId == entt::null) { return entt::null; }
//		//auto newEnt = Entity::Duplicate(myRegistry, targetScene->GetRegistry(), myScriptFieldCache, targetScene->GetScriptFieldCache(), myRootId, aTargetEntity, { RelationshipComponent::comp_guid, RigidbodyComponent::comp_guid, CharacterControllerComponent::comp_guid }, true);
//
//		//UpdateComponents(targetScene, newEnt);
//		//CorrectEntityReferences(targetScene, newEnt);
//
//		//targetScene->InvalidateEntityTransform(newEnt);
//
//		return entt::null;
//	}
//
//	void Prefab::UpdateComponents(Scene* targetScene, entt::entity aTargetEntity)
//	{
//		Entity targetEntity = { aTargetEntity, targetScene };
//		Entity prefabEntity = { targetEntity.GetComponent<PrefabComponent>().prefabEntity, targetScene };
//
//		if (prefabEntity.HasComponent<RigidbodyComponent>())
//		{
//			const auto srcComp = prefabEntity.GetComponent<RigidbodyComponent>();
//			if (targetEntity.HasComponent<RigidbodyComponent>())
//			{
//				targetEntity.RemoveComponent<RigidbodyComponent>();
//			}
//
//			targetEntity.AddComponent<RigidbodyComponent>(srcComp.bodyType, srcComp.layerId, srcComp.mass, srcComp.linearDrag, srcComp.lockFlags, srcComp.angularDrag, srcComp.disableGravity, srcComp.isKinematic, srcComp.collisionType);
//		}
//
//		if (targetEntity.HasComponent<NetActorComponent>())
//		{
//			auto& srcComp = targetEntity.GetComponent<NetActorComponent>();
//			srcComp.repId = Nexus::RandRepID();
//		}
//
//		if (prefabEntity.HasComponent<CharacterControllerComponent>())
//		{
//			const auto srcComp = prefabEntity.GetComponent<CharacterControllerComponent>();
//			if (targetEntity.HasComponent<CharacterControllerComponent>())
//			{
//				targetEntity.RemoveComponent<CharacterControllerComponent>();
//			}
//
//			targetEntity.AddComponent<CharacterControllerComponent>(srcComp.climbingMode, srcComp.slopeLimit, srcComp.invisibleWallHeight, srcComp.maxJumpHeight, srcComp.contactOffset, srcComp.stepOffset, srcComp.density, srcComp.layer, srcComp.hasGravity);
//		}
//
//		if (targetEntity.HasComponent<AnimationControllerComponent>())
//		{
//			auto& comp = targetEntity.GetComponent<AnimationControllerComponent>();
//
//			if (comp.animationGraph != Asset::Null())
//			{
//				auto graph = AssetManager::GetAsset<AnimationGraphAsset>(comp.animationGraph);
//				if (graph && graph->IsValid())
//				{
//					comp.controller = CreateRef<AnimationController>(graph, Entity{ aTargetEntity, targetScene });
//				}
//			}
//		}
//
//		if (targetEntity.HasComponent<CommonComponent>())
//		{
//			auto& comp = targetEntity.GetComponent<CommonComponent>();
//			comp.layerId = targetScene->GetActiveLayer();
//			comp.randomValue = Random::Float(0.f, 1.f);
//			comp.timeSinceCreation = 0.f;
//		}
//		else
//		{
//			auto& comp = targetEntity.AddComponent<CommonComponent>();
//			comp.layerId = targetScene->GetActiveLayer();
//			comp.randomValue = Random::Float(0.f, 1.f);
//			comp.timeSinceCreation = 0.f;
//		}
//
//		if (targetEntity.HasComponent<RelationshipComponent>())
//		{
//			auto& relationshipComp = targetEntity.GetComponent<RelationshipComponent>();
//
//			for (auto child : relationshipComp.children)
//			{
//				UpdateComponents(targetScene, child);
//			}
//		}
//	}
//
//	void Prefab::CorrectEntityReferences(Scene* scene, entt::entity targetEntity)
//	{
//		CorrectEntityReferencesRecursive(scene, targetEntity, targetEntity);
//	}
//
//	void Prefab::CorrectEntityReferencesRecursive(Scene* scene, entt::entity targetEntityId, entt::entity startEntityId)
//	{
//		Entity targetEntity = { targetEntityId, scene };
//		Entity startEntity = { startEntityId, scene };
//
//		/*for (const auto& [guid, pool] : targetRegistry.GetPools())
//		{
//			if (!targetRegistry.HasComponent(guid, targetEntity))
//			{
//				continue;
//			}
//
//			if (guid == RelationshipComponent::comp_guid || guid == PrefabComponent::comp_guid)
//			{
//				continue;
//			}
//
//			const auto& compInfo = Wire::ComponentRegistry::GetRegistryDataFromGUID(guid);
//			uint8_t* data = (uint8_t*)targetRegistry.GetComponentPtr(guid, targetEntity);
//			for (const auto& prop : compInfo.properties)
//			{
//				if (!prop.visible)
//				{
//					continue;
//				}
//
//				if (prop.type == Wire::ComponentRegistry::PropertyType::EntityId)
//				{
//					entt::entity currentEntityId = *(entt::entity*)&data[prop.offset];
//					entt::entity correspondingEntity = FindCorrespondingEntity(targetRegistry, startEntity, currentEntityId);
//
//					*(entt::entity*)&data[prop.offset] = correspondingEntity;
//				}
//			}
//		}*/
//
//		if (targetEntity.HasComponent<MonoScriptComponent>())
//		{
//			for (const auto& script : targetEntity.GetComponent<MonoScriptComponent>().scriptIds)
//			{
//				const auto& fieldMap = scene->GetScriptFieldCache().GetCache().at(script);
//
//				for (const auto& [fieldName, fieldInstance] : fieldMap)
//				{
//					if (fieldInstance->field.type.IsEntity())
//					{
//						entt::entity currentEntityId = *fieldInstance->data.As<entt::entity>();
//						entt::entity correspondingEntity = FindCorrespondingEntity(scene, startEntity.GetID(), currentEntityId);
//
//						fieldInstance->SetValue(correspondingEntity, sizeof(entt::entity));
//					}
//				}
//			}
//		}
//
//		if (targetEntity.HasComponent<RelationshipComponent>())
//		{
//			for (const auto& child : targetEntity.GetComponent<RelationshipComponent>().children)
//			{
//				CorrectEntityReferencesRecursive(scene, child, startEntity.GetID());
//			}
//		}
//	}
//
//	entt::entity Prefab::FindCorrespondingEntity(Scene* scene, entt::entity currentEntityId, entt::entity wantedPrefabEntityId)
//	{
//		Entity currentEntity = { currentEntityId, scene };
//		Entity wantedPrefabEntity = { wantedPrefabEntityId, scene };
//
//		if (!currentEntity.HasComponent<PrefabComponent>())
//		{
//			return entt::null;
//		}
//
//		if (currentEntity.GetComponent<PrefabComponent>().prefabEntity == wantedPrefabEntityId)
//		{
//			return currentEntity.GetID();
//		}
//
//		if (currentEntity.HasComponent<RelationshipComponent>())
//		{
//			for (const auto& child : currentEntity.GetComponent<RelationshipComponent>().children)
//			{
//				entt::entity result = FindCorrespondingEntity(scene, child, wantedPrefabEntity.GetID());
//				if (result != entt::null)
//				{
//					return result;
//				}
//			}
//		}
//
//		return entt::null;
//	}
//
//	void Prefab::OverridePrefabAsset(Scene* scene, entt::entity aSrcEntity, AssetHandle aPrefabId)
//	{
//		const Ref<Prefab> prefabAsset = AssetManager::GetAssetLocking<Prefab>(aPrefabId);
//		if (!prefabAsset || !prefabAsset->IsValid())
//		{
//			return;
//		}
//
//		prefabAsset->CreatePrefab(scene, aSrcEntity);
//		prefabAsset->myVersion++;
//	}
//
//	bool Prefab::CreatePrefab(Scene* scene, entt::entity rootEntityId)
//	{
//		if (!scene->GetRegistry().valid(rootEntityId)) { return false; }
//
//		myRegistry.clear();
//		myRootId = RecursiveAddToPrefab(scene, rootEntityId);
//
//		Entity rootEntity = { myRootId, scene };
//
//		if (rootEntity.HasComponent<RelationshipComponent>())
//		{
//			auto& [parent, children] = rootEntity.GetComponent<RelationshipComponent>();
//			parent = entt::null;
//		}
//
//		return true;
//	}
//
//	entt::entity Prefab::RecursiveAddToPrefab(Scene* scene, entt::entity aSrcEntity)
//	{
//		//auto& srcRegistry = scene->GetRegistry();
//
//		//Entity srcEntity = { aSrcEntity, scene };
//		//entt::entity newEntity = srcEntity.GetID();
//		//
//		//if (srcEntity.HasComponent<PrefabComponent>())
//		//{
//		//	newEntity = srcEntity.GetComponent<PrefabComponent>().prefabEntity;
//		//}
//		//else
//		//{
//		//	srcEntity.AddComponent<PrefabComponent>();
//		//}
//
//		//auto& srcPrefabComp = srcEntity.GetComponent<PrefabComponent>();
//		//srcPrefabComp.prefabAsset = handle;
//		//srcPrefabComp.prefabEntity = newEntity;
//		//srcPrefabComp.version = myVersion;
//
//		//myRegistry.create(newEntity);
//		//Entity::Copy(srcRegistry, myRegistry, scene->GetScriptFieldCache(), myScriptFieldCache, aSrcEntity, newEntity);
//
//	/*	if (srcEntity.HasComponent<RelationshipComponent>())
//		{
//			auto& srcRelationshipComp = srcEntity.GetComponent<RelationshipComponent>();
//			auto& targetRelationshipComp = myRegistry.GetComponent<RelationshipComponent>(newEntity);
//
//			auto srcParent = srcRelationshipComp.Parent;
//
//			if (srcParent != Wire::NullID)
//			{
//				if (srcRegistry.HasComponent<PrefabComponent>(srcParent))
//				{
//					targetRelationshipComp.Parent = srcRegistry.GetComponent<PrefabComponent>(srcParent).prefabEntity;
//				}
//			}
//
//			auto& children = srcRelationshipComp.Children;
//
//			targetRelationshipComp.Children.clear();
//			for (auto child : children)
//			{
//				entt::entity newChild = child;
//
//				if (srcRegistry.HasComponent<PrefabComponent>(child))
//				{
//					newChild = srcRegistry.GetComponent<PrefabComponent>(child).prefabEntity;
//				}
//
//				targetRelationshipComp.Children.emplace_back(newChild);
//			}
//
//			for (auto child : children)
//			{
//				RecursiveAddToPrefab(scene, child);
//			}
//		}*/
//
//		return entt::null;
//	}
//
//	entt::entity Prefab::UpdateEntity(Scene* targetScene, entt::entity aTargetEntity, AssetHandle prefabHandle)
//	{
//		/*if (auto asset = AssetManager::GetAssetLocking<Prefab>(prefabHandle))
//		{
//			Volt::Entity ent(aTargetEntity, targetScene);
//
//			auto& registry = targetScene->GetRegistry();
//
//			auto oldTransformComp = ent.GetComponent<Volt::TransformComponent>();
//			auto oldEntityDataComp = ent.GetComponent<Volt::EntityDataComponent>();
//
//			auto parent = ent.GetComponent<Volt::RelationshipComponent>().Parent;
//			targetScene->UnparentEntity(ent);
//			targetScene->RemoveEntity(ent);
//
//			targetScene->RemoveEntity({ aTargetEntity, targetScene });
//			ent = { asset->Instantiate(targetScene, aTargetEntity), targetScene };
//
//			targetScene->ParentEntity(Volt::Entity(parent, targetScene), ent);
//
//			if (registry.HasComponent<Volt::TransformComponent>(ent.GetId()))
//			{
//				auto& newTransformComp = registry.GetComponent<Volt::TransformComponent>(ent.GetId());
//				newTransformComp = oldTransformComp;
//			}
//
//			targetScene->MoveToLayer(ent, oldEntityDataComp.layerId);
//		}*/
//
//		return entt::null;
//	}
//}

#include "Prefab.h"

#include "Volt/Asset/Asset.h"
#include "Volt/Asset/AssetManager.h"
#include "Volt/Asset/Animation/AnimationGraphAsset.h"

#include "Volt/Animation/AnimationController.h"

#include "Volt/Components/CoreComponents.h"
#include "Volt/Components/PhysicsComponents.h"
#include "Volt/Components/RenderingComponents.h"
#include "Volt/Net/SceneInteraction/NetActorComponent.h"

#include "Volt/Scene/Scene.h"

#include "Volt/Utility/Random.h"

namespace Volt
{
	Prefab::Prefab(Entity srcRootEntity)
	{
		CreatePrefab(srcRootEntity);
	}

	Entity Prefab::Instantiate(Weak<Scene> targetScene)
	{
		if (!IsPrefabValid())
		{
			return Entity::Null();
		}

		auto targetScenePtr = targetScene.lock();

		Entity newEntity = Entity::Duplicate(Entity{ m_rootEntityId, m_prefabScene }, targetScenePtr);
		if (targetScenePtr->IsPlaying())
		{
			InitializeComponents(newEntity);
		}

		targetScenePtr->InvalidateEntityTransform(newEntity.GetID());
		return newEntity;
	}

	const bool Prefab::UpdateEntityInPrefab(Entity srcEntity)
	{
		const bool updateSucceded = UpdateEntityInPrefabInternal(srcEntity, true);

		if (updateSucceded)
		{
			ValidatePrefabUpdate(srcEntity);
		}

		return updateSucceded;
	}

	void Prefab::UpdateEntityInScene(Entity sceneEntity)
	{
		if (!sceneEntity.HasComponent<PrefabComponent>())
		{
			return;
		}

		const auto& scenePrefabComp = sceneEntity.GetComponent<PrefabComponent>();
		if (scenePrefabComp.prefabAsset != handle)
		{
			return;
		}

		Entity prefabEntity{ scenePrefabComp.prefabEntity, m_prefabScene };
		if (!prefabEntity)
		{
			sceneEntity.GetScene()->RemoveEntity(sceneEntity);
			return;
		}

		auto sceneTransform = sceneEntity.GetComponent<TransformComponent>();
		auto sceneRelationships = sceneEntity.GetComponent<RelationshipComponent>();
		auto sceneCommonComponent = sceneEntity.GetComponent<CommonComponent>();

		Entity::Copy(prefabEntity, sceneEntity);

		if (prefabEntity.GetID() == m_rootEntityId)
		{
			sceneEntity.GetComponent<TransformComponent>() = sceneTransform;
		}

		sceneEntity.GetComponent<RelationshipComponent>() = sceneRelationships;
		sceneEntity.GetComponent<CommonComponent>() = sceneCommonComponent;

		sceneEntity.GetScene()->InvalidateEntityTransform(sceneEntity.GetID());

		for (const auto& sceneChild : sceneEntity.GetChildren())
		{
			UpdateEntityInScene(sceneChild);
		}

		for (const auto& prefabChild : prefabEntity.GetChildren())
		{
			bool exists = false;

			for (const auto& sceneChild : sceneEntity.GetChildren())
			{
				if (!sceneChild.HasComponent<PrefabComponent>())
				{
					continue;
				}

				if (prefabChild.GetID() == sceneChild.GetComponent<PrefabComponent>().prefabEntity)
				{
					exists = true;
					break;
				}
			}

			if (!exists)
			{
				auto newEntity = InstantiateEntity(sceneEntity.GetScene(), prefabChild);
				auto preParentTransform = newEntity.GetComponent<TransformComponent>();

				newEntity.SetParent(sceneEntity);

				newEntity.GetComponent<TransformComponent>() = preParentTransform;
			}
		}
	}

	const bool Prefab::IsEntityValidInPrefab(Entity entity) const
	{
		if (!entity.HasComponent<PrefabComponent>())
		{
			return false;
		}

		const auto& prefabComponent = entity.GetComponent<PrefabComponent>();

		if (prefabComponent.prefabAsset != handle)
		{
			return false;
		}

		Entity prefabEntity{ entity.GetComponent<PrefabComponent>().prefabEntity, m_prefabScene };
		const bool isValid = prefabEntity.IsValid();
		return isValid;
	}

	const bool Prefab::IsEntityRoot(Entity entity) const
	{
		if (!entity.HasComponent<PrefabComponent>())
		{
			return false;
		}

		const auto& prefabComponent = entity.GetComponent<PrefabComponent>();

		if (prefabComponent.prefabAsset != handle)
		{
			return false;
		}

		return prefabComponent.prefabEntity == m_rootEntityId;
	}

	void Prefab::InitializeComponents(Entity entity)
	{
		Entity prefabEntity{ entity.GetComponent<PrefabComponent>().prefabEntity, m_prefabScene };

		if (prefabEntity.HasComponent<RigidbodyComponent>())
		{
			if (entity.HasComponent<RigidbodyComponent>())
			{
				entity.RemoveComponent<RigidbodyComponent>();
			}

			const auto& srcComp = prefabEntity.GetComponent<RigidbodyComponent>();
			entity.AddComponent<RigidbodyComponent>(srcComp.bodyType, srcComp.layerId, srcComp.mass, srcComp.linearDrag, srcComp.lockFlags, srcComp.angularDrag, srcComp.disableGravity, srcComp.isKinematic, srcComp.collisionType);
		}

		if (prefabEntity.HasComponent<CharacterControllerComponent>())
		{
			if (entity.HasComponent<CharacterControllerComponent>())
			{
				entity.RemoveComponent<CharacterControllerComponent>();
			}

			const auto& srcComp = prefabEntity.GetComponent<CharacterControllerComponent>();
			entity.AddComponent<CharacterControllerComponent>(srcComp.climbingMode, srcComp.slopeLimit, srcComp.invisibleWallHeight, srcComp.maxJumpHeight, srcComp.contactOffset, srcComp.stepOffset, srcComp.density, srcComp.layer, srcComp.hasGravity);
		}

		if (entity.HasComponent<NetActorComponent>())
		{
			entity.GetComponent<NetActorComponent>().repId = Nexus::RandRepID();
		}

		if (entity.HasComponent<AnimationControllerComponent>())
		{
			auto& controllerComp = entity.GetComponent<AnimationControllerComponent>();
			if (controllerComp.animationGraph != Asset::Null())
			{
				auto graphAsset = AssetManager::GetAsset<AnimationGraphAsset>(controllerComp.animationGraph);
				if (graphAsset && graphAsset->IsValid())
				{
					controllerComp.controller = CreateRef<AnimationController>(graphAsset, entity);
				}
			}
		}

		// Common data
		{
			if (!entity.HasComponent<CommonComponent>())
			{
				entity.AddComponent<CommonComponent>();
			}

			auto& commonComponent = entity.GetComponent<CommonComponent>();
			commonComponent.layerId = entity.GetScene()->GetActiveLayer();
			commonComponent.randomValue = Random::Float(0.f, 1.f);
			commonComponent.timeSinceCreation = 0.f;
		}

		for (const auto& child : entity.GetChildren())
		{
			InitializeComponents(child);
		}
	}

	void Prefab::CreatePrefab(Entity srcRootEntity)
	{
		if (!srcRootEntity.IsValid())
		{
			return;
		}

		m_prefabScene = CreateRef<Scene>();
		m_rootEntityId = srcRootEntity.GetID();

		AddEntityToPrefabRecursive(srcRootEntity, Entity::Null());
		Entity rootPrefabEntity{ m_rootEntityId, m_prefabScene };
		rootPrefabEntity.GetComponent<RelationshipComponent>().parent = entt::null;

		rootPrefabEntity.SetPosition({ 0.f });
		rootPrefabEntity.SetRotation(glm::identity<glm::quat>());
	}

	void Prefab::AddEntityToPrefabRecursive(Entity srcEntity, Entity parentPrefabEntity)
	{
		entt::entity newEntityId = srcEntity.GetID();

		if (srcEntity.HasComponent<PrefabComponent>())
		{
			newEntityId = srcEntity.GetComponent<PrefabComponent>().prefabEntity;
		}
		else
		{
			srcEntity.AddComponent<PrefabComponent>();
		}

		auto newEntity = m_prefabScene->CreateEntity("", newEntityId);

		auto& srcPrefabComp = srcEntity.GetComponent<PrefabComponent>();
		srcPrefabComp.prefabAsset = handle;
		srcPrefabComp.prefabEntity = newEntity.GetID();
		srcPrefabComp.version = m_version;

		Entity::Copy(srcEntity, newEntity, true);

		auto preParentTransform = newEntity.GetComponent<TransformComponent>();

		if (parentPrefabEntity.IsValid())
		{
			newEntity.SetParent(parentPrefabEntity);
		}

		newEntity.GetComponent<TransformComponent>() = preParentTransform;

		for (const auto& child : srcEntity.GetChildren())
		{
			AddEntityToPrefabRecursive(child, newEntity);
		}
	}

	void Prefab::ValidatePrefabUpdate(Entity srcEntity)
	{
		std::vector<entt::entity> entitiesToRemove;
		std::vector<Entity> srcHeirarchy = FlattenEntityHeirarchy(srcEntity);

		m_prefabScene->ForEachWithComponents<const PrefabComponent>([&](const entt::entity id, const PrefabComponent& prefabComp) 
		{
			bool exists = false;

			for (const auto& srcEnt : srcHeirarchy)
			{
				if (id == srcEnt.GetComponent<PrefabComponent>().prefabEntity)
				{
					exists = true;
					break;
				}
			}

			if (!exists)
			{
				entitiesToRemove.emplace_back(id);
			}
		});

		for (const auto& id : entitiesToRemove)
		{
			m_prefabScene->RemoveEntity(Entity{ id, m_prefabScene });
		}
	}

	const bool Prefab::UpdateEntityInPrefabInternal(Entity srcEntity, bool isFirst)
	{
		if (isFirst)
		{
			if (!srcEntity.HasComponent<PrefabComponent>())
			{
				return false;
			}
		}
		else
		{
			if (!srcEntity.HasComponent<PrefabComponent>())
			{
				Entity prefabParent = Entity::Null();

				if (srcEntity.HasParent() && srcEntity.GetParent().HasComponent<PrefabComponent>())
				{
					prefabParent = { srcEntity.GetParent().GetComponent<PrefabComponent>().prefabEntity, m_prefabScene };
				}

				AddEntityToPrefabRecursive(srcEntity, prefabParent);
			}
		}

		auto& srcPrefabComp = srcEntity.GetComponent<PrefabComponent>();
		if (srcPrefabComp.prefabAsset != handle)
		{
			return false;
		}

		Entity targetEntity{ srcPrefabComp.prefabEntity, m_prefabScene };
		if (!targetEntity)
		{
			VT_CORE_WARN("[Prefab]: Trying to update an invalid entity in prefab!");
			return false;
		}

		auto prefabRelationships = targetEntity.GetComponent<RelationshipComponent>();
		auto prefabCommonComponent = targetEntity.GetComponent<CommonComponent>();
		auto prefabComponent = targetEntity.GetComponent<PrefabComponent>();

		Entity::Copy(srcEntity, targetEntity);

		targetEntity.GetComponent<RelationshipComponent>() = prefabRelationships;
		targetEntity.GetComponent<CommonComponent>() = prefabCommonComponent;
		targetEntity.GetComponent<PrefabComponent>() = prefabComponent;

		for (const auto& srcChild : srcEntity.GetChildren())
		{
			UpdateEntityInPrefabInternal(srcChild, false);
		}

		return true;
	}

	Entity Prefab::InstantiateEntity(Weak<Scene> targetScene, Entity prefabEntity)
	{
		auto targetScenePtr = targetScene.lock();

		Entity newEntity = Entity::Duplicate(Entity{ prefabEntity.GetID(), m_prefabScene }, targetScenePtr);
		if (targetScenePtr->IsPlaying())
		{
			InitializeComponents(newEntity);
		}

		targetScenePtr->InvalidateEntityTransform(newEntity.GetID());
		return newEntity;
	}

	const std::vector<Entity> Prefab::FlattenEntityHeirarchy(Entity entity)
	{
		std::vector<Entity> result;

		result.emplace_back(entity);

		for (auto child : entity.GetChildren())
		{
			auto childResult = FlattenEntityHeirarchy(child);
		
			for (auto& childRes : childResult)
			{
				result.emplace_back(childRes);
			}
		}

		return result;
	}
}
