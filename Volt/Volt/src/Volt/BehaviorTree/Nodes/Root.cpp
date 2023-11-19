#include "vtpch.h"
#include "BehaviorTreeNode.h"
#include "../BehaviorTree.hpp"

namespace Volt::BehaviorTree
{
	eNodeStatus Root::Run()
	{
		return m_tree->GetNodeManager().GetNodeFromUUID(m_tree->GetNodeManager().GetLinksFromUUID(m_uuid)[0].m_childID)->Run();
	}


	void Node::SetUUID(const UUID64& in_id)
	{
		Ref<Node> this_node = m_tree->GetNodeManager().m_nodes[m_uuid];
		m_tree->GetNodeManager().m_nodes.erase(m_tree->GetNodeManager().m_nodes.find(m_uuid));
		m_uuid = in_id;
		m_tree->GetNodeManager().RegisterNode(m_uuid, this_node);
	}

}
