#pragma once
#include "Volt/Core/Base.h"

#include "Structs.h"
#include "Nodes/BehaviorTreeNode.h"

#include <unordered_map>

namespace Volt::BehaviorTree
{
	// #MAX_TODO: need to set child when linking nodes

	class NodeManager
	{
	public:
		Ref<Node> GetNodeFromUUID(const UUID64& in_uuid) const;
		const Vector<Link>& GetLinksFromUUID(const UUID64& in_uuid);
		const Link& GetLink(const UUID64& in_uuid);

		std::unordered_map<UUID64, Ref<Node>>& GetNodes()  { return m_nodes; };

		const std::unordered_map<UUID64, Link>& GetLinkIDs() const { return m_linkIDs; };
		std::unordered_map<UUID64, Vector<Link>>& GetLinksNoConst() { return m_links; };
		const std::unordered_map<UUID64, Vector<Link>>& GetLinks() const { return m_links; };

		bool RegisterNode(const UUID64& in_uuid, Ref<Node> in_node);
		bool RegisterLink(const UUID64& in_parentID, const UUID64& in_childID, const UUID64& in_linkID = 0);

		void UnregisterNode(const UUID64& in_uuid);
		void UnregisterLink(const Link& in_link);
		void UnregisterLink(const UUID64& in_linkID);

	private:
		friend class Tree;
		friend class BehaviorTreeImporter;
		friend class BehaviourTreeSerializer;
		friend class Node;
		NodeManager();
		~NodeManager();

		std::unordered_map<UUID64, Ref<Node>> m_nodes;

		std::unordered_map<UUID64, Link> m_linkIDs;
		// <ParentUUiD, Link vec>
		std::unordered_map<UUID64, Vector<Link>> m_links;
	};

}
