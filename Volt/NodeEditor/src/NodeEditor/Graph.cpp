#include "nepch.h"
#include "Graph.h"

namespace NE
{
	void Graph::AddLink(ax::NodeEditor::PinId startPin, ax::NodeEditor::PinId endPin)
	{
		myLinks.emplace_back(std::make_shared<Link>(GetId(), startPin, endPin));
	}

	void Graph::AddNode(std::shared_ptr<Node> node)
	{
		if (node->myId == (ed::NodeId(0)))
		{
			node->myId = GetId();
		}

		node->myGraph = this;
		myNodes.emplace_back(node);
		node->OnCreate();
	}

	const bool Graph::PinHasConnection(ed::PinId pinId) const
	{
		for (const auto& link : myLinks)
		{
			if (link->endPin == pinId || link->startPin == pinId)
			{
				return true;
			}
		}

		return false;
	}

	const Pin* Graph::FindPin(ax::NodeEditor::PinId pinId) const
	{
		for (auto& node : myNodes)
		{
			for (auto& pin : node->GetInputPins())
			{
				if (pin->id == pinId)
				{
					return pin.get();
				}
			}

			for (auto& pin : node->GetOutputPins())
			{
				if (pin->id == pinId)
				{
					return pin.get();
				}
			}
		}

		return nullptr;
	}
}