#pragma once

#include "Volt/Scene/EntityID.h"

#include <entt.hpp>

namespace Volt
{
	class UIScene;

	class UIWidget
	{
	public:
		UIWidget();
		UIWidget(entt::entity handle, Weak<UIScene> scene);

		template<typename T> VT_NODISCARD VT_INLINE T& GetComponent();
		template<typename T> VT_NODISCARD VT_INLINE const T& GetComponent() const;
		template<typename T> VT_NODISCARD VT_INLINE const bool HasComponent() const;
		template<typename T, typename... Args> VT_INLINE T& AddComponent(Args&&... args);
		template<typename T> VT_INLINE void RemoveComponent();

		const bool IsValid() const;
		const EntityID GetID() const;

		inline bool operator!() const { return !IsValid(); }
		inline explicit operator bool() const { return IsValid(); }

		static UIWidget Null();

	private:
		entt::entity m_handle;
		Weak<UIScene> m_scene;
	};

	template<typename T>
	VT_NODISCARD VT_INLINE T& UIWidget::GetComponent()
	{
		auto& registry = m_scene->GetRegistry();
		return registry.get<T>(m_handle);
	}

	template<typename T>
	VT_NODISCARD VT_INLINE const T& UIWidget::GetComponent() const
	{
		auto& registry = m_scene->GetRegistry();
		return registry.get<T>(m_handle);
	}

	template<typename T>
	VT_NODISCARD VT_INLINE const bool UIWidget::HasComponent() const
	{
		auto& registry = m_scene->GetRegistry();
		return registry.any_of<T>(m_handle);
	}

	inline const bool UIWidget::IsValid() const
	{
		return m_handle != entt::null && !m_scene.IsExpired();
	}

	template<typename T, typename ...Args>
	VT_INLINE T& UIWidget::AddComponent(Args && ...args)
	{
		auto& registry = m_scene->GetRegistry();
		return registry.emplace<T>(m_handle, std::forward<Args>(args)...);
	}

	template<typename T>
	VT_INLINE void UIWidget::RemoveComponent()
	{
		auto& registry = m_scene->GetRegistry();
		registry.remove<T>(m_handle);
	}
}
