#include "vtpch.h"
#include "Entity.h"

#include "Volt/Components/CoreComponents.h"

#include "Volt/Scripting/Mono/MonoScriptEngine.h"

#include "Volt/Utility/StringUtility.h"

#include <cassert>

namespace Volt
{
	Entity::Entity()
		: m_id(entt::null)
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
		auto scenePtr = GetScene();
		auto& registry = scenePtr->GetRegistry();

		assert(registry.any_of<TagComponent>(m_id) && "Entity must have tag component!");
		return registry.get<TagComponent>(m_id).tag;
	}

	const std::string Entity::ToString() const 
	{
		return std::to_string(static_cast<uint32_t>(m_id));
	}

	const uint32_t Entity::GetLayerID() const
	{
		auto scenePtr = GetScene();
		auto& registry = scenePtr->GetRegistry();

		assert(registry.any_of<CommonComponent>(m_id) && "Entity must have common component!");
		return registry.get<CommonComponent>(m_id).layerId;
	}

	void Entity::SetTag(std::string_view tag)
	{
		auto scenePtr = GetScene();
		auto& registry = scenePtr->GetRegistry();

		assert(registry.any_of<TagComponent>(m_id) && "Entity must have tag component!");
		registry.get<TagComponent>(m_id).tag = tag;
	}

	const glm::mat4 Entity::GetTransform() const
	{
		auto scenePtr = GetScene();
		return scenePtr->GetWorldTransform(*this);
	}

	const glm::mat4 Entity::GetLocalTransform() const
	{
		auto scenePtr = GetScene();
		auto& registry = scenePtr->GetRegistry();

		assert(registry.any_of<TransformComponent>(m_id) && "Entity must have transform component!");
		return registry.get<TransformComponent>(m_id).GetTransform();
	}

	const glm::vec3 Entity::GetForward() const
	{
		return glm::rotate(GetRotation(), glm::vec3{ 0.f, 0.f, 1.f });
	}

	const glm::vec3 Entity::GetRight() const
	{
		return glm::rotate(GetRotation(), glm::vec3{ 1.f, 0.f, 0.f });
	}

	const glm::vec3 Entity::GetUp() const
	{
		return glm::rotate(GetRotation(), glm::vec3{ 0.f, 1.f, 0.f });
	}

	const glm::vec3 Entity::GetLocalForward() const
	{
		auto scenePtr = GetScene();
		auto& registry = scenePtr->GetRegistry();

		assert(registry.any_of<TransformComponent>(m_id) && "Entity must have transform component!");
		return registry.get<TransformComponent>(m_id).GetForward();
	}

	const glm::vec3 Entity::GetLocalRight() const
	{
		auto scenePtr = GetScene();
		auto& registry = scenePtr->GetRegistry();

		assert(registry.any_of<TransformComponent>(m_id) && "Entity must have transform component!");
		return registry.get<TransformComponent>(m_id).GetRight();
	}

	const glm::vec3 Entity::GetLocalUp() const
	{
		auto scenePtr = GetScene();
		auto& registry = scenePtr->GetRegistry();

		assert(registry.any_of<TransformComponent>(m_id) && "Entity must have transform component!");
		return registry.get<TransformComponent>(m_id).GetUp();
	}

	const glm::vec3 Entity::GetPosition() const
	{
		auto scenePtr = GetScene();
		return scenePtr->GetWorldPosition(*this);
	}

	const glm::quat Entity::GetRotation() const
	{
		auto scenePtr = GetScene();
		return scenePtr->GetWorldRotation(*this);
	}

	const glm::vec3 Entity::GetScale() const
	{
		auto scenePtr = GetScene();
		return scenePtr->GetWorldScale(*this);
	}

	const glm::vec3& Entity::GetLocalPosition() const
	{
		auto scenePtr = GetScene();
		auto& registry = scenePtr->GetRegistry();

		assert(registry.any_of<TransformComponent>(m_id) && "Entity must have transform component!");
		return registry.get<TransformComponent>(m_id).position;
	}

	const glm::quat& Entity::GetLocalRotation() const
	{
		auto scenePtr = GetScene();
		auto& registry = scenePtr->GetRegistry();

		assert(registry.any_of<TransformComponent>(m_id) && "Entity must have transform component!");
		return registry.get<TransformComponent>(m_id).rotation;
	}

	const glm::vec3& Entity::GetLocalScale() const
	{
		auto scenePtr = GetScene();
		auto& registry = scenePtr->GetRegistry();

		assert(registry.any_of<TransformComponent>(m_id) && "Entity must have transform component!");
		return registry.get<TransformComponent>(m_id).scale;
	}

	void Entity::SetPosition(const glm::vec3& position, bool updatePhysics)
	{
		Entity parent = GetParent();
		Scene::TQS parransform{};

		if (parent)
		{
			parransform = m_scene.lock()->GetWorldTQS(parent);
		}

		const glm::vec3 translatedPoint = position - parransform.position;
		const glm::vec3 invertedScale = 1.f / parransform.scale;
		const glm::vec3 rotatedPoint = glm::conjugate(parransform.rotation) * translatedPoint;

		const glm::vec3 localPoint = rotatedPoint * invertedScale;
		SetLocalPosition(localPoint, updatePhysics);
	}

	void Entity::SetRotation(const glm::quat& rotation, bool updatePhysics)
	{
		Entity parent = GetParent();
		glm::quat parentRotation = glm::identity<glm::quat>();

		if (parent)
		{
			parentRotation = m_scene.lock()->GetWorldRotation(parent);
		}

		const glm::quat localRotation = glm::conjugate(parentRotation) * rotation;
		SetLocalRotation(localRotation, updatePhysics);
	}

	void Entity::SetScale(const glm::vec3& scale)
	{
		Entity parent = GetParent();
		glm::vec3 parentScale = { 1.f };

		if (parent)
		{
			parentScale = m_scene.lock()->GetWorldScale(parent);
		}

		const glm::vec3 inverseScale = 1.f / parentScale;
		const glm::vec3 localScale = scale * inverseScale;

		SetLocalScale(localScale);
	}

	void Entity::SetLocalPosition(const glm::vec3& position, bool updatePhysics)
	{
		auto scenePtr = GetScene();
		auto& registry = scenePtr->GetRegistry();

		assert(registry.any_of<TransformComponent>(m_id) && "Entity must have transform component!");
		registry.get<TransformComponent>(m_id).position = position;
	}

	void Entity::SetLocalRotation(const glm::quat& rotation, bool updatePhysics)
	{
		auto scenePtr = GetScene();
		auto& registry = scenePtr->GetRegistry();

		assert(registry.any_of<TransformComponent>(m_id) && "Entity must have transform component!");
		registry.get<TransformComponent>(m_id).rotation = rotation;
	}

	void Entity::SetLocalScale(const glm::vec3& scale)
	{
		auto scenePtr = GetScene();
		auto& registry = scenePtr->GetRegistry();

		assert(registry.any_of<TransformComponent>(m_id) && "Entity must have transform component!");
		registry.get<TransformComponent>(m_id).scale = scale;
	}

	void Entity::ClearParent()
	{
		auto scenePtr = GetScene();
		auto& registry = scenePtr->GetRegistry();

		assert(registry.any_of<RelationshipComponent>(m_id) && "Entity must have relationship component!");
	
		auto parent = GetParent();
		parent.RemoveChild(*this);

		registry.get<RelationshipComponent>(m_id).parent = entt::null;
	}

	void Entity::ClearChildren()
	{
		auto scenePtr = GetScene();
		auto& registry = scenePtr->GetRegistry();

		assert(registry.any_of<RelationshipComponent>(m_id) && "Entity must have relationship component!");

		auto& children = registry.get<RelationshipComponent>(m_id).children;

		for (auto& childId : children)
		{
			Entity child{ childId, m_scene };

			if (child.GetParent().GetID() == m_id)
			{
				child.GetComponent<RelationshipComponent>().parent = entt::null;
			}
		}

		children.clear();
	}

	void Entity::RemoveChild(Entity entity)
	{
		auto scenePtr = GetScene();
		auto& registry = scenePtr->GetRegistry();

		assert(registry.any_of<RelationshipComponent>(m_id) && "Entity must have relationship component!");

		auto& relComp = registry.get<RelationshipComponent>(m_id);
		for (uint32_t index = 0; auto& childId : relComp.children)
		{
			Entity childEnt{ childId, m_scene };
			
			if (childEnt.GetID() == entity.GetID())
			{
				childEnt.GetComponent<RelationshipComponent>().parent = entt::null;
				
				relComp.children.erase(relComp.children.begin() + index);
				break;
			}

			index++;
		}
	}

	const Entity Entity::GetParent() const
	{
		auto scenePtr = GetScene();
		auto& registry = scenePtr->GetRegistry();

		assert(registry.any_of<RelationshipComponent>(m_id) && "Entity must have relationship component!");

		auto& relComp = registry.get<RelationshipComponent>(m_id);
		if (relComp.parent == entt::null)
		{
			return Null();
		}

		return { relComp.parent, m_scene };
	}

	const std::vector<Entity> Entity::GetChildren() const
	{
		auto scenePtr = GetScene();
		auto& registry = scenePtr->GetRegistry();

		assert(registry.any_of<RelationshipComponent>(m_id) && "Entity must have relationship component!");

		const auto& children = registry.get<RelationshipComponent>(m_id).children;

		std::vector<Entity> result{};
		for (const auto& id : children)
		{
			result.emplace_back(id, m_scene);
		}

		return result;
	}

	const bool Entity::HasParent() const
	{
		auto scenePtr = GetScene();
		auto& registry = scenePtr->GetRegistry();

		assert(registry.any_of<RelationshipComponent>(m_id) && "Entity must have relationship component!");

		auto& relComp = registry.get<RelationshipComponent>(m_id);
		return relComp.parent != entt::null;
	}

	const bool Entity::HasComponent(std::string_view componentName) const
	{
		const std::string lowerCompName = ::Utility::ToLower(std::string(componentName));

		auto scenePtr = GetScene();
		auto& registry = scenePtr->GetRegistry();

		for (auto&& curr : registry.storage())
		{
			if (auto& storage = curr.second; storage.contains(m_id))
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

		if (MonoScriptEngine::IsRunning() && HasComponent<MonoScriptComponent>())
		{
			const auto& monoComp = GetComponent<MonoScriptComponent>();
			for (const auto& scriptId : monoComp.scriptIds)
			{
				if (state)
				{
					MonoScriptEngine::OnEnableInstance(scriptId);
				}
				else
				{
					MonoScriptEngine::OnDisableInstance(scriptId);
				}
			}
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

	const bool Entity::IsVisible() const
	{
		auto scenePtr = GetScene();
		auto& registry = scenePtr->GetRegistry();

		assert(registry.any_of<TransformComponent>(m_id) && "Entity must have transform component!");
		return registry.get<TransformComponent>(m_id).visible;
	}

	const bool Entity::IsLocked() const
	{
		auto scenePtr = GetScene();
		auto& registry = scenePtr->GetRegistry();

		assert(registry.any_of<TransformComponent>(m_id) && "Entity must have transform component!");
		return registry.get<TransformComponent>(m_id).locked;
	}

	const bool Entity::IsValid() const
	{
		if (m_scene.expired() || m_id == static_cast<entt::entity>(0))
		{
			return false;
		}

		auto scenePtr = m_scene.lock();
		return scenePtr->GetRegistry().valid(m_id);
	}
	
	Entity& Entity::operator=(const Entity& entity)
	{
		m_scene = entity.m_scene;
		m_id = entity.m_id;

		return *this;
	}

	Entity Entity::Null()
	{
		return {};
	}

	void Entity::Copy(Entity srcEntity, Entity dstEntity)
	{
		// #TODO_Ivar: Implement MonoComponent copying

		auto srcScene = srcEntity.GetScene();
		auto& srcRegistry = srcScene->GetRegistry();

		auto dstScene = dstEntity.GetScene();
		auto& dstRegistry = dstScene->GetRegistry();

		for (auto&& curr : srcRegistry.storage())
		{
			auto& storage = curr.second;

			if (!storage.contains(srcEntity.GetID()))
			{
				continue;
			}

			const IComponentTypeDesc* componentDesc = reinterpret_cast<const IComponentTypeDesc*>(Volt::ComponentRegistry::GetTypeDescFromName(storage.type().name()));
			if (!componentDesc)
			{
				continue;
			}

			if (componentDesc->GetValueType() != ValueType::Component)
			{
				continue;
			}

			ComponentRegistry::Helpers::AddComponentWithGUID(componentDesc->GetGUID(), dstRegistry, dstEntity.GetID());
			void* voidCompPtr = Volt::ComponentRegistry::Helpers::GetComponentWithGUID(componentDesc->GetGUID(), dstRegistry, dstEntity.GetID());
			uint8_t* componentData = reinterpret_cast<uint8_t*>(voidCompPtr);

			CopyComponent(reinterpret_cast<const uint8_t*>(storage.get(srcEntity.GetID())), componentData, 0, componentDesc);
		}

		// Clear the relationship component, as this is a standalone entity
		{
			RelationshipComponent& relComp = dstEntity.GetComponent<RelationshipComponent>();
			relComp.children.clear();
			relComp.parent = entt::null;
		}
	}

	void Entity::CopyComponent(const uint8_t* srcData, uint8_t* dstData, const size_t offset, const IComponentTypeDesc* compDesc)
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
						CopyComponent(srcData, dstData, offset + member.offset, memberCompDesc);
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
	}
}
