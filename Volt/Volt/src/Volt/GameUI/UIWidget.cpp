#include "vtpch.h"

#include "Volt/GameUI/UIScene.h"
#include "UIWidget.h"


namespace Volt
{
	UIWidget::UIWidget(entt::entity handle, Weak<UIScene> scene)
		: m_scene(scene), m_handle(handle)
	{
	}
}
