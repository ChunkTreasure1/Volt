#pragma once

#include "Volt/Core/Base.h"
#include "Volt/Scene/Scene.h"

namespace Volt
{
	class Entity
	{
	public:
		Entity();
		Entity(entt::entity id, Weak<Scene> scene);
		Entity(entt::entity id, Scene* scene);
		Entity(const Entity& entity);

		~Entity();

		Ref<Scene> GetScene() const { return m_scene.lock(); }

		const std::string& GetTag() const;
		const std::string ToString() const;

		const uint32_t GetLayerID() const;

		const glm::mat4 GetTransform() const;
		const glm::mat4 GetLocalTransform() const;

		const glm::vec3 GetForward() const;
		const glm::vec3 GetRight() const;
		const glm::vec3 GetUp() const;

		const glm::vec3 GetLocalForward() const;
		const glm::vec3 GetLocalRight() const;
		const glm::vec3 GetLocalUp() const;

		const glm::vec3 GetPosition() const;
		const glm::quat GetRotation() const;
		const glm::vec3 GetScale() const;

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

		void ClearParent();
		void ClearChildren();

		void RemoveChild(Entity entity);

		const Entity GetParent() const;
		const std::vector<Entity> GetChildren() const;
		const bool HasParent() const;

		void SetVisible(bool state);
		void SetLocked(bool state);

		const bool IsVisible() const;
		const bool IsLocked() const;

		const bool IsValid() const;
		inline const entt::entity GetID() const { return m_id; }
		inline const uint32_t GetUIntID() const { return static_cast<uint32_t>(m_id); }

		template<typename T> T& GetComponent();
		template<typename T> const T& GetComponent() const;
		template<typename T> const bool HasComponent() const;
		template<typename T, typename... Args> T& AddComponent(Args&&... args);
		template<typename T> void RemoveComponent();

		const bool HasComponent(std::string_view componentName) const;

		Entity& operator=(const Entity& entity);

		inline bool operator==(const Entity& entity) const { return m_id == entity.m_id; }
		inline bool operator!() const { return !IsValid(); }
		inline explicit operator bool() const { return IsValid(); }
		inline explicit operator std::string() const { return ToString(); }

		static Entity Null();

	private:
		Weak<Scene> m_scene;
		entt::entity m_id = entt::null;
	};

	template<typename T>
	inline T& Entity::GetComponent()
	{
		auto scenePtr = GetScene();
		auto& registry = scenePtr->GetRegistry();
		return registry.get<T>(m_id);
	}

	template<typename T>
	inline const T& Entity::GetComponent() const
	{
		auto scenePtr = GetScene();
		auto& registry = scenePtr->GetRegistry();
		return registry.get<T>(m_id);
	}

	template<typename T>
	inline const bool Entity::HasComponent() const
	{
		auto scenePtr = GetScene();
		auto& registry = scenePtr->GetRegistry();
		return registry.any_of<T>(m_id);
	}

	template<typename T, typename ...Args>
	inline T& Entity::AddComponent(Args && ...args)
	{
		auto scenePtr = GetScene();
		auto& registry = scenePtr->GetRegistry();
		return registry.emplace<T>(m_id);
	}

	template<typename T>
	inline void Entity::RemoveComponent()
	{
		auto scenePtr = GetScene();
		auto& registry = scenePtr->GetRegistry();
		registry.remove<T>(m_id);
	}
}
