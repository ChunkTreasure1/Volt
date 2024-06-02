#pragma once

#include <entt.hpp>

namespace Volt
{
	class UIWidget;

	class UIScene : public std::enable_shared_from_this<UIScene>
	{
	public:
		UIScene() = default;

		VT_NODISCARD VT_INLINE entt::registry& GetRegistry() { return m_registry; }
		
		UIWidget CreateWidget(const std::string& tag = "");

		template<typename... T, typename F>
		VT_INLINE void ForEachWithComponents(const F& func);

	private:
		entt::registry m_registry;
	};

	template<typename ...T, typename F>
	inline void UIScene::ForEachWithComponents(const F& func)
	{
		auto view = m_registry.view<T...>();
		view.each(func);
	}
}
