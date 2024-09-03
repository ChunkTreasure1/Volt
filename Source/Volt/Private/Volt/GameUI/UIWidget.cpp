#include "vtpch.h"

// Note: Needs to be declared before UIWidget.h
#include "Volt/GameUI/UIScene.h"

#include "Volt/GameUI/UIWidget.h"

#include "Volt/GameUI/UIComponents.h"

namespace Volt
{
	UIWidget::UIWidget()
		: m_handle(entt::null)
	{
	}

	UIWidget::UIWidget(entt::entity handle, Weak<UIScene> scene)
		: m_scene(scene), m_handle(handle)
	{
	}

	UIWidget UIWidget::Null()
	{
		return {};
	}

	const EntityID UIWidget::GetID() const
	{
		auto& registry = m_scene->GetRegistry();
		return registry.get<UIIDComponent>(m_handle).id;
	}
}
