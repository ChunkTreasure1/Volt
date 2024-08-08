#include "vtpch.h"
#include "Prefab.h"

#include "Volt/Asset/Asset.h"
#include "Volt/Asset/AssetManager.h"

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

		auto rootEntity = m_prefabScene->GetEntityFromUUID(m_rootEntityId);
		if (!rootEntity)
		{
			VT_LOG(Error, "[Prefab] Root Entity is not a valid entity!");
			return Entity::Null();
		}

		Entity newEntity = Entity::Duplicate(rootEntity, targetScene, Entity::Null(), EntityCopyFlags::SkipCommonData);
		if (targetScene->IsPlaying())
		{
			InitializeComponents(newEntity);
		}

		// Set scene root entity & update prefab references
		{
			Vector<Entity> flatInstantiatedHeirarchy = FlattenEntityHeirarchy(newEntity);
			for (auto& entity : flatInstantiatedHeirarchy)
			{
				if (m_prefabReferencesMap.contains(entity.GetID()))
				{
					const auto& prefabRefData = m_prefabReferencesMap.at(entity.GetID());

					Ref<Prefab> prefabRefAsset = AssetManager::GetAsset<Prefab>(prefabRefData.prefabAsset);
					prefabRefAsset->UpdateEntityInSceneInternal(entity, prefabRefData.prefabReferenceEntity);
				}
				else
				{
					entity.GetComponent<PrefabComponent>().sceneRootEntity = newEntity.GetID();
				}
			}
		}

		targetScene->InvalidateEntityTransform(newEntity.GetID());
		return newEntity;
	}

	const bool Prefab::UpdateEntityInPrefab(Entity srcEntity)
	{
		if (!srcEntity.HasComponent<PrefabComponent>())
		{
			return false;
		}

		const bool updateSucceded = UpdateEntityInPrefabInternal(srcEntity, srcEntity.GetComponent<PrefabComponent>().sceneRootEntity, Entity::NullID());

		if (updateSucceded)
		{
			ValidatePrefabUpdate(srcEntity);
		}

		// Increase prefab version, because we made a change
		m_version++;

		// Update all entities prefab versions
		{
			Vector<Entity> flatHeirarchy = FlattenEntityHeirarchy(srcEntity);
			for (auto& entity : flatHeirarchy)
			{
				entity.GetComponent<PrefabComponent>().version = m_version;
			}
		}

		UpdatePrefabVersion(m_prefabScene->GetEntityFromUUID(m_rootEntityId), m_version);

		return updateSucceded;
	}

	void Prefab::UpdateEntityInScene(Entity sceneEntity)
	{
		UpdateEntityInSceneInternal(sceneEntity, Entity::NullID());
	}

	void Prefab::CopyPrefabEntity(Entity dstEntity, EntityID srcPrefabEntityId, const EntityCopyFlags copyFlags) const
	{
		Entity prefabEntity = m_prefabScene->GetEntityFromUUID(srcPrefabEntityId);
		if (!prefabEntity)
		{
			return;
		}

		Entity::Copy(prefabEntity, dstEntity, copyFlags);
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

		const EntityID prefabEntityId = entity.GetComponent<PrefabComponent>().prefabEntity;

		Entity prefabEntity = m_prefabScene->GetEntityFromUUID(prefabEntityId);
		const bool isValid = prefabEntity.IsValid();
		return isValid;
	}

	const bool Prefab::IsEntityValidInPrefab(EntityID prefabEntityId) const
	{
		return m_prefabScene->IsEntityValid(prefabEntityId);
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

	const bool Prefab::IsReference(Entity entity) const
	{
		if (!entity.HasComponent<PrefabComponent>())
		{
			return false;
		}

		return m_prefabReferencesMap.contains(entity.GetComponent<PrefabComponent>().prefabEntity);
	}

	const Prefab::PrefabReferenceData& Prefab::GetReferenceData(Entity entity) const
	{
		return m_prefabReferencesMap.at(entity.GetComponent<PrefabComponent>().prefabEntity);
	}

	const Entity Prefab::GetRootEntity() const
	{
		return m_prefabScene->GetEntityFromUUID(m_rootEntityId);
	}

	void Prefab::InitializeComponents(Entity entity)
	{
		Entity prefabEntity = m_prefabScene->GetEntityFromUUID(entity.GetComponent<PrefabComponent>().prefabEntity);

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
		Entity rootPrefabEntity = m_prefabScene->GetEntityFromUUID(m_rootEntityId);
		rootPrefabEntity.GetComponent<RelationshipComponent>().parent = Entity::NullID();

		rootPrefabEntity.SetPosition({ 0.f });
		rootPrefabEntity.SetRotation(glm::identity<glm::quat>());
	}

	void Prefab::AddEntityToPrefabRecursive(Entity srcEntity, Entity parentPrefabEntity)
	{
		EntityID newEntityId = srcEntity.GetID();
		auto newEntity = m_prefabScene->CreateEntityWithUUID(newEntityId);

		// If this entity already has a prefab component, it probably is another prefab. Add it as a reference
		if (srcEntity.HasComponent<PrefabComponent>())
		{
			const auto& prefabComp = srcEntity.GetComponent<PrefabComponent>();
			if (prefabComp.prefabAsset != handle)
			{
				auto& prefabRefData = m_prefabReferencesMap[newEntity.GetID()];
				prefabRefData.prefabReferenceEntity = prefabComp.prefabEntity;
				prefabRefData.prefabAsset = prefabComp.prefabAsset;
			}
		}
		else
		{
			srcEntity.AddComponent<PrefabComponent>();
		}

		auto& srcPrefabComp = srcEntity.GetComponent<PrefabComponent>();
		srcPrefabComp.prefabAsset = handle;
		srcPrefabComp.prefabEntity = newEntity.GetID();
		srcPrefabComp.version = m_version;

		Entity::Copy(srcEntity, newEntity);

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
		Vector<EntityID> entitiesToRemove;
		Vector<Entity> srcHeirarchy = FlattenEntityHeirarchy(srcEntity);

		Entity srcPrefabEntity = m_prefabScene->GetEntityFromUUID(srcEntity.GetComponent<PrefabComponent>().prefabEntity);

		m_prefabScene->ForEachWithComponents<const PrefabComponent, const IDComponent>([&](const entt::entity id, const PrefabComponent& prefabComp, const IDComponent& idComponent)
		{
			bool exists = false;

			if (!m_prefabScene->IsRelatedTo(srcPrefabEntity, Entity{ id, m_prefabScene }))
			{
				return;
			}

			for (const auto& srcEnt : srcHeirarchy)
			{
				if (idComponent.id == srcEnt.GetComponent<PrefabComponent>().prefabEntity)
				{
					exists = true;
					break;
				}
			}

			if (!exists)
			{
				entitiesToRemove.emplace_back(idComponent.id);
			}
		});

		for (const auto& id : entitiesToRemove)
		{
			// Remove references
			if (m_prefabReferencesMap.contains(id))
			{
				m_prefabReferencesMap.erase(id);
			}

			auto entity = m_prefabScene->GetEntityFromUUID(id);
			if (!entity)
			{
				continue;
			}

			m_prefabScene->RemoveEntity(entity);
		}
	}

	void Prefab::UpdatePrefabVersion(Entity entity, uint32_t targetVersion)
	{
		// We are not going to try and update a entity tree which root is not actually a prefab
		if (!entity.HasComponent<PrefabComponent>())
		{
			return;
		}

		entity.GetComponent<PrefabComponent>().version = targetVersion;
	
		for (const auto& child : entity.GetChildren())
		{
			UpdatePrefabVersion(child, targetVersion);
		}
	}

	const bool Prefab::UpdateEntityInPrefabInternal(Entity srcEntity, EntityID sceneRootId, EntityID forcedPrefabEntity)
	{
		bool shouldAddEntity = !srcEntity.HasComponent<PrefabComponent>();
		if (!shouldAddEntity)
		{
			const auto& srcPrefabComp = srcEntity.GetComponent<PrefabComponent>();

			if (m_prefabReferencesMap.contains(srcPrefabComp.prefabEntity))
			{
				const auto& prefabRefData = m_prefabReferencesMap.at(srcPrefabComp.prefabEntity);

				Ref<Prefab> prefabRefAsset = AssetManager::GetAsset<Prefab>(prefabRefData.prefabAsset);
				if (!prefabRefAsset || !prefabRefAsset->IsValid())
				{
					return false;
				}

				return prefabRefAsset->UpdateEntityInPrefabInternal(srcEntity, sceneRootId, prefabRefData.prefabReferenceEntity);
			}

			// Make sure we are not updating from another prefab
			if (srcEntity.GetComponent<PrefabComponent>().prefabAsset != handle && forcedPrefabEntity == Entity::NullID())
			{
				shouldAddEntity = true;
			}
		}

		if (shouldAddEntity)
		{
			Entity prefabParent = Entity::Null();

			if (srcEntity.HasParent() && srcEntity.GetParent().HasComponent<PrefabComponent>())
			{
				prefabParent = m_prefabScene->GetEntityFromUUID(srcEntity.GetParent().GetComponent<PrefabComponent>().prefabEntity);
			}

			AddEntityToPrefabRecursive(srcEntity, prefabParent);
		}

		auto& srcPrefabComp = srcEntity.GetComponent<PrefabComponent>();

		if (srcPrefabComp.sceneRootEntity != sceneRootId)
		{
			return false;
		}

		const EntityID prefabEntityId = forcedPrefabEntity != Entity::NullID() ? forcedPrefabEntity : srcPrefabComp.prefabEntity;
		Entity targetEntity = m_prefabScene->GetEntityFromUUID(prefabEntityId);
		if (!targetEntity)
		{
			VT_LOG(Warning, "[Prefab]: Trying to update an invalid entity in prefab!");
			return false;
		}

		auto prefabRelationships = targetEntity.GetComponent<RelationshipComponent>();
		auto prefabCommonComponent = targetEntity.GetComponent<CommonComponent>();
		auto prefabComponent = targetEntity.GetComponent<PrefabComponent>();

		Entity::Copy(srcEntity, targetEntity, EntityCopyFlags::SkipID | EntityCopyFlags::SkipRelationships | EntityCopyFlags::SkipCommonData);

		targetEntity.GetComponent<RelationshipComponent>() = prefabRelationships;
		targetEntity.GetComponent<CommonComponent>() = prefabCommonComponent;
		targetEntity.GetComponent<PrefabComponent>() = prefabComponent;

		for (const auto& srcChild : srcEntity.GetChildren())
		{
			UpdateEntityInPrefabInternal(srcChild, sceneRootId, Entity::NullID());
		}

		return true;
	}

	void Prefab::UpdateEntityInSceneInternal(Entity sceneEntity, EntityID forcedPrefabEntity)
	{
		if (!sceneEntity.HasComponent<PrefabComponent>())
		{
			return;
		}

		const auto& scenePrefabComp = sceneEntity.GetComponent<PrefabComponent>();
		const EntityID prefabEntityId = forcedPrefabEntity != Entity::NullID() ? forcedPrefabEntity : scenePrefabComp.prefabEntity;

		if (m_prefabReferencesMap.contains(prefabEntityId))
		{
			Ref<Prefab> prefabRefAsset = AssetManager::GetAsset<Prefab>(scenePrefabComp.prefabAsset);
			prefabRefAsset->UpdateEntityInSceneInternal(sceneEntity, m_prefabReferencesMap.at(prefabEntityId).prefabReferenceEntity);

			return;
		}

		Entity prefabEntity = m_prefabScene->GetEntityFromUUID(prefabEntityId);
		if (!prefabEntity)
		{
			sceneEntity.GetScene()->RemoveEntity(sceneEntity);
			return;
		}

		auto sceneTransform = sceneEntity.GetComponent<TransformComponent>();
		auto sceneRelationships = sceneEntity.GetComponent<RelationshipComponent>();
		auto sceneCommonComponent = sceneEntity.GetComponent<CommonComponent>();
		const auto sceneRootId = sceneEntity.GetComponent<PrefabComponent>().sceneRootEntity;

		Entity::Copy(prefabEntity, sceneEntity, EntityCopyFlags::SkipID | EntityCopyFlags::SkipCommonData | EntityCopyFlags::SkipRelationships);

		if (prefabEntity.GetID() == m_rootEntityId)
		{
			sceneEntity.GetComponent<TransformComponent>() = sceneTransform;
		}

		sceneEntity.GetComponent<PrefabComponent>().sceneRootEntity = sceneRootId;
		sceneEntity.GetComponent<PrefabComponent>().version = m_version;
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
				newEntity.GetComponent<PrefabComponent>().sceneRootEntity = sceneEntity.GetComponent<PrefabComponent>().sceneRootEntity;
				newEntity.GetComponent<TransformComponent>() = preParentTransform;
			}
		}
	}

	Entity Prefab::InstantiateEntity(Weak<Scene> targetScene, Entity prefabEntity)
	{
		Entity newEntity = Entity::Duplicate(prefabEntity, targetScene);
		if (targetScene->IsPlaying())
		{
			InitializeComponents(newEntity);
		}

		UpdatePrefabVersion(newEntity, m_version);

		targetScene->InvalidateEntityTransform(newEntity.GetID());
		return newEntity;
	}

	const Vector<Entity> Prefab::FlattenEntityHeirarchy(Entity entity)
	{
		Vector<Entity> result;

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

	Prefab::Prefab(Ref<Scene> prefabScene, EntityID rootEntityId, uint32_t version)
		: m_prefabScene(prefabScene), m_rootEntityId(rootEntityId), m_version(version)
	{
	}
}
