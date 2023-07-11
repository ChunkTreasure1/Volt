#include "gkpch.h"
#include "Node.h"

#include "GraphKey/Registry.h"

#include "GraphKey/Nodes/ParameterNodes.h"
#include "GraphKey/Nodes/CustomEventNode.h"

namespace GraphKey
{
	Attribute Node::AttributeConfig(const std::string& name, AttributeDirection direction, const std::function<void()>& function)
	{
		Attribute newAttr{};
		newAttr.name = name;
		newAttr.direction = direction;
		newAttr.function = function;
		newAttr.inputHidden = true;
		newAttr.type = AttributeType::Flow;

		return newAttr;
	}

	const bool Node::InputHasData(uint32_t index)
	{
		VT_CORE_ASSERT(index < inputs.size(), "Index out of bounds!");
		return inputs.at(index).data.has_value();
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

	Ref<Node> Node::CreateCopy(Graph* ownerGraph, Wire::EntityId entity)
	{
		Ref<Node> newNode = Registry::Create(GetRegistryName());

		if (auto paramType = std::dynamic_pointer_cast<ParameterNode>(newNode))
		{
			paramType->parameterId = reinterpret_cast<ParameterNode*>(this)->parameterId;
		}
		else if (auto eventType = std::dynamic_pointer_cast<CustomEventNode>(newNode))
		{
			eventType->eventId = reinterpret_cast<CustomEventNode*>(this)->eventId;
		}

		newNode->myGraph = ownerGraph;
		newNode->id = id;
		newNode->editorState = editorState;
		newNode->nodeEntity = entity;

		for (size_t i = 0; i < inputs.size(); i++)
		{
			newNode->inputs[i].data = inputs[i].data;
			newNode->inputs[i].id = inputs[i].id;
			newNode->inputs[i].links = inputs[i].links;
		}

		for (size_t i = 0; i < outputs.size(); i++)
		{
			newNode->outputs[i].data = outputs[i].data;
			newNode->outputs[i].id = outputs[i].id;
			newNode->outputs[i].links = outputs[i].links;
		}

		return newNode;
	}
	
	const uint32_t Node::GetAttributeIndexFromID(const Volt::UUID attrId) const
	{
		auto itInputs = std::find_if(inputs.begin(), inputs.end(), [attrId](const auto& lhs) { return lhs.id == attrId; });
		if (itInputs != inputs.end())
		{
			return (uint32_t)std::distance(inputs.begin(), itInputs);
		}

		auto itOutputs = std::find_if(outputs.begin(), outputs.end(), [attrId](const auto& lhs) { return lhs.id == attrId; });
		if (itOutputs != outputs.end())
		{
			return (uint32_t)std::distance(outputs.begin(), itOutputs);
		}

		VT_CORE_ASSERT(false, "");
		return 0;
	}
}
