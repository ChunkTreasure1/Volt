#include "gkpch.h"
#include "Registry.h"

#include "Node.h"

namespace GraphKey
{
	bool Registry::Register(const std::string& name, const std::string& category, std::function<Ref<Node>()>&& createFunction)
	{
		if (myRegistry.contains(name))
		{
			return false;
		}

		myCategories.emplace(name, category);
		myRegistry.emplace(name, createFunction);
		return true;
	}

	Ref<Node> Registry::Create(const std::string& name)
	{	
		if (!myRegistry.contains(name))
		{
			return nullptr;
		}

		auto node = myRegistry.at(name)();
		node->myRegistryName = name;
		return node;
	}
	const std::string& Registry::GetCategory(const std::string& name)
	{
		VT_CORE_ASSERT(myCategories.contains(name), "Node not registered");
		return myCategories.at(name);
	}
}