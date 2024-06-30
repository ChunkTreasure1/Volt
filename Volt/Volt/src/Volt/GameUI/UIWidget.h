#pragma once

#include <entt.hpp>

namespace Volt
{
	class UIScene;

	class UIWidget
	{
	public:
		UIWidget(entt::entity handle, Weak<UIScene> scene);

		template<typename T> VT_NODISCARD VT_INLINE T& GetComponent();
		template<typename T> VT_NODISCARD VT_INLINE const T& GetComponent() const;
		template<typename T> VT_NODISCARD VT_INLINE const bool HasComponent() const;
		template<typename T, typename... Args> VT_INLINE T& AddComponent(Args&&... args);
		template<typename T> VT_INLINE void RemoveComponent();

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
