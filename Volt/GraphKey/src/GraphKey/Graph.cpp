#include "gkpch.h"
#include "Graph.h"

#include "GraphKey/Node.h"

namespace GraphKey
{
	Graph::Graph()
	{}

	Graph::Graph(Volt::Entity entity)
		: myEntity(entity)
	{}

	Graph::Graph(const GraphSpecification& spec)
		: mySpecification(spec)
	{}

	void Graph::OnEvent(Volt::Event & e)
	{
		for (const auto& n : mySpecification.nodes)
		{
			n->OnEvent(e);
		}
	}

	void Graph::AddNode(Ref<Node> node)
	{
		node->myGraph = this;
		mySpecification.nodes.emplace_back(node);
	}

	void Graph::AddLink(Ref<Link> link)
	{
		mySpecification.links.emplace_back(link);
	}

	void Graph::CreateLink(const Volt::UUID inputId, const Volt::UUID outputId)
	{
		Ref<Link> newLink = CreateRef<Link>();
		newLink->input = inputId;
		newLink->output = outputId;

		mySpecification.links.emplace_back(newLink);

		Attribute* input = GetAttributeByID(inputId);
		Attribute* output = GetAttributeByID(outputId);

		input->links.emplace_back(newLink->id);
		output->links.emplace_back(newLink->id);
	}

	void Graph::RemoveNode(uint32_t id)
	{}

	void Graph::RemoveLink(uint32_t id)
	{}

	Attribute* Graph::GetAttributeByID(const Volt::UUID id) const
	{
		for (const auto& node : mySpecification.nodes)
		{
			for (auto& input : node->inputs)
			{
				if (input.id == id)
				{
					return &input;
				}
			}

			for (auto& output : node->outputs)
			{
				if (output.id == id)
				{
					return &output;
				}
			}
		}

		return nullptr;
	}

	Ref<Link> Graph::GetLinkByID(const Volt::UUID id)
	{
		if (auto it = std::find_if(mySpecification.links.begin(), mySpecification.links.end(), [&](const Ref<Link>& link) { return link->id == id; }); it != mySpecification.links.end())
		{
			return *it;
		}

		return nullptr;
	}

	Ref<Node> Graph::GetNodeByID(const Volt::UUID id)
	{
		if (auto it = std::find_if(mySpecification.nodes.begin(), mySpecification.nodes.end(), [&](const Ref<Node>& node) { return node->id == id; }); it != mySpecification.nodes.end())
		{
			return *it;
		}

		return nullptr;
	}

	const bool Graph::IsAttributeLinked(const Volt::UUID id) const
	{
		const auto* attr = GetAttributeByID(id);
		if (attr)
		{
			return !attr->links.empty();
		}

		return false;
	}

	void Graph::Copy(Ref<Graph> srcGraph, Ref<Graph> dstGraph)
	{
		dstGraph->mySpecification = srcGraph->mySpecification;
	}
}
