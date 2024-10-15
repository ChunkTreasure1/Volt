#include "vtpch.h"
#include "Volt/Scene/Entity.h"

#include "Volt/Components/CoreComponents.h"

#include "Volt/Physics/Physics.h"
#include "Volt/Physics/PhysicsScene.h"

#include <CoreUtilities/StringUtility.h>

#include <cassert>

namespace Volt
{
	Entity::Entity()
		: m_handle(entt::null)
	{
	}

	Entity::Entity(const Entity& entity)
	{
		*this = entity;
	}

	Entity::~Entity()
	{
	}

	const std::string& Entity::GetTag() const
	{
		auto& registry = m_scene->GetRegistry();

		assert(registry.any_of<TagComponent>(m_handle) && "Entity must have tag component!");
		return registry.get<TagComponent>(m_handle).tag;
	}

	const std::string Entity::ToString() const
	{
		return std::to_string(static_cast<uint32_t>(GetComponent<IDComponent>().id));
	}

	void Entity::SetTag(std::string_view tag)
	{
		auto& registry = m_scene->GetRegistry();

		assert(registry.any_of<TagComponent>(m_handle) && "Entity must have tag component!");
		registry.get<TagComponent>(m_handle).tag = tag;
	}

	glm::mat4 Entity::GetTransform() const
	{
		return m_scene->GetWorldTransform(*this);
	}

	glm::mat4 Entity::GetLocalTransform() const
	{
		auto& registry = m_scene->GetRegistry();

		assert(registry.any_of<TransformComponent>(m_handle) && "Entity must have transform component!");
		return registry.get<TransformComponent>(m_handle).GetTransform();
	}

	glm::vec3 Entity::GetForward() const
	{
		return glm::rotate(GetRotation(), glm::vec3{ 0.f, 0.f, 1.f });
	}

	glm::vec3 Entity::GetRight() const
	{
		return glm::rotate(GetRotation(), glm::vec3{ 1.f, 0.f, 0.f });
	}

	glm::vec3 Entity::GetUp() const
	{
		return glm::rotate(GetRotation(), glm::vec3{ 0.f, 1.f, 0.f });
	}

	glm::vec3 Entity::GetLocalForward() const
	{
		auto& registry = m_scene->GetRegistry();

		assert(registry.any_of<TransformComponent>(m_handle) && "Entity must have transform component!");
		return registry.get<TransformComponent>(m_handle).GetForward();
	}

	glm::vec3 Entity::GetLocalRight() const
	{
		auto& registry = m_scene->GetRegistry();

		assert(registry.any_of<TransformComponent>(m_handle) && "Entity must have transform component!");
		return registry.get<TransformComponent>(m_handle).GetRight();
	}

	glm::vec3 Entity::GetLocalUp() const
	{
		auto& registry = m_scene->GetRegistry();

		assert(registry.any_of<TransformComponent>(m_handle) && "Entity must have transform component!");
		return registry.get<TransformComponent>(m_handle).GetUp();
	}

	glm::vec3 Entity::GetPosition() const
	{
		const auto tqs = m_scene->GetEntityWorldTQS(*this);
		return tqs.translation;
	}

	glm::quat Entity::GetRotation() const
	{
		const auto tqs = m_scene->GetEntityWorldTQS(*this);
		return tqs.rotation;
	}

	glm::vec3 Entity::GetScale() const
	{
		const auto tqs = m_scene->GetEntityWorldTQS(*this);
		return tqs.scale;
	}

	const glm::vec3& Entity::GetLocalPosition() const
	{
		auto& registry = m_scene->GetRegistry();

		assert(registry.any_of<TransformComponent>(m_handle) && "Entity must have transform component!");
		return registry.get<TransformComponent>(m_handle).position;
	}

	const glm::quat& Entity::GetLocalRotation() const
	{
		auto& registry = m_scene->GetRegistry();

		assert(registry.any_of<TransformComponent>(m_handle) && "Entity must have transform component!");
		return registry.get<TransformComponent>(m_handle).rotation;
	}

	const glm::vec3& Entity::GetLocalScale() const
	{
		auto& registry = m_scene->GetRegistry();

		assert(registry.any_of<TransformComponent>(m_handle) && "Entity must have transform component!");
		return registry.get<TransformComponent>(m_handle).scale;
	}

	void Entity::SetPosition(const glm::vec3& position, bool updatePhysics)
	{
		Entity parent = GetParent();
		TQS parentTransform{};

		if (parent)
		{
			parentTransform = m_scene->GetEntityWorldTQS(parent);
		}

		const glm::vec3 translatedPoint = position - parentTransform.translation;
		const glm::vec3 invertedScale = 1.f / parentTransform.scale;
		const glm::vec3 rotatedPoint = glm::conjugate(parentTransform.rotation) * translatedPoint;

		const glm::vec3 localPoint = rotatedPoint * invertedScale;
		SetLocalPosition(localPoint, updatePhysics);
	}

	void Entity::SetRotation(const glm::quat& rotation, bool updatePhysics)
	{
		Entity parent = GetParent();
		TQS parentTransform{};

		if (parent)
		{
			parentTransform = m_scene->GetEntityWorldTQS(parent);
		}

		const glm::quat localRotation = glm::conjugate(parentTransform.rotation) * rotation;
		SetLocalRotation(localRotation, updatePhysics);
	}

	void Entity::SetScale(const glm::vec3& scale)
	{
		Entity parent = GetParent();
		TQS parentTransform{};

		if (parent)
		{
			parentTransform = m_scene->GetEntityWorldTQS(parent);
		}

		const glm::vec3 inverseScale = 1.f / parentTransform.scale;
		const glm::vec3 localScale = scale * inverseScale;

		SetLocalScale(localScale);
	}

	void Entity::SetLocalPosition(const glm::vec3& position, bool updatePhysics)
	{
		auto& registry = m_scene->GetRegistry();

		assert(registry.any_of<TransformComponent>(m_handle) && "Entity must have transform component!");
		registry.get<TransformComponent>(m_handle).position = position;

		UpdatePhysicsTranslation(updatePhysics);
		m_scene->InvalidateEntityTransform(GetID());
	}

	void Entity::SetLocalRotation(const glm::quat& rotation, bool updatePhysics)
	{
		auto& registry = m_scene->GetRegistry();

		assert(registry.any_of<TransformComponent>(m_handle) && "Entity must have transform component!");
		registry.get<TransformComponent>(m_handle).rotation = rotation;

		UpdatePhysicsRotation(updatePhysics);
		m_scene->InvalidateEntityTransform(GetID());
	}

	void Entity::SetLocalScale(const glm::vec3& scale)
	{
		auto& registry = m_scene->GetRegistry();

		assert(registry.any_of<TransformComponent>(m_handle) && "Entity must have transform component!");
		registry.get<TransformComponent>(m_handle).scale = scale;

		m_scene->InvalidateEntityTransform(GetID());
	}

	void Entity::SetParent(Entity parentEntity)
	{
		m_scene->ParentEntity(parentEntity, *this);
	}

	void Entity::AddChild(Entity childEntity)
	{
		m_scene->ParentEntity(*this, childEntity);
	}

	void Entity::ClearParent()
	{
		auto& registry = m_scene->GetRegistry();

		assert(registry.any_of<RelationshipComponent>(m_handle) && "Entity must have relationship component!");

		auto parent = GetParent();
		parent.RemoveChild(*this);

		registry.get<RelationshipComponent>(m_handle).parent = Entity::NullID();
	}

	void Entity::ClearChildren()
	{
		auto& registry = m_scene->GetRegistry();

		assert(registry.any_of<RelationshipComponent>(m_handle) && "Entity must have relationship component!");

		auto& children = registry.get<RelationshipComponent>(m_handle).children;

		for (auto& childId : children)
		{
			auto child = m_scene->GetEntityFromID(childId);

			if (child.GetParent().GetID() == GetID())
			{
				child.GetComponent<RelationshipComponent>().parent = NullID();
			}
		}

		children.clear();
	}

	void Entity::RemoveChild(Entity entity)
	{
		auto& registry = m_scene->GetRegistry();

		assert(registry.any_of<RelationshipComponent>(m_handle) && "Entity must have relationship component!");

		auto& relComp = registry.get<RelationshipComponent>(m_handle);
		for (uint32_t index = 0; auto & childId : relComp.children)
		{
			auto childEnt = m_scene->GetEntityFromID(childId);

			if (childEnt.GetID() == entity.GetID())
			{
				childEnt.GetComponent<RelationshipComponent>().parent = Entity::NullID();

				relComp.children.erase(relComp.children.begin() + index);
				break;
			}

			index++;
		}
	}

	Ref<PhysicsActor> Entity::GetPhysicsActor() const
	{
		if (!Physics::GetScene())
		{
			return nullptr;
		}

		return Physics::GetScene()->GetActor(*this);
	}

	Entity Entity::GetParent() const
	{
		auto& registry = m_scene->GetRegistry();

		assert(registry.any_of<RelationshipComponent>(m_handle) && "Entity must have relationship component!");

		auto& relComp = registry.get<RelationshipComponent>(m_handle);
		if (relComp.parent == Entity::NullID())
		{
			return Null();
		}

		return m_scene->GetEntityFromID(relComp.parent);
	}

	Vector<Entity> Entity::GetChildren() const
	{
		auto& registry = m_scene->GetRegistry();

		if (!registry.any_of<RelationshipComponent>(m_handle))
		{
			return {};
		}

		assert(registry.any_of<RelationshipComponent>(m_handle) && "Entity must have relationship component!");

		const auto& children = registry.get<RelationshipComponent>(m_handle).children;

		Vector<Entity> result{};
		for (const auto& id : children)
		{
			auto entity = m_scene->GetEntityFromID(id);
			if (entity != Entity::Null())
			{
				result.emplace_back(entity);
			}
		}

		return result;
	}

	bool Entity::HasParent() const
	{
		auto& registry = m_scene->GetRegistry();

		if (!registry.any_of<RelationshipComponent>(m_handle))
		{
			return false;
		}

		assert(registry.any_of<RelationshipComponent>(m_handle) && "Entity must have relationship component!");

		auto& relComp = registry.get<RelationshipComponent>(m_handle);
		auto parentEntity = m_scene->GetEntityFromID(relComp.parent);

		return parentEntity.IsValid();
	}

	void Entity::RemoveComponent(const VoltGUID& guid)
	{
		ComponentRegistry::Helpers::RemoveComponentWithGUID(guid, m_scene->GetRegistry(), m_handle);
	}

	bool Entity::HasComponent(std::string_view componentName) const
	{
		const std::string lowerCompName = ::Utility::ToLower(std::string(componentName));

		auto& registry = m_scene->GetRegistry();

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

	void Entity::SetVisible(bool state)
	{
		if (!HasComponent<TransformComponent>())
		{
			return;
		}

		const bool lastVisibleState = GetComponent<TransformComponent>().visible;
		GetComponent<TransformComponent>().visible = state;

		for (auto child : GetChildren())
		{
			child.SetVisible(state);
		}

		if (lastVisibleState == state)
		{
			return;
		}
	}

	void Entity::SetLocked(bool state)
	{
		if (!HasComponent<TransformComponent>())
		{
			return;
		}

		GetComponent<TransformComponent>().locked = state;

		for (auto child : GetChildren())
		{
			child.SetLocked(state);
		}
	}

	bool Entity::IsVisible() const
	{
		auto& registry = m_scene->GetRegistry();

		assert(registry.any_of<TransformComponent>(m_handle) && "Entity must have transform component!");
		return registry.get<TransformComponent>(m_handle).visible;
	}

	bool Entity::IsLocked() const
	{
		auto& registry = m_scene->GetRegistry();

		assert(registry.any_of<TransformComponent>(m_handle) && "Entity must have transform component!");
		return registry.get<TransformComponent>(m_handle).locked;
	}

	bool Entity::IsValid() const
	{
		if (!m_scene || m_handle == entt::null)
		{
			return false;
		}

		return m_scene->GetRegistry().valid(m_handle);
	}

	EntityID Entity::GetID() const
	{
		VT_ASSERT_MSG(HasComponent<IDComponent>(), "Entity must have IDComponent!");
		return GetComponent<IDComponent>().id;
	}

	Entity& Entity::operator=(const Entity& entity)
	{
		m_scene = entity.m_scene;
		m_handle = entity.m_handle;

		return *this;
	}

	Entity Entity::Null()
	{
		return {};
	}

	void Entity::Copy(Entity srcEntity, Entity dstEntity, const EntityCopyFlags copyFlags)
	{
		auto srcScene = srcEntity.GetScene();
		auto& srcRegistry = srcScene->GetRegistry();

		auto dstScene = dstEntity.GetScene();
		auto& dstRegistry = dstScene->GetRegistry();

		for (auto&& curr : srcRegistry.storage())
		{
			auto& storage = curr.second;

			if (!storage.contains(srcEntity))
			{
				continue;
			}

			const IComponentTypeDesc* componentDesc = reinterpret_cast<const IComponentTypeDesc*>(GetComponentRegistry().GetTypeDescFromName(storage.type().name()));
			if (!componentDesc)
			{
				continue;
			}

			if (componentDesc->GetValueType() != ValueType::Component)
			{
				continue;
			}

			if (!ComponentRegistry::Helpers::HasComponentWithGUID(componentDesc->GetGUID(), dstRegistry, dstEntity))
			{
				ComponentRegistry::Helpers::AddComponentWithGUID(componentDesc->GetGUID(), dstRegistry, dstEntity);
			}

			void* voidCompPtr = Volt::ComponentRegistry::Helpers::GetComponentWithGUID(componentDesc->GetGUID(), dstRegistry, dstEntity);
			uint8_t* componentData = reinterpret_cast<uint8_t*>(voidCompPtr);

			if (componentDesc->GetGUID() == GetTypeGUID<PrefabComponent>() && (copyFlags & EntityCopyFlags::SkipPrefab) != EntityCopyFlags::None)
			{
				continue;
			}
			else if ((componentDesc->GetGUID() == GetTypeGUID<RelationshipComponent>() && (copyFlags & EntityCopyFlags::SkipRelationships) != EntityCopyFlags::None))
			{
				continue;
			}
			else if ((componentDesc->GetGUID() == GetTypeGUID<TransformComponent>() && (copyFlags & EntityCopyFlags::SkipTransform) != EntityCopyFlags::None))
			{
				continue;
			}
			else if (componentDesc->GetGUID() == GetTypeGUID<CommonComponent>() && (copyFlags & EntityCopyFlags::SkipCommonData) != EntityCopyFlags::None)
			{
				continue;
			}
			else if (componentDesc->GetGUID() == GetTypeGUID<IDComponent>() && (copyFlags & EntityCopyFlags::SkipID) != EntityCopyFlags::None)
			{
				continue;
			}

			CopyComponent(reinterpret_cast<const uint8_t*>(storage.get(srcEntity)), componentData, 0, componentDesc, dstEntity);
		}
	}

	Entity Entity::Duplicate(Entity srcEntity, Ref<Scene> targetScene, Entity parent, const EntityCopyFlags copyFlags)
	{
		auto scene = targetScene ? targetScene : srcEntity.GetScene().GetSharedPtr();

		Entity newEntity = scene->CreateEntity();

		Copy(srcEntity, newEntity, EntityCopyFlags::SkipID | EntityCopyFlags::SkipRelationships | copyFlags);

		Vector<EntityID> newChildren;

		for (const auto& child : srcEntity.GetChildren())
		{
			newChildren.emplace_back(Duplicate(child, targetScene, newEntity).GetID());
		}

		newEntity.GetComponent<RelationshipComponent>().children = newChildren;
		newEntity.GetComponent<RelationshipComponent>().parent = parent ? parent.GetID() : Entity::NullID();

		return newEntity;
	}

	void Entity::CopyComponent(const uint8_t* srcData, uint8_t* dstData, const size_t offset, const IComponentTypeDesc* compDesc, Entity dstEntity)
	{
		for (const auto& member : compDesc->GetMembers())
		{
			if ((member.flags & ComponentMemberFlag::NoCopy) != ComponentMemberFlag::None)
			{
				continue;
			}

			if (member.typeDesc != nullptr)
			{
				switch (member.typeDesc->GetValueType())
				{
					case ValueType::Component:
					{
						const IComponentTypeDesc* memberCompDesc = reinterpret_cast<const IComponentTypeDesc*>(member.typeDesc);
						CopyComponent(srcData, dstData, offset + member.offset, memberCompDesc, dstEntity);
						break;
					}

					case ValueType::Enum:
						*reinterpret_cast<int32_t*>(&dstData[offset + member.offset]) = *(reinterpret_cast<const int32_t*>(&srcData[offset + member.offset]));
						break;

					case ValueType::Array:
						member.copyFunction(&dstData[offset + member.offset], &srcData[offset + member.offset]);
						break;
				}
			}
			else
			{
				member.copyFunction(&dstData[offset + member.offset], &srcData[offset + member.offset]);
			}
		}

		compDesc->OnComponentCopied(dstEntity.GetScene()->GetEntityHelperFromEntityID(dstEntity.GetID()));
	}

	void Entity::UpdatePhysicsTranslation(bool updateThis)
	{
		if (updateThis && m_scene->IsPlaying() && (HasComponent<RigidbodyComponent>() || HasComponent<CharacterControllerComponent>()))
		{
			auto actor = Physics::GetScene()->GetActor(*this);
			if (actor)
			{
				const glm::vec3 tempPosition = GetPosition();
				actor->SetPosition(tempPosition, true, false);
			}
			else
			{
				auto cc = Physics::GetScene()->GetControllerActor(*this);
				cc->SetFootPosition(GetPosition());
			}
		}
	}

	void Entity::UpdatePhysicsRotation(bool updateThis)
	{
		if (updateThis && m_scene->IsPlaying() && HasComponent<RigidbodyComponent>())
		{
			auto actor = Physics::GetScene()->GetActor(*this);
			if (actor)
			{
				const glm::quat tempRotation = GetRotation();
				actor->SetRotation(tempRotation, true, false);
			}
		}
	}
}
