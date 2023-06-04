#include "gkpch.h"
#include "AnimationSwitchNode.h"

#include "GraphKey/Nodes/Animation/BaseAnimationNodes.h"

namespace GraphKey
{
	AnimationSwitchNode::AnimationSwitchNode()
	{
		inputs =
		{
			AttributeConfigDefault("Value", AttributeDirection::Input, int32_t(0)),
			AttributeConfig("0", AttributeDirection::Input),
			AttributeConfig("1", AttributeDirection::Input),
			AttributeConfig("2", AttributeDirection::Input),
			AttributeConfig("3", AttributeDirection::Input),
			AttributeConfig("4", AttributeDirection::Input),
			AttributeConfig("5", AttributeDirection::Input),
			AttributeConfig("6", AttributeDirection::Input),
			AttributeConfig("7", AttributeDirection::Input),
			AttributeConfig("8", AttributeDirection::Input),
			AttributeConfig("9", AttributeDirection::Input),
			AttributeConfig("10", AttributeDirection::Input)
		};

		outputs =
		{
			AttributeConfig("Output", AttributeDirection::Output, GK_BIND_FUNCTION(AnimationSwitchNode::GetSwitchedAnimation))
		};
	}

	void AnimationSwitchNode::GetSwitchedAnimation()
	{
		const int32_t inputIndex = GetInput<int32_t>(0);
		switch (inputIndex)
		{
			case 0:
				SetOutputData(0, GetInput<AnimationOutputData>(1));
				break;

			case 1:
				SetOutputData(0, GetInput<AnimationOutputData>(2));
				break;

			case 2:
				SetOutputData(0, GetInput<AnimationOutputData>(3));
				break;

			case 3:
				SetOutputData(0, GetInput<AnimationOutputData>(4));
				break;

			case 4:
				SetOutputData(0, GetInput<AnimationOutputData>(5));
				break;

			case 5:
				SetOutputData(0, GetInput<AnimationOutputData>(6));
				break;

			case 6:
				SetOutputData(0, GetInput<AnimationOutputData>(7));
				break;

			case 7:
				SetOutputData(0, GetInput<AnimationOutputData>(8));
				break;

			case 8:
				SetOutputData(0, GetInput<AnimationOutputData>(9));
				break;

			case 9:
				SetOutputData(0, GetInput<AnimationOutputData>(10));
				break;

			case 10:
				SetOutputData(0, GetInput<AnimationOutputData>(11));
				break;
		}
	}
}
