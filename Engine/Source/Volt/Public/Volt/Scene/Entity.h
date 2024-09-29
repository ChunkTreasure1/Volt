#pragma once

#include "Volt/Core/Base.h"
#include "Volt/Scene/Scene.h"

#include <EntitySystem/EntityID.h>

#include <CoreUtilities/VoltGUID.h>
#include <entt.hpp>

namespace Volt
{
	class IComponentTypeDesc;
	class PhysicsActor;

	template<class T>
	concept EntityId = std::is_same<T, uint32_t>::value || std::is_same<T, entt::entity>::value;

	enum class EntityCopyFlags
	{
		None = 0,
		SkipRelationships = BIT(0),
		SkipPrefab = BIT(1),
		SkipTransform = BIT(2),
		SkipCommonData = BIT(3),
		SkipID = BIT(4)
	};

	VT_SETUP_ENUM_CLASS_OPERATORS(EntityCopyFlags)


	class Entity
	{
	public:
		Entity();

		template<EntityId T>
		Entity(T id, Weak<Scene> scene);

		template<EntityId T>
		Entity(T id, Scene* scene);

		Entity(const Entity& entity);

		~Entity();

		Weak<Scene> GetScene() const { return m_scene; }

		const std::string& GetTag() const;
		const std::string ToString() const;

		glm::mat4 GetTransform() const;
		glm::mat4 GetLocalTransform() const;

		glm::vec3 GetForward() const;
		glm::vec3 GetRight() const;
		glm::vec3 GetUp() const;

		glm::vec3 GetLocalForward() const;
		glm::vec3 GetLocalRight() const;
		glm::vec3 GetLocalUp() const;

		glm::vec3 GetPosition() const;
		glm::quat GetRotation() const;
		glm::vec3 GetScale() const;

		const glm::vec3& GetLocalPosition() const;
		const glm::quat& GetLocalRotation() const;
		const glm::vec3& GetLocalScale() const;

		void SetTag(std::string_view tag);

		void SetPosition(const glm::vec3& position, bool updatePhysics = true);
		void SetRotation(const glm::quat& rotation, bool updatePhysics = true);
		void SetScale(const glm::vec3& scale);

		void SetLocalPosition(const glm::vec3& position, bool updatePhysics = true);
		void SetLocalRotation(const glm::quat& rotation, bool updatePhysics = true);
		void SetLocalScale(const glm::vec3& scale);

		void SetParent(Entity parentEntity);
		void AddChild(Entity childEntity);

		void ClearParent();
		void ClearChildren();

		void RemoveChild(Entity entity);

		Ref<PhysicsActor> GetPhysicsActor() const;

		Entity GetParent() const;
		Vector<Entity> GetChildren() const;
		bool HasParent() const;

		void SetVisible(bool state);
		void SetLocked(bool state);

		bool IsVisible() const;
		bool IsLocked() const;

		bool IsValid() const;
		EntityID GetID() const;
		entt::entity GetHandle() const { return m_handle; }

		template<typename T> T& GetComponent();
		template<typename T> const T& GetComponent() const;
		template<typename T> const bool HasComponent() const;
		template<typename T, typename... Args> T& AddComponent(Args&&... args);
		template<typename T> void RemoveComponent();

		void RemoveComponent(const VoltGUID& guid);
		bool HasComponent(std::string_view componentName) const;

		Entity& operator=(const Entity& entity);

		inline bool operator==(const Entity& entity) const { return m_handle == entity.m_handle; }
		inline bool operator!() const { return !IsValid(); }
		inline explicit operator bool() const { return IsValid(); }
		inline explicit operator std::string() const { return ToString(); }
		inline operator entt::entity() const { return m_handle; }
		inline operator uint32_t() const { return static_cast<uint32_t>(m_handle); }

		static Entity Null();
		constexpr static EntityID NullID() { return EntityID(0); }

		// Copies a single entity
		static void Copy(Entity srcEntity, Entity dstEntity, const EntityCopyFlags copyFlags = EntityCopyFlags::SkipRelationships);

		// Duplicates an entire entity tree
		static Entity Duplicate(Entity srcEntity, Ref<Scene> targetScene = nullptr, Entity parent = Entity::Null(), const EntityCopyFlags copyFlags = EntityCopyFlags::None);

	private:
		static void CopyComponent(const uint8_t* srcData, uint8_t* dstData, const size_t offset, const IComponentTypeDesc* compDesc, Entity dstEntity);

		void UpdatePhysicsTranslation(bool updateThis);
		void UpdatePhysicsRotation(bool updateThis);

		Weak<Scene> m_scene;
		entt::entity m_handle = entt::null;
	};

	template<EntityId T>
	inline Entity::Entity(T id, Weak<Scene> scene)
		: m_handle(static_cast<entt::entity>(id)), m_scene(scene)
	{
	}

	template<EntityId T>
	inline Entity::Entity(T id, Scene* scene)
		: m_handle(static_cast<entt::entity>(id))
	{
		m_scene = scene->shared_from_this();
	}

	template<typename T>
	inline T& Entity::GetComponent()
	{
		auto scenePtr = GetScene();
		auto& registry = scenePtr->GetRegistry();
		return registry.get<T>(m_handle);
	}

	template<typename T>
	inline const T& Entity::GetComponent() const
	{
		auto scenePtr = GetScene();
		auto& registry = scenePtr->GetRegistry();
		return registry.get<T>(m_handle);
	}

	template<typename T>
	inline const bool Entity::HasComponent() const
	{
		auto scenePtr = GetScene();
		auto& registry = scenePtr->GetRegistry();
		return registry.any_of<T>(m_handle);
	}

	template<typename T, typename ...Args>
	inline T& Entity::AddComponent(Args && ...args)
	{
		auto scenePtr = GetScene();
		auto& registry = scenePtr->GetRegistry();
		return registry.emplace<T>(m_handle, std::forward<Args>(args)...);
	}

	template<typename T>
	inline void Entity::RemoveComponent()
	{
		auto scenePtr = GetScene();
		auto& registry = scenePtr->GetRegistry();
		registry.remove<T>(m_handle);
	}
}
