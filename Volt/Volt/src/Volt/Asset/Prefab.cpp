#include "vtpch.h"
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
