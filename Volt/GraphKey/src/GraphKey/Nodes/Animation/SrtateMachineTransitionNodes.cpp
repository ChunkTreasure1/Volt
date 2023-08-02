#include "gkpch.h"
#include "SrtateMachineTransitionNodes.h"

#include <Volt/Animation/AnimationTransitionGraph.h>
#include <Volt/Animation/AnimationStateMachine.h>

#include <Volt/Asset/Animation/AnimationGraphAsset.h>
#include <Volt/Asset/Animation/Animation.h>
#include <Volt/Asset/AssetManager.h>



GraphKey::GetRelevantAnimationLengthNode::GetRelevantAnimationLengthNode()
{
	outputs =
	{
		AttributeConfigDefault("Return Value", AttributeDirection::Output, 0.f, false, GK_BIND_FUNCTION(GetRelevantAnimationLength))
	};
}

void GraphKey::GetRelevantAnimationLengthNode::GetRelevantAnimationLength()
{
	auto animTransitionGraph = static_cast<Volt::AnimationTransitionGraph*>(myGraph);
	auto stateMachine = animTransitionGraph->GetStateMachine();
	auto transition = stateMachine->GetTransitionById(animTransitionGraph->GetTransitionID());
	auto fromState = stateMachine->GetStateById(transition->fromState);
	Volt::AssetHandle relevantAnimationHandle = fromState->stateGraph->GetRelevantAnimationHandle();
	
	auto relevantAnimation = Volt::AssetManager::GetAsset<Volt::Animation>(relevantAnimationHandle);
	
	float length =  relevantAnimation->GetDuration();
	SetOutputData(0, length);
}
