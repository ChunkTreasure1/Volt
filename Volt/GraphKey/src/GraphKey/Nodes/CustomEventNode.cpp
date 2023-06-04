#include "gkpch.h"
#include "CustomEventNode.h"

namespace GraphKey
{
	CallCustomEventNode::CallCustomEventNode()
	{
		inputs =
		{
			AttributeConfig("", AttributeDirection::Input, GK_BIND_FUNCTION(CallCustomEventNode::CallEvent)),
			AttributeConfigDefault<Volt::Entity>("Target", AttributeDirection::Input, Volt::Entity{ 0, nullptr })
		};

		outputs =
		{
			AttributeConfig("", AttributeDirection::Output)
		};
	}

	const std::string CallCustomEventNode::GetName()
	{
		const std::string nullMessage = "NULL EVENT";

		if (!myGraph->IsAttributeLinked(inputs.at(1).id))
		{
			return nullMessage;
		}

		auto entity = GetInput<Volt::Entity>(1);
		if (!entity)
		{
			return nullMessage;
		}

		if (!entity.HasComponent<Volt::VisualScriptingComponent>())
		{
			return nullMessage;
		}

		auto& comp = entity.GetComponent<Volt::VisualScriptingComponent>();
		if (!comp.graph)
		{
			return nullMessage;
		}

		return "Call " + comp.graph->GetEventNameFromId(eventId);
	}

	void CallCustomEventNode::CallEvent()
	{
		auto entity = GetInput<Volt::Entity>(1);
		if (!entity)
		{
			return;
		}

		if (!entity.HasComponent<Volt::VisualScriptingComponent>())
		{
			return;
		}

		auto& comp = entity.GetComponent<Volt::VisualScriptingComponent>();
		if (!comp.graph)
		{
			return;
		}

		comp.graph->GetEventSystem().Dispatch(eventId);
		ActivateOutput(0);
	}

	RecieveCustomEventNode::RecieveCustomEventNode()
	{
		outputs =
		{
			AttributeConfig("", AttributeDirection::Output)
		};
	}

	void RecieveCustomEventNode::OnCopy()
	{
		myGraph->GetEventSystem().RegisterListener(id, eventId, [&]()
		{
			ActivateOutput(0);
		});
	}

	RecieveCustomEventNode::~RecieveCustomEventNode()
	{
		if (myGraph)
		{
			myGraph->GetEventSystem().UnregisterListener(id, eventId);
		}
	}
}
