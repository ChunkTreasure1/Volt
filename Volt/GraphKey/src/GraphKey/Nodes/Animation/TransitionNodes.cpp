#include "gkpch.h"
#include "TransitionNodes.h"

namespace GraphKey
{
	TransitionOutputNode::TransitionOutputNode()
	{
		inputs =
		{
			AttributeConfigDefault("Can Enter Transition", AttributeDirection::Input, false)
		};
	}

	const bool TransitionOutputNode::Evaluate()
	{
		const bool result = GetInput<bool>(0);
		return result;
	}
}

