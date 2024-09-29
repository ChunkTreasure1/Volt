#include "espch.h"

#include "EntitySystem/EntityHelper.h"
#include "EntitySystem/EntityScene.h"
#include "EntitySystem/SceneEvents.h"

#include "EntitySystem/Scripting/CoreComponents.h"
#include "EntitySystem/Scripting/CommonComponent.h"
#include "EntitySystem/Scripting/ECSSystemRegistry.h"
#include "EntitySystem/Scripting/ECSBuilder.h"

#include <EventSystem/EventSystem.h>

#include <CoreUtilities/Time/TimeUtility.h>

#include <ranges>

namespace Volt
{
	EntityScene::EntityScene()
	{
		Initialize();
	}

	EntityScene::~EntityScene()
	{
	}

	void EntityScene::OnRuntimeStart()
	{
		m_isPlaying = true;
		ComponentOnStart();
		
		OnSceneRuntimeStartEvent startEvent(*this);
		EventSystem::DispatchEvent(startEvent);
	}

	void EntityScene::OnRuntimeEnd()
	{
		ComponentOnStop();

		OnSceneRuntimeEndEvent endEvent(*this);
		EventSystem::DispatchEvent(endEvent);

		m_isPlaying = false;
	}

	void EntityScene::Update(float deltaTime)
	{
		VT_PROFILE_FUNCTION();
		m_ecsBuilder->GetGameLoop(GameLoop::Variable).Execute(*this, deltaTime);
	}

	void EntityScene::FixedUpdate(float deltaTime)
	{
		VT_PROFILE_FUNCTION();
		m_ecsBuilder->GetGameLoop(GameLoop::Fixed).Execute(*this, deltaTime);
	}

	void EntityScene::SortScene()
	{
		m_registry.sort<CommonComponent>([&](entt::entity lhs, entt::entity rhs)
		{
			const auto& lhsComp = m_registry.get<CommonComponent>(lhs);
			const auto& rhsComp = m_registry.get<CommonComponent>(rhs);

			return lhsComp.timeCreatedID < rhsComp.timeCreatedID;
		});
	}

	void EntityScene::ClearScene()
	{
		m_registry.clear();
	}

	EntityHelper EntityScene::CreateEntity(const std::string& tag)
	{
		entt::entity entityHandle = m_registry.create();

		EntityHelper newHelper(entityHandle, this);

		// Setup default components
		{
			auto& transformComponent = newHelper.AddComponent<TransformComponent>();
			transformComponent.position = 0.f;
			transformComponent.rotation = glm::identity<glm::quat>();
			transformComponent.scale = 1.f;

			auto& tagComponent = newHelper.AddComponent<TagComponent>();
			if (tag.empty())
			{
				tagComponent.tag = "New Entity";
			}
			else
			{
				tagComponent.tag = tag;
			}

			auto& idComponent = newHelper.AddComponent<IDComponent>();
			while (m_entityRegistry.Contains(idComponent.id))
			{
				idComponent.id = {};
			}

			auto& commonComponent = newHelper.AddComponent<CommonComponent>();
			commonComponent.timeCreatedID = TimeUtility::GetTimeSinceEpoch();

			newHelper.AddComponent<RelationshipComponent>();
		}

		m_entityRegistry.AddEntity(newHelper);
		m_entityRegistry.MarkEntityAsEdited(newHelper);
		
		InvalidateEntityTransform(newHelper.GetID());
		SortScene();
		return newHelper;
	}

	EntityHelper EntityScene::CreateEntityWithID(EntityID id, const std::string& tag)
	{
		VT_ENSURE(!m_entityRegistry.Contains(id));

		entt::entity entityHandle = m_registry.create();

		EntityHelper newHelper(entityHandle, this);

		// Setup default components
		{
			auto& transformComponent = newHelper.AddComponent<TransformComponent>();
			transformComponent.position = 0.f;
			transformComponent.rotation = glm::identity<glm::quat>();
			transformComponent.scale = 1.f;

			auto& tagComponent = newHelper.AddComponent<TagComponent>();
			if (tag.empty())
			{
				tagComponent.tag = "New Entity";
			}
			else
			{
				tagComponent.tag = tag;
			}

			auto& idComponent = newHelper.AddComponent<IDComponent>();
			idComponent.id = id;

			auto& commonComponent = newHelper.AddComponent<CommonComponent>();
			commonComponent.timeCreatedID = TimeUtility::GetTimeSinceEpoch();

			newHelper.AddComponent<RelationshipComponent>();
		}

		m_entityRegistry.AddEntity(newHelper);
		m_entityRegistry.MarkEntityAsEdited(newHelper);

		InvalidateEntityTransform(newHelper.GetID());
		SortScene();
		return newHelper;
	}

	void EntityScene::DestroyEntity(EntityID id, bool isDestroyingChildFromParent)
	{
		if (!IsEntityValid(id))
		{
			return;
		}

		EntityHelper helper = GetEntityHelperFromEntityID(id);

		// We need to handle the entity's parent and children.
		VT_ENSURE(helper.HasComponent<RelationshipComponent>());

		auto& relationshipComponent = helper.GetComponent<RelationshipComponent>();
		if (relationshipComponent.parent != EntityID::Null())
		{
			if (!isDestroyingChildFromParent)
			{
				EntityHelper parentHelper = GetEntityHelperFromEntityID(relationshipComponent.parent);
				VT_ENSURE(parentHelper.HasComponent<RelationshipComponent>());

				auto& parentRelationshipComponent = parentHelper.GetComponent<RelationshipComponent>();
				parentRelationshipComponent.children.erase_with_predicate([&helper](EntityID id) 
				{
					return id == helper.GetID();
				});
			}
		}

		// We need to do this backwards, otherwise we will be pointing to invalid indices
		for (int32_t i = static_cast<int32_t>(relationshipComponent.children.size()) - 1; i >= 0; --i)
		{
			DestroyEntity(relationshipComponent.children.at(i), true);

			// This is required, because removing components from entt::registry might
			// invalidate pointers.
			relationshipComponent = helper.GetComponent<RelationshipComponent>();
		}

		m_entityRegistry.RemoveEntity(helper);
		m_registry.destroy(helper.GetHandle());
	}

	void EntityScene::MarkEntityAsEdited(const EntityHelper& entityHelper)
	{
		m_entityRegistry.MarkEntityAsEdited(entityHelper);
	}

	void EntityScene::ClearEditedEntities()
	{
		m_entityRegistry.ClearEditedEntities();
	}

	Vector<EntityID> EntityScene::InvalidateEntityTransform(EntityID entityId)
	{
		Vector<EntityID> invalidatedEntities;
		invalidatedEntities.reserve(10);

		Vector<EntityID> entityStack;
		entityStack.reserve(10);
		entityStack.push_back(entityId);

		while (!entityStack.empty())
		{
			const EntityID currentId = entityStack.back();
			entityStack.pop_back();

			EntityHelper entityHelper(m_entityRegistry.GetHandleFromID(currentId), this);

			VT_ENSURE(entityHelper.HasComponent<RelationshipComponent>());
		
			auto& relationshipComponent = entityHelper.GetComponent<RelationshipComponent>();

			m_transformCache.InvalidateTransform(currentId);

			for (const auto& child : relationshipComponent.children)
			{
				entityStack.push_back(child);
			}

			invalidatedEntities.emplace_back(currentId);
		}

		return invalidatedEntities;
	}

	bool EntityScene::IsEntityValid(EntityID entityId) const
	{
		return m_registry.valid(m_entityRegistry.GetHandleFromID(entityId));
	}

	TQS EntityScene::GetEntityWorldTQS(const EntityHelper& entityHelper) const
	{
		if (m_transformCache.HasCachedTransform(entityHelper.GetID()))
		{
			return m_transformCache.GetCachedTransform(entityHelper.GetID());
		}

		Vector<EntityHelper> hierarchy{};
		hierarchy.emplace_back(entityHelper);

		EntityHelper currentEntity = entityHelper;
		while (currentEntity.HasParent())
		{
			auto parent = currentEntity.GetParent();
			hierarchy.emplace_back(parent);
			currentEntity = parent;
		}

		TQS resultTransform{};
		for (const auto& ent : std::ranges::reverse_view(hierarchy))
		{
			const auto& transformComp = m_registry.get<TransformComponent>(ent.GetHandle());

			resultTransform.translation = resultTransform.translation + resultTransform.rotation * transformComp.position;
			resultTransform.rotation = resultTransform.rotation * transformComp.rotation;
			resultTransform.scale = resultTransform.scale * transformComp.scale;
		}

		m_transformCache.CacheTransform(entityHelper.GetID(), resultTransform);
		return resultTransform;
	}

	EntityHelper EntityScene::GetEntityHelperFromEntityID(EntityID entityId) const
	{
		if (!m_entityRegistry.Contains(entityId))
		{
			return EntityHelper::Null();
		}

		return { m_entityRegistry.GetHandleFromID(entityId), this };
	}

	EntityHelper EntityScene::GetEntityHelperFromEntityHandle(entt::entity entityHandle) const
	{
		if (!m_entityRegistry.Contains(entityHandle))
		{
			return EntityHelper::Null();
		}

		return { entityHandle, this };
	}

	uint32_t EntityScene::GetEntityAliveCount() const
	{
		return static_cast<uint32_t>(m_registry.alive());
	}

	void EntityScene::Initialize()
	{
		m_ecsBuilder = CreateScope<ECSBuilder>();
		m_registry.set_user_data(this);

		ComponentRegistry::Helpers::SetupComponentCallbacks(m_registry);
		GetECSSystemRegistry().Build(*m_ecsBuilder);
		m_ecsBuilder->Compile();
	}

	void EntityScene::ComponentOnStart()
	{
		VT_PROFILE_FUNCTION();

		for (auto&& curr : m_registry.storage())
		{
			auto& storage = curr.second;
			std::string_view typeName = storage.type().name();

			const ICommonTypeDesc* typeDesc = GetComponentRegistry().GetTypeDescFromName(typeName);
			if (!typeDesc)
			{
				continue;
			}

			if (typeDesc->GetValueType() != ValueType::Component)
			{
				continue;
			}

			for (auto& entity : storage)
			{
				const IComponentTypeDesc* compTypeDesc = reinterpret_cast<const IComponentTypeDesc*>(typeDesc);
				auto entityHelper = GetEntityHelperFromEntityHandle(entity);
				compTypeDesc->OnStart(entityHelper);
			}
		}
	}
	
	void EntityScene::ComponentOnStop()
	{
		VT_PROFILE_FUNCTION();

		for (auto&& curr : m_registry.storage())
		{
			auto& storage = curr.second;
			std::string_view typeName = storage.type().name();

			const ICommonTypeDesc* typeDesc = GetComponentRegistry().GetTypeDescFromName(typeName);
			if (!typeDesc)
			{
				continue;
			}

			if (typeDesc->GetValueType() != ValueType::Component)
			{
				continue;
			}

			for (auto& entity : storage)
			{
				const IComponentTypeDesc* compTypeDesc = reinterpret_cast<const IComponentTypeDesc*>(typeDesc);
				auto entityHelper = GetEntityHelperFromEntityHandle(entity);
				compTypeDesc->OnStop(entityHelper);
			}
		}
	}
}
