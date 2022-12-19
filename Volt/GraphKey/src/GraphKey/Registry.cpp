#include "gkpch.h"
#include "Registry.h"

namespace GraphKey
{
	bool Registry::Register(const std::string& name, std::function<Ref<Node>()>&& createFunction)
	{
		if (myRegistry.contains(name))
		{
			return false;
		}

		myRegistry.emplace(name, createFunction);
		return true;
	}

	Ref<Node> Registry::Create(const std::string& name)
	{
		if (!myRegistry.contains(name))
		{
			return nullptr;
		}

		return myRegistry.at(name)();
	}
}