#include "gkpch.h"
#include "StateMachineTransitionNodes.h"

#include <Volt/Animation/AnimationTransitionGraph.h>
#include <Volt/Animation/AnimationStateMachine.h>

#include <Volt/Asset/Animation/AnimationGraphAsset.h>
#include <Volt/Asset/Animation/Animation.h>
#include <Volt/Asset/AssetManager.h>

#include "GraphKey/Nodes/Animation/SequenceNodes.h"

namespace GraphKey
{
	Ref<Node> GetRelevantAnimationNode(GraphKey::Graph* aGraph)
	{
		const auto animTransitionGraph = static_cast<Volt::AnimationTransitionGraph*>(aGraph);
		const auto stateMachine = animTransitionGraph->GetStateMachine();
		const auto transition = stateMachine->GetTransitionById(animTransitionGraph->GetTransitionID());
		const auto fromState = stateMachine->GetStateById(transition->fromState);

		return fromState->stateGraph->GetRelevantAnimationNode();
	}

	GraphKey::GetRelevantAnimationLengthNode::GetRelevantAnimationLengthNode()
	{
		outputs =
		{
			AttributeConfig<float>("Return Value", AttributeDirection::Output, true, GK_BIND_FUNCTION(GetRelevantAnimationLengthNode::GetRelevantAnimationLength))
		};
	}

	void GraphKey::GetRelevantAnimationLengthNode::GetRelevantAnimationLength()
	{
		const auto relevantAnimNode = GetRelevantAnimationNode(myGraph);

		if (!relevantAnimNode)
			return;

		float duration = 0.f;
		if (relevantAnimNode->GetName() == "Sequence Player")
		{
			auto anim = Volt::AssetManager::GetAsset<Volt::Animation>(relevantAnimNode->GetInput<Volt::AssetHandle>(0));
			if (anim)
			{
				duration = anim->GetDuration();
			}
		}

		SetOutputData<float>(0, duration);
	}

	GraphKey::GetRelevantAnimationTimeNode::GetRelevantAnimationTimeNode()
	{
		outputs =
		{
			AttributeConfig<float>("Return Value", AttributeDirection::Output, true, GK_BIND_FUNCTION(GetRelevantAnimationTimeNode::GetRelevantAnimationTime))
		};
	}

	void GraphKey::GetRelevantAnimationTimeNode::GetRelevantAnimationTime()
	{
		const auto relevantAnimNode = GetRelevantAnimationNode(myGraph);

		if (!relevantAnimNode)
			return;

		float currentTime = 0.f;

		if (relevantAnimNode->GetName() == "Sequence Player")
		{
			auto sequencePlayer = std::reinterpret_pointer_cast<GraphKey::SequencePlayerNode>(relevantAnimNode);
			currentTime = sequencePlayer->GetCurrentAnimationTime();
		}

		SetOutputData<float>(0, currentTime);
	}

	GetRelevantAnimationTimeNormalizedNode::GetRelevantAnimationTimeNormalizedNode()
	{
		outputs =
		{
			AttributeConfig<float>("Return Value", AttributeDirection::Output, true, GK_BIND_FUNCTION(GetRelevantAnimationTimeNormalizedNode::GetRelevantAnimationTimeNormalized))
		};
	}

	void GetRelevantAnimationTimeNormalizedNode::GetRelevantAnimationTimeNormalized()
	{
		const auto relevantAnimNode = GetRelevantAnimationNode(myGraph);

		if (!relevantAnimNode)
			return;

		float currentTime = 0.f;

		if (relevantAnimNode->GetName() == "Sequence Player")
		{
			auto sequencePlayer = std::reinterpret_pointer_cast<GraphKey::SequencePlayerNode>(relevantAnimNode);
			currentTime = sequencePlayer->GetCurrentAnimationTimeNormalized();
		}
		
		SetOutputData<float>(0, currentTime);
	}

}
