#include "vtpch.h"
#include "AnimationGraphAsset.h"

#include <GraphKey/Nodes/Animation/StateMachineNodes.h>
#include <GraphKey/Nodes/Animation/SequenceNodes.h>

namespace Volt
{
	Ref<AnimationGraphAsset> AnimationGraphAsset::CreateCopy(entt::entity entity)
	{
		Ref<AnimationGraphAsset> newGraph = CreateRef<AnimationGraphAsset>(mySkeletonHandle);
		newGraph->SetEntity(entity);
		GraphKey::Graph::Copy(shared_from_this(), newGraph);

		return newGraph;
	}
	
	Ref<GraphKey::Node> AnimationGraphAsset::GetRelevantAnimationNode()
	{
		auto sequencePlayerNodes = GetNodesOfType("SequencePlayerNode");

		if (sequencePlayerNodes.size() == 0)
			return Ref<GraphKey::Node>();

		//TODO: Update this to check for all the relevant nodes
		//TODO: Update this to have a relevancy order 
		auto sequencePlayerNode = std::dynamic_pointer_cast<GraphKey::SequencePlayerNode>(sequencePlayerNodes[0]);

		return sequencePlayerNode;
	}

	void AnimationGraphAsset::SetSkeletonHandle(AssetHandle aSkeletonHandle)
	{
		mySkeletonHandle = aSkeletonHandle;
		
		auto stateMachines = GetNodesOfType("StateMachineNode");
		for (const auto& node : stateMachines)
		{
			auto stateNodeType = std::reinterpret_pointer_cast<GraphKey::StateMachineNode>(node);
			
			stateNodeType->GetStateMachine()->SetSkeletonHandle(aSkeletonHandle);
		}
	}
}

