#include "gkpch.h"
#include "Graph.h"

#include "GraphKey/Node.h"

namespace GraphKey
{
	Graph::Graph()
	{}

	Graph::Graph(Wire::EntityId entity)
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
		newLink->input = outputId;
		newLink->output = inputId;

		mySpecification.links.emplace_back(newLink);

		Attribute* input = GetAttributeByID(outputId);
		Attribute* output = GetAttributeByID(inputId);

		input->links.emplace_back(newLink->id);
		output->links.emplace_back(newLink->id);
	}

	void Graph::RemoveNode(Volt::UUID id)
	{
		auto it = std::find_if(mySpecification.nodes.begin(), mySpecification.nodes.end(), [&id](const auto& lhs)
			{
				return lhs->id == id;
			});

		if (it == mySpecification.nodes.end())
		{
			return;
		}

		Ref<Node> node = *it;
		for (const auto& attr : node->inputs)
		{
			for (const auto& l : attr.links)
			{
				RemoveLink(l);
			}
		}

		for (const auto& attr : node->outputs)
		{
			for (const auto& l : attr.links)
			{
				RemoveLink(l);
			}
		}

		mySpecification.nodes.erase(it);
	}

	void Graph::RemoveLink(Volt::UUID id)
	{
		auto it = std::find_if(mySpecification.links.begin(), mySpecification.links.end(), [&id](const auto& lhs)
			{
				return lhs->id == id;
			});

		if (it == mySpecification.links.end())
		{
			return;
		}

		auto link = *it;
		
		Attribute* input = GetAttributeByID(link->input);
		Attribute* output = GetAttributeByID(link->output);
		
		input->links.erase(std::remove(input->links.begin(), input->links.end(), link->id), input->links.end());
		output->links.erase(std::remove(output->links.begin(), output->links.end(), link->id), output->links.end());

		mySpecification.links.erase(it);
	}

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
		dstGraph->myEntity = srcGraph->myEntity;
	}
}
