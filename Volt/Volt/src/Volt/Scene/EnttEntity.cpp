#include "vtpch.h"
#include "EnttEntity.h"

#include "Volt/Components/CoreComponents.h"

#include <cassert>

namespace Volt
{
	EnttEntity::EnttEntity()
		: m_id(entt::null)
	{
	}

	EnttEntity::EnttEntity(entt::entity id, Weak<Scene> scene)
		: m_id(id), m_scene(scene)
	{
	}

	EnttEntity::EnttEntity(const EnttEntity& entity)
	{
		*this = entity;
	}

	EnttEntity::~EnttEntity()
	{
	}

	const std::string& EnttEntity::GetTag()
	{
		auto scenePtr = GetScene();
		auto& registry = scenePtr->GetEnTTRegistry();

		assert(registry.any_of<EnTTTagComponent>(m_id) && "Entity must have tag component!");
		return registry.get<EnTTTagComponent>(m_id).tag;
	}

	const uint32_t EnttEntity::GetLayerID() const
	{
		auto scenePtr = GetScene();
		auto& registry = scenePtr->GetEnTTRegistry();

		assert(registry.any_of<EnTTCommonComponent>(m_id) && "Entity must have common component!");
		return registry.get<EnTTCommonComponent>(m_id).layerId;
	}

	void EnttEntity::SetTag(std::string_view tag)
	{
		auto scenePtr = GetScene();
		auto& registry = scenePtr->GetEnTTRegistry();

		assert(registry.any_of<EnTTTagComponent>(m_id) && "Entity must have tag component!");
		registry.get<EnTTTagComponent>(m_id).tag = tag;
	}

	const glm::mat4 EnttEntity::GetTransform() const
	{
		auto scenePtr = GetScene();
		return scenePtr->GetWorldTransform(*this);
	}

	const glm::mat4 EnttEntity::GetLocalTransform() const
	{
		auto scenePtr = GetScene();
		auto& registry = scenePtr->GetEnTTRegistry();

		assert(registry.any_of<EnTTTransformComponent>(m_id) && "Entity must have transform component!");
		return registry.get<EnTTTransformComponent>(m_id).GetTransform();
	}

	const glm::vec3 EnttEntity::GetForward() const
	{
		return glm::rotate(GetRotation(), glm::vec3{ 0.f, 0.f, 1.f });
	}

	const glm::vec3 EnttEntity::GetRight() const
	{
		return glm::rotate(GetRotation(), glm::vec3{ 1.f, 0.f, 0.f });
	}

	const glm::vec3 EnttEntity::GetUp() const
	{
		return glm::rotate(GetRotation(), glm::vec3{ 0.f, 1.f, 0.f });
	}

	const glm::vec3 EnttEntity::GetLocalForward() const
	{
		auto scenePtr = GetScene();
		auto& registry = scenePtr->GetEnTTRegistry();

		assert(registry.any_of<EnTTTransformComponent>(m_id) && "Entity must have transform component!");
		return registry.get<EnTTTransformComponent>(m_id).GetForward();
	}

	const glm::vec3 EnttEntity::GetLocalRight() const
	{
		auto scenePtr = GetScene();
		auto& registry = scenePtr->GetEnTTRegistry();

		assert(registry.any_of<EnTTTransformComponent>(m_id) && "Entity must have transform component!");
		return registry.get<EnTTTransformComponent>(m_id).GetRight();
	}

	const glm::vec3 EnttEntity::GetLocalUp() const
	{
		auto scenePtr = GetScene();
		auto& registry = scenePtr->GetEnTTRegistry();

		assert(registry.any_of<EnTTTransformComponent>(m_id) && "Entity must have transform component!");
		return registry.get<EnTTTransformComponent>(m_id).GetUp();
	}

	const glm::vec3 EnttEntity::GetPosition() const
	{
		auto scenePtr = GetScene();
		return scenePtr->GetWorldPosition(*this);
	}

	const glm::quat EnttEntity::GetRotation() const
	{
		auto scenePtr = GetScene();
		return scenePtr->GetWorldRotation(*this);
	}

	const glm::vec3 EnttEntity::GetScale() const
	{
		auto scenePtr = GetScene();
		return scenePtr->GetWorldScale(*this);
	}

	const glm::vec3& EnttEntity::GetLocalPosition() const
	{
		auto scenePtr = GetScene();
		auto& registry = scenePtr->GetEnTTRegistry();

		assert(registry.any_of<EnTTTransformComponent>(m_id) && "Entity must have transform component!");
		return registry.get<EnTTTransformComponent>(m_id).position;
	}

	const glm::quat& EnttEntity::GetLocalRotation() const
	{
		auto scenePtr = GetScene();
		auto& registry = scenePtr->GetEnTTRegistry();

		assert(registry.any_of<EnTTTransformComponent>(m_id) && "Entity must have transform component!");
		return registry.get<EnTTTransformComponent>(m_id).rotation;
	}

	const glm::vec3& EnttEntity::GetLocalScale() const
	{
		auto scenePtr = GetScene();
		auto& registry = scenePtr->GetEnTTRegistry();

		assert(registry.any_of<EnTTTransformComponent>(m_id) && "Entity must have transform component!");
		return registry.get<EnTTTransformComponent>(m_id).scale;
	}

	void EnttEntity::SetPosition(const glm::vec3& position, bool updatePhysics)
	{
		EnttEntity parent = GetParent();
		Scene::TQS parentTransform{};

		if (parent)
		{
			parentTransform = m_scene.lock()->GetWorldTQS(parent);
		}

		const glm::vec3 translatedPoint = position - parentTransform.position;
		const glm::vec3 invertedScale = 1.f / parentTransform.scale;
		const glm::vec3 rotatedPoint = glm::conjugate(parentTransform.rotation) * translatedPoint;

		const glm::vec3 localPoint = rotatedPoint * invertedScale;
		SetLocalPosition(localPoint, updatePhysics);
	}

	void EnttEntity::SetRotation(const glm::quat& rotation, bool updatePhysics)
	{
		EnttEntity parent = GetParent();
		glm::quat parentRotation = glm::identity<glm::quat>();

		if (parent)
		{
			parentRotation = m_scene.lock()->GetWorldRotation(parent);
		}

		const glm::quat localRotation = glm::conjugate(parentRotation) * rotation;
		SetLocalRotation(localRotation, updatePhysics);
	}

	void EnttEntity::SetScale(const glm::vec3& scale)
	{
		EnttEntity parent = GetParent();
		glm::vec3 parentScale = { 1.f };

		if (parent)
		{
			parentScale = m_scene.lock()->GetWorldScale(parent);
		}

		const glm::vec3 inverseScale = 1.f / parentScale;
		const glm::vec3 localScale = scale * inverseScale;

		SetLocalScale(localScale);
	}

	void EnttEntity::SetLocalPosition(const glm::vec3& position, bool updatePhysics)
	{
		auto scenePtr = GetScene();
		auto& registry = scenePtr->GetEnTTRegistry();

		assert(registry.any_of<EnTTTransformComponent>(m_id) && "Entity must have transform component!");
		registry.get<EnTTTransformComponent>(m_id).position = position;
	}

	void EnttEntity::SetLocalRotation(const glm::quat& rotation, bool updatePhysics)
	{
		auto scenePtr = GetScene();
		auto& registry = scenePtr->GetEnTTRegistry();

		assert(registry.any_of<EnTTTransformComponent>(m_id) && "Entity must have transform component!");
		registry.get<EnTTTransformComponent>(m_id).rotation = rotation;
	}

	void EnttEntity::SetLocalScale(const glm::vec3& scale)
	{
		auto scenePtr = GetScene();
		auto& registry = scenePtr->GetEnTTRegistry();

		assert(registry.any_of<EnTTTransformComponent>(m_id) && "Entity must have transform component!");
		registry.get<EnTTTransformComponent>(m_id).scale = scale;
	}

	void EnttEntity::ClearParent()
	{
		auto scenePtr = GetScene();
		auto& registry = scenePtr->GetEnTTRegistry();

		assert(registry.any_of<EnTTRelationshipComponent>(m_id) && "Entity must have relationship component!");
	
		auto parent = GetParent();
		parent.RemoveChild(*this);

		registry.get<EnTTRelationshipComponent>(m_id).parent = entt::null;
	}

	void EnttEntity::ClearChildren()
	{
		auto scenePtr = GetScene();
		auto& registry = scenePtr->GetEnTTRegistry();

		assert(registry.any_of<EnTTRelationshipComponent>(m_id) && "Entity must have relationship component!");

		auto& children = registry.get<EnTTRelationshipComponent>(m_id).children;

		for (const auto& child : children)
		{
			EnttEntity childEnt{ child, m_scene };
			if (childEnt.GetParent().GetId() == m_id)
			{
				childEnt.GetComponent<EnTTRelationshipComponent>().parent = entt::null;
			}
		}

		children.clear();
	}

	void EnttEntity::RemoveChild(EnttEntity entity)
	{
		auto scenePtr = GetScene();
		auto& registry = scenePtr->GetEnTTRegistry();

		assert(registry.any_of<EnTTRelationshipComponent>(m_id) && "Entity must have relationship component!");

		auto& relComp = registry.get<EnTTRelationshipComponent>(m_id);
		for (uint32_t index = 0; const auto& id : relComp.children)
		{
			if (id == entity.GetId())
			{
				EnttEntity childEnt = { id, m_scene };
				childEnt.GetComponent<EnTTRelationshipComponent>().parent = entt::null;
				
				relComp.children.erase(relComp.children.begin() + index);
				break;
			}

			index++;
		}
	}

	const EnttEntity EnttEntity::GetParent() const
	{
		auto scenePtr = GetScene();
		auto& registry = scenePtr->GetEnTTRegistry();

		assert(registry.any_of<EnTTRelationshipComponent>(m_id) && "Entity must have relationship component!");

		auto& relComp = registry.get<EnTTRelationshipComponent>(m_id);
		if (relComp.parent == entt::null)
		{
			return Null();
		}

		return { relComp.parent, m_scene };
	}

	const std::vector<EnttEntity> EnttEntity::GetChildren() const
	{
		auto scenePtr = GetScene();
		auto& registry = scenePtr->GetEnTTRegistry();

		assert(registry.any_of<EnTTRelationshipComponent>(m_id) && "Entity must have relationship component!");

		const auto& children = registry.get<EnTTRelationshipComponent>(m_id).children;

		std::vector<EnttEntity> result{};
		for (const auto& id : children)
		{
			result.emplace_back(id, m_scene);
		}

		return result;
	}

	const bool EnttEntity::HasParent() const
	{
		auto scenePtr = GetScene();
		auto& registry = scenePtr->GetEnTTRegistry();

		assert(registry.any_of<EnTTRelationshipComponent>(m_id) && "Entity must have relationship component!");

		auto& relComp = registry.get<EnTTRelationshipComponent>(m_id);
		return relComp.parent != entt::null;
	}

	const bool EnttEntity::IsVisible() const
	{
		auto scenePtr = GetScene();
		auto& registry = scenePtr->GetEnTTRegistry();

		assert(registry.any_of<EnTTTransformComponent>(m_id) && "Entity must have transform component!");
		return registry.get<EnTTTransformComponent>(m_id).visible;
	}

	const bool EnttEntity::IsLocked() const
	{
		auto scenePtr = GetScene();
		auto& registry = scenePtr->GetEnTTRegistry();

		assert(registry.any_of<EnTTTransformComponent>(m_id) && "Entity must have transform component!");
		return registry.get<EnTTTransformComponent>(m_id).locked;
	}

	const bool EnttEntity::IsValid() const
	{
		if (m_scene.expired())
		{
			return false;
		}

		auto scenePtr = m_scene.lock();
		return scenePtr->GetEnTTRegistry().valid(m_id);
	}
	
	EnttEntity& EnttEntity::operator=(const EnttEntity& entity)
	{
		m_scene = entity.m_scene;
		m_id = entity.m_id;

		return *this;
	}

	EnttEntity EnttEntity::Null()
	{
		return {};
	}
}
