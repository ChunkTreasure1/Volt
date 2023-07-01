#include "vtpch.h"
#include "AnimationGraphAsset.h"

#include <GraphKey/Nodes/Animation/StateMachineNodes.h>

namespace Volt
{
	Ref<AnimationGraphAsset> AnimationGraphAsset::CreateCopy(Wire::EntityId entity)
	{
		Ref<AnimationGraphAsset> newGraph = CreateRef<AnimationGraphAsset>(mySkeletonHandle);
		newGraph->SetEntity(entity);
		GraphKey::Graph::Copy(shared_from_this(), newGraph);

		return newGraph;
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

