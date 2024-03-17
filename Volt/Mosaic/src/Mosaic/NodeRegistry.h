#pragma once


#include <CoreUtilities/Core.h>
#include <CoreUtilities/VoltGUID.h>

#include <unordered_map>
#include <functional>

#define UNPACK(...) __VA_ARGS__
#define REGISTER_NODE(nodeType) inline static bool nodeType ## _node_registered = Mosaic::NodeRegistry::RegisterNode<nodeType>()
#define REGISTER_NODE_TEMPLATE(varName, nodeType) inline static bool varName ## _node_registered = Mosaic::NodeRegistry::RegisterNode<UNPACK nodeType>()

namespace Mosaic
{
	class MosaicNode;
	class MosaicGraph;

	struct NodeInfo
	{
		std::function<Ref<MosaicNode>(MosaicGraph* ownerGraph)> createFunction;
		std::string name;
		std::string category;
	};

	class NodeRegistry
	{
	public:
		template<typename T>
		inline static const bool RegisterNode()
		{
			// Instantiate node to get GUID
			Ref<T> tempNode = CreateRef<T>(nullptr);
			const VoltGUID guid = tempNode->GetGUID();

			if (s_registry.contains(guid))
			{	
				return false;
			}

			NodeInfo nodeInfo{};
			nodeInfo.name = tempNode->GetName();
			nodeInfo.category = tempNode->GetCategory();
			nodeInfo.createFunction = [](MosaicGraph* ownerGraph)
			{
				return CreateRef<T>(ownerGraph);
			};

			s_registry[guid] = nodeInfo;
			return true;
		}

		inline static Ref<MosaicNode> CreateNode(const VoltGUID guid, MosaicGraph* ownerGraph)
		{
			if (!s_registry.contains(guid))
			{
				return nullptr;
			}

			return s_registry.at(guid).createFunction(ownerGraph);
		}

		inline static const auto& GetRegistry() { return s_registry; }
		inline static const auto& GetNodeInfo(const VoltGUID guid) { return s_registry.at(guid); }

	private:
		inline static std::unordered_map<VoltGUID, NodeInfo> s_registry;
	};
}
