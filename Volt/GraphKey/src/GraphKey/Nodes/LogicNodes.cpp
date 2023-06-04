#include "gkpch.h"
#include "LogicNodes.h"

namespace GraphKey
{
	BranchNode::BranchNode()
	{
		inputs =
		{
			AttributeConfig("", AttributeDirection::Input, GK_BIND_FUNCTION(BranchNode::Branch)),
			AttributeConfig<bool>("Condition", AttributeDirection::Input, false)
		};

		outputs =
		{
			AttributeConfig("True", AttributeDirection::Output),
			AttributeConfig("False", AttributeDirection::Output)
		};
	}

	void BranchNode::Branch()
	{
		const auto cond = GetInput<bool>(1);
		if (cond)
		{
			ActivateOutput(0);
		}
		else
		{
			ActivateOutput(1);
		}
	}

	DoOnceNode::DoOnceNode()
	{
		inputs =
		{
			AttributeConfig("", AttributeDirection::Input, GK_BIND_FUNCTION(DoOnceNode::Do)),
		};

		outputs =
		{
			AttributeConfig("", AttributeDirection::Output),
		};
	}

	void DoOnceNode::Do()
	{
		if (!myHasDone)
		{
			myHasDone = true;
			ActivateOutput(0);
		}
	}
}
