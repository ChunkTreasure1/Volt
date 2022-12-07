#pragma once

#include "Node.h"

#include <vector>
#include <memory>

namespace NE
{
	class Graph
	{
	public:

		void AddLink(ax::NodeEditor::PinId startPin, ax::NodeEditor::PinId endPin);
		const Pin* FindPin(ax::NodeEditor::PinId pinId) const;

		void AddNode(std::shared_ptr<Node> node);

		const bool PinHasConnection(ed::PinId pinId) const;

		inline const std::vector<std::shared_ptr<Node>>& GetNodes() const { return myNodes; }
		inline const std::vector<std::shared_ptr<Link>>& GetLinks() const { return myLinks; }
		inline const uint32_t GetId() { return myCurrentId++; }
	
	private:
		uint32_t myCurrentId = 1; // 0 is invalid

		std::vector<std::shared_ptr<Node>> myNodes;
		std::vector<std::shared_ptr<Link>> myLinks;
	};
}