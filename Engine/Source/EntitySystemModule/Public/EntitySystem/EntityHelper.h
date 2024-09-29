#pragma once

#include "EntitySystem/EntityScene.h"

#include <CoreUtilities/VoltGUID.h>

#include <entt.hpp>
#include <glm/glm.hpp>

namespace Volt
{
	class VTES_API EntityHelper
	{
	public:
		EntityHelper();
		EntityHelper(entt::entity entityHandle, EntityScene* scene);
		EntityHelper(entt::entity entityHandle, const EntityScene* scene);
		~EntityHelper();

		void SetPosition(const glm::vec3& position);
		void SetRotation(const glm::quat& rotation);
		void SetScale(const glm::vec3& scale);

		void SetLocalPosition(const glm::vec3& position);
		void SetLocalRotation(const glm::quat& rotation);
		void SetLocalScale(const glm::vec3& scale);

		void SetTag(const std::string& tag);

		VT_NODISCARD glm::vec3 GetPosition() const;
		VT_NODISCARD glm::quat GetRotation() const;
		VT_NODISCARD glm::vec3 GetScale() const;

		VT_NODISCARD const glm::vec3& GetLocalPosition() const;
		VT_NODISCARD const glm::quat& GetLocalRotation() const;
		VT_NODISCARD const glm::vec3& GetLocalScale() const;

		VT_NODISCARD glm::vec3 GetForward() const;
		VT_NODISCARD glm::vec3 GetRight() const;
		VT_NODISCARD glm::vec3 GetUp() const;

		VT_NODISCARD glm::vec3 GetLocalForward() const;
		VT_NODISCARD glm::vec3 GetLocalRight() const;
		VT_NODISCARD glm::vec3 GetLocalUp() const;

		VT_NODISCARD const std::string& GetTag() const;

		VT_NODISCARD bool HasParent() const;
		VT_NODISCARD EntityHelper GetParent() const;

		VT_NODISCARD VT_INLINE bool IsValid() const { return m_handle != entt::null && m_sceneReference != nullptr && m_sceneReference->GetRegistry().valid(m_handle); }

		VT_NODISCARD EntityID GetID() const;
		VT_NODISCARD VT_INLINE entt::entity GetHandle() const { return m_handle; }

		template<typename T> VT_NODISCARD T& GetComponent();
		template<typename T> VT_NODISCARD const T& GetComponent() const;
		template<typename T> VT_NODISCARD bool HasComponent() const;
		template<typename T, typename... Args> T& AddComponent(Args&&... args);
		template<typename T> void RemoveComponent();
		void RemoveComponent(const VoltGUID& guid);
		VT_NODISCARD bool HasComponent(std::string_view componentName) const;

		VT_NODISCARD VT_INLINE explicit operator bool() const { return IsValid(); }

		VT_NODISCARD static EntityHelper Null();

	private:
		EntityScene* m_sceneReference = nullptr;
		entt::entity m_handle = entt::null;
	};

	template<typename T>
	inline T& EntityHelper::GetComponent()
	{
		VT_ENSURE(IsValid());

		auto& registry = m_sceneReference->GetRegistry();
		VT_ENSURE(registry.any_of<T>(m_handle));
		return registry.get<T>(m_handle);
	}
	
	template<typename T>
	inline const T& EntityHelper::GetComponent() const
	{
		VT_ENSURE(IsValid());

		auto& registry = m_sceneReference->GetRegistry();
		VT_ENSURE(registry.any_of<T>(m_handle));
		return registry.get<T>(m_handle);
	}
	
	template<typename T>
	inline bool EntityHelper::HasComponent() const
	{
		VT_ENSURE(IsValid());

		auto& registry = m_sceneReference->GetRegistry();
		return registry.any_of<T>(m_handle);
	}
	
	template<typename T, typename ...Args>
	inline T& EntityHelper::AddComponent(Args && ...args)
	{
		VT_ENSURE(IsValid());

		auto& registry = m_sceneReference->GetRegistry();
		VT_ENSURE(!registry.any_of<T>(m_handle));
		return registry.emplace<T>(m_handle, std::forward<Args>(args)...);
	}
	
	template<typename T>
	inline void EntityHelper::RemoveComponent()
	{
		VT_ENSURE(IsValid());

		auto& registry = m_sceneReference->GetRegistry();
		registry.remove<T>(m_handle);
	}
}
