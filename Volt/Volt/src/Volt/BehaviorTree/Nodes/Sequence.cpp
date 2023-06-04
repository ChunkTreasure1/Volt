#include "vtpch.h"
#include "BehaviorTreeNode.h"
#include "Volt/BehaviorTree/BehaviorTree.hpp"

namespace Volt::BehaviorTree
{
	eNodeStatus Sequence::Run()
	{
		for (auto child : m_tree->GetNodeManager().GetLinksFromUUID(m_uuid))
		{
			auto ret = m_tree->GetNodeManager().GetNodeFromUUID(child.m_childID)->Run();
			if (ret == eNodeStatus::FAILURE)
			{
				return ret;
			}
		}
		return eNodeStatus::SUCCESS;
	}
}
