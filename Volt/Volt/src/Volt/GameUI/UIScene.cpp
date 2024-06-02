#include "vtpch.h"
#include "UIScene.h"

#include "Volt/GameUI/UIWidget.h"
#include "Volt/GameUI/UIComponents.h"

#include "Volt/Utility/TimeUtility.h"

namespace Volt
{
	UIWidget UIScene::CreateWidget(const std::string& tag)
	{
		entt::entity handle = m_registry.create();
		
		UIWidget newWidget = UIWidget(handle, shared_from_this());
		newWidget.AddComponent<UITransformComponent>();
		
		auto& idComp = newWidget.AddComponent<UIIDComponent>();
		idComp.timeCreateID = TimeUtility::GetTimeSinceEpoch();
	
		// Tag
		{
			auto& tagComp = newWidget.AddComponent<UITagComponent>();
			if (tag.empty())
			{
				tagComp.tag = "New Tag";
			}
			else
			{
				tagComp.tag = tag;
			}
		}

		return newWidget;
	}
}
