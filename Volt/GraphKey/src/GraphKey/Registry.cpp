#include "gkpch.h"
#include "Registry.h"

#include "Node.h"

namespace GraphKey
{
	bool Registry::Register(const std::string& name, const std::string& category, GraphType editorType, bool visible, std::function<Ref<Node>()>&& createFunction)
	{
		if (myRegistry.contains(name))
		{
			return false;
		}

		RegistryEntry entry{};
		entry.category = category;
		entry.createFunc = createFunction;
		entry.graphType = editorType;
		entry.name = name;
		entry.visible = visible;

		myCategories.emplace(name, category);
		myRegistry.emplace(name, entry);
		return true;
	}

	Ref<Node> Registry::Create(const std::string& name)
	{
		if (!myRegistry.contains(name))
		{
			return nullptr;
		}

		auto node = myRegistry.at(name).createFunc();
		node->myRegistryName = name;
		return node;
	}

	const std::string& Registry::GetCategory(const std::string& name)
	{
		VT_ASSERT_MSG(myCategories.contains(name), "Node not registered");
		return myCategories.at(name);
	}

	const Vector<RegistryEntry> Registry::GetNodesOfGraphType(GraphType type)
	{
		Vector<RegistryEntry> result;

		for (const auto& [name, data] : myRegistry)
		{
			if (data.graphType == type || data.graphType == GraphType::All)
			{
				result.emplace_back(data);
			}
		}

		return result;
	}
}
