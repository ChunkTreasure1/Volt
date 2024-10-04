#include "espch.h"

#include "EntitySystem/EntityHelper.h"
#include "EntitySystem/ComponentRegistry.h"
#include "EntitySystem/Scripting/CoreComponents.h"

#include <CoreUtilities/StringUtility.h>

namespace Volt
{
	EntityHelper::EntityHelper()
	{
	}

	EntityHelper::EntityHelper(entt::entity entityHandle, EntityScene* scene)
		: m_handle(entityHandle), m_sceneReference(scene)
	{
	}

	EntityHelper::EntityHelper(entt::entity entityHandle, const EntityScene* scene)
		: m_handle(entityHandle), m_sceneReference(const_cast<EntityScene*>(scene))
	{
	}
	
	EntityHelper::~EntityHelper()
	{
	}

	void EntityHelper::SetPosition(const glm::vec3& position)
	{
		VT_ENSURE(IsValid());

		EntityHelper parent = GetParent();
		TQS parentTransform;

		if (parent)
		{
			parentTransform = m_sceneReference->GetEntityWorldTQS(parent);
		}

		const glm::vec3 translatedPosition = position - parentTransform.translation;
		const glm::vec3 invertedScale = 1.f / parentTransform.scale;
		const glm::vec3 rotatedPoint = glm::conjugate(parentTransform.rotation) * translatedPosition;

		const glm::vec3 localPoint = rotatedPoint * invertedScale;
		SetLocalPosition(localPoint);
	}

	void EntityHelper::SetRotation(const glm::quat& rotation)
	{
		VT_ENSURE(IsValid());

		EntityHelper parent = GetParent();
		TQS parentTransform;

		if (parent)
		{
			parentTransform = m_sceneReference->GetEntityWorldTQS(parent);
		}

		const glm::quat localRotation = glm::conjugate(parentTransform.rotation) * rotation;
		SetLocalRotation(localRotation);
	}

	void EntityHelper::SetScale(const glm::vec3& scale)
	{
		VT_ENSURE(IsValid());

		EntityHelper parent = GetParent();
		TQS parentTransform{};

		if (parent)
		{
			parentTransform = m_sceneReference->GetEntityWorldTQS(parent);
		}

		const glm::vec3 inverseScale = 1.f / parentTransform.scale;
		const glm::vec3 localScale = scale * inverseScale;

		SetLocalScale(localScale);
	}

	void EntityHelper::SetLocalPosition(const glm::vec3& position)
	{
		VT_ENSURE(IsValid());

		auto& registry = m_sceneReference->GetRegistry();
		VT_ENSURE(registry.any_of<TransformComponent>(m_handle));

		registry.get<TransformComponent>(m_handle).position = position;
		m_sceneReference->InvalidateEntityTransform(GetID());
	}

	void EntityHelper::SetLocalRotation(const glm::quat& rotation)
	{
		VT_ENSURE(IsValid());

		auto& registry = m_sceneReference->GetRegistry();
		VT_ENSURE(registry.any_of<TransformComponent>(m_handle));

		registry.get<TransformComponent>(m_handle).rotation = rotation;
		m_sceneReference->InvalidateEntityTransform(GetID());
	}

	void EntityHelper::SetLocalScale(const glm::vec3& scale)
	{
		VT_ENSURE(IsValid());

		auto& registry = m_sceneReference->GetRegistry();
		VT_ENSURE(registry.any_of<TransformComponent>(m_handle));

		registry.get<TransformComponent>(m_handle).scale = scale;
		m_sceneReference->InvalidateEntityTransform(GetID());
	}

	glm::vec3 EntityHelper::GetForward() const
	{
		VT_ENSURE(IsValid());

		return glm::rotate(GetRotation(), glm::vec3{ 0.f, 0.f, 1.f });
	}

	glm::vec3 EntityHelper::GetRight() const
	{
		VT_ENSURE(IsValid());

		return glm::rotate(GetRotation(), glm::vec3{ 1.f, 0.f, 0.f });
	}

	glm::vec3 EntityHelper::GetUp() const
	{
		VT_ENSURE(IsValid());

		return glm::rotate(GetRotation(), glm::vec3{ 0.f, 1.f, 0.f });
	}

	glm::vec3 EntityHelper::GetLocalForward() const
	{
		VT_ENSURE(IsValid());

		auto& registry = m_sceneReference->GetRegistry();
		VT_ENSURE(registry.any_of<TransformComponent>(m_handle));
		return registry.get<TransformComponent>(m_handle).GetForward();
	}

	glm::vec3 EntityHelper::GetLocalRight() const
	{
		VT_ENSURE(IsValid());

		auto& registry = m_sceneReference->GetRegistry();
		VT_ENSURE(registry.any_of<TransformComponent>(m_handle));
		return registry.get<TransformComponent>(m_handle).GetRight();
	}

	glm::vec3 EntityHelper::GetLocalUp() const
	{
		VT_ENSURE(IsValid());

		auto& registry = m_sceneReference->GetRegistry();
		VT_ENSURE(registry.any_of<TransformComponent>(m_handle));
		return registry.get<TransformComponent>(m_handle).GetUp();
	}

	void EntityHelper::SetTag(const std::string& tag)
	{
		VT_ENSURE(IsValid());

		auto& registry = m_sceneReference->GetRegistry();
		VT_ENSURE(registry.any_of<TagComponent>(m_handle));
		
		registry.get<TagComponent>(m_handle).tag = tag;
	}
	
	glm::vec3 EntityHelper::GetPosition() const
	{
		VT_ENSURE(IsValid());
		const auto tqs = m_sceneReference->GetEntityWorldTQS(*this);
		return tqs.translation;
	}
	
	glm::quat EntityHelper::GetRotation() const
	{
		VT_ENSURE(IsValid());
		const auto tqs = m_sceneReference->GetEntityWorldTQS(*this);
		return tqs.rotation;
	}
	
	glm::vec3 EntityHelper::GetScale() const
	{
		VT_ENSURE(IsValid());
		const auto tqs = m_sceneReference->GetEntityWorldTQS(*this);
		return tqs.scale;
	}
	
	const glm::vec3& EntityHelper::GetLocalPosition() const
	{
		VT_ENSURE(IsValid());

		auto& registry = m_sceneReference->GetRegistry();
		VT_ENSURE(registry.any_of<TransformComponent>(m_handle));
		return registry.get<TransformComponent>(m_handle).position;
	}
	
	const glm::quat& EntityHelper::GetLocalRotation() const
	{
		VT_ENSURE(IsValid());

		auto& registry = m_sceneReference->GetRegistry();
		VT_ENSURE(registry.any_of<TransformComponent>(m_handle));
		return registry.get<TransformComponent>(m_handle).rotation;
	}
	
	const glm::vec3& EntityHelper::GetLocalScale() const
	{
		VT_ENSURE(IsValid());

		auto& registry = m_sceneReference->GetRegistry();

		VT_ENSURE(registry.any_of<TransformComponent>(m_handle));
		return registry.get<TransformComponent>(m_handle).scale;
	}

	const std::string& EntityHelper::GetTag() const
	{
		VT_ENSURE(IsValid());

		auto& registry = m_sceneReference->GetRegistry();
		VT_ENSURE(registry.any_of<TagComponent>(m_handle));
		return registry.get<TagComponent>(m_handle).tag;
	}

	bool EntityHelper::HasParent() const
	{
		VT_ENSURE(IsValid());

		auto& registry = m_sceneReference->GetRegistry();
		VT_ENSURE(registry.any_of<RelationshipComponent>(m_handle));

		auto& relationshipComponent = registry.get<RelationshipComponent>(m_handle);
		auto parentEntity = m_sceneReference->GetEntityHelperFromEntityID(relationshipComponent.parent);

		return parentEntity.IsValid();
	}

	EntityHelper EntityHelper::GetParent() const
	{
		VT_ENSURE(IsValid());

		auto& registry = m_sceneReference->GetRegistry();
		VT_ENSURE(registry.any_of<RelationshipComponent>(m_handle));

		auto& relationshipComponent = registry.get<RelationshipComponent>(m_handle);
		if (relationshipComponent.parent == EntityID::Null())
		{
			return Null();
		}

		return m_sceneReference->GetEntityHelperFromEntityID(relationshipComponent.parent);
	}

	EntityID EntityHelper::GetID() const
	{
		VT_ENSURE(IsValid());

		auto& registry = m_sceneReference->GetRegistry();
		VT_ENSURE(registry.any_of<IDComponent>(m_handle));
		return registry.get<IDComponent>(m_handle).id;
	}

	void EntityHelper::RemoveComponent(const VoltGUID& guid)
	{
		VT_ENSURE(IsValid());

		ComponentRegistry::Helpers::RemoveComponentWithGUID(guid, m_sceneReference->GetRegistry(), m_handle);
	}

	bool EntityHelper::HasComponent(std::string_view componentName) const
	{
		VT_ENSURE(IsValid());

		const std::string lowerCompName = ::Utility::ToLower(std::string(componentName));

		auto& registry = m_sceneReference->GetRegistry();
		for (auto&& curr : registry.storage())
		{
			if (auto& storage = curr.second; storage.contains(m_handle))
			{
				std::string tempStr(storage.type().name());

				if (::Utility::ToLower(tempStr) == lowerCompName)
				{
					return true;
				}
			}
		}

		return false;
	}

	bool EntityHelper::HasComponent(const VoltGUID& componentGUID) const
	{
		return ComponentRegistry::Helpers::HasComponentWithGUID(componentGUID, m_sceneReference->GetRegistry(), m_handle);
	}

	EntityHelper EntityHelper::Null()
	{
		return {};
	}
}
