#include "gkpch.h"
#include "Graph.h"

#include "GraphKey/Node.h"

namespace GraphKey
{
	Graph::Graph()
	{}

	Graph::Graph(const GraphSpecification & spec)
		: mySpecification(spec)
	{}

	void Graph::AddNode(Ref<Node> node)
	{
		node->id = GetNextId();

		for (auto& input : node->inputs)
		{
			input.id = GetNextId();
		}

		for (auto& output : node->outputs)
		{
			output.id = GetNextId();
		}

		mySpecification.nodes.emplace_back(node);
	}

	void Graph::AddLink(Ref<Link> link)
	{
		mySpecification.links.emplace_back(link);
	}

	void Graph::RemoveNode(uint32_t id)
	{}

	void Graph::RemoveLink(uint32_t id)
	{}
}