#include "gkpch.h"
#include "Node.h"

namespace GraphKey
{
	Node::Node(const Ref<Node> node)
	{
	}

	Attribute Node::AttributeConfig(const std::string& name, AttributeDirection direction, const std::function<void()>& function)
	{
		Attribute newAttr{};
		newAttr.name = name;
		newAttr.direction = direction;
		newAttr.function = function;
		newAttr.hidden = true;
		newAttr.type = AttributeType::Flow;

		return newAttr;
	}
	
	void Node::ActivateOutput(uint32_t index)
	{
		for (const auto& linkId : outputs[index].links)
		{
			const auto link = myGraph->GetLinkByID(linkId);
			if (!link)
			{
				continue;
			}

			const auto attr = myGraph->GetAttributeByID(link->input);
			if (!attr)
			{
				continue;
			}

			if (attr->function)
			{
				attr->function();
			}
		}
	}
}