#pragma once

#include "Volt/Core/Base.h"
#include "Volt/Scene/Scene.h"

namespace Volt
{
	class EnttEntity
	{
	public:
		EnttEntity();
		EnttEntity(entt::entity id, Weak<Scene> scene);
		EnttEntity(const EnttEntity& entity);

		~EnttEntity();

		Ref<Scene> GetScene() const { return m_scene.lock(); }

		const std::string& GetTag();
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

		void RemoveChild(EnttEntity entity);

		const EnttEntity GetParent() const;
		const std::vector<EnttEntity> GetChildren() const;
		const bool HasParent() const;

		const bool IsVisible() const;
		const bool IsLocked() const;

		const bool IsValid() const;
		inline const entt::entity GetId() const { return m_id; }

		template<typename T> T& GetComponent();
		template<typename T> const T& GetComponent() const;
		template<typename T> const bool HasComponent() const;
		template<typename T, typename... Args> T& AddComponent(Args&&... args);
		template<typename T> void RemoveComponent();

		EnttEntity& operator=(const EnttEntity& entity);

		inline bool operator==(const EnttEntity& entity) const { return m_id == entity.m_id; }
		inline bool operator!() const { return !IsValid(); }
		inline explicit operator bool() const { return IsValid(); }

		static EnttEntity Null();

	private:
		Weak<Scene> m_scene;
		entt::entity m_id = entt::null;
	};

	template<typename T>
	inline T& EnttEntity::GetComponent()
	{
		auto& registry = m_scene->GetEnTTRegistry();
		return registry.get<T>(m_id);
	}

	template<typename T>
	inline const T& EnttEntity::GetComponent() const
	{
		auto& registry = m_scene->GetEnTTRegistry();
		return registry.get<T>(m_id);
	}

	template<typename T>
	inline const bool EnttEntity::HasComponent() const
	{
		auto& registry = m_scene->GetEnTTRegistry();
		return registry.any_of<T>(m_id);
	}

	template<typename T, typename ...Args>
	inline T& EnttEntity::AddComponent(Args && ...args)
	{
		auto& registry = m_scene->GetEnTTRegistry();
		return registry.emplace<T>(m_id);
	}

	template<typename T>
	inline void EnttEntity::RemoveComponent()
	{
		auto& registry = m_scene->GetEnTTRegistry();
		registry.remove<T>(m_id);
	}
}
