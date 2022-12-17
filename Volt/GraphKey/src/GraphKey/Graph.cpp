#include "gkpch.h"
#include "Graph.h"

namespace GraphKey
{
	Graph::Graph()
	{}

	Graph::Graph(const GraphSpecification & spec)
		: mySpecification(spec)
	{}

	void Graph::AddNode(Ref<Node> node)
	{}

	void Graph::AddLink(Ref<Link> link)
	{}

	void Graph::RemoveNode(uint32_t id)
	{}

	void Graph::RemoveLink(uint32_t id)
	{}
}