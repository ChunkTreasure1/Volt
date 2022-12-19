#include "gkpch.h"
#include "BaseNodes.h"

#include "GraphKey/Registry.h"

namespace GraphKey
{
	GK_REGISTER_NODE(StartNode);
	GK_REGISTER_NODE(UpdateNode);

	StartNode::StartNode()
	{
		outputs =
		{
			AttributeConfig("", AttributeDirection::Output, nullptr)
		};
	}

	void StartNode::OnEvent(Volt::Event & e)
	{
		Volt::EventDispatcher dispatcher{ e };
		dispatcher.Dispatch<Volt::OnScenePlayEvent>([this](Volt::OnScenePlayEvent& e) { ActivateOutput(0); return false; });
	}
	
	UpdateNode::UpdateNode()
	{
		outputs =
		{
			AttributeConfig("", AttributeDirection::Output, nullptr)
		};
	}

	void UpdateNode::OnEvent(Volt::Event & e)
	{
		Volt::EventDispatcher dispatcher{ e };
		dispatcher.Dispatch<Volt::AppUpdateEvent>([this](Volt::AppUpdateEvent& e) { ActivateOutput(0); return false; });
	}

}