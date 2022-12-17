#include "gkpch.h"
#include "Node.h"

namespace GraphKey
{
	InputAttribute::InputAttribute(const std::string& name, bool linkable)
	{
		this->name = name;
		this->linkable = linkable;
	}

	OutputAttribute::OutputAttribute(const std::string& name, bool linkable)
	{
		this->name = name;
		this->linkable = linkable;
	}
	
	Node::Node(const Ref<Node> node)
	{
	}
}