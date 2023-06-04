#include "vtpch.h"
#include "AnimationGraphAsset.h"

#include <GraphKey/Nodes/Animation/StateMachineNodes.h>

namespace Volt
{
	Ref<AnimationGraphAsset> AnimationGraphAsset::CreateCopy(Wire::EntityId entity)
	{
		Ref<AnimationGraphAsset> newGraph = CreateRef<AnimationGraphAsset>(myAnimatedCharacter);
		newGraph->SetEntity(entity);
		GraphKey::Graph::Copy(shared_from_this(), newGraph);

		return newGraph;
	}

	void AnimationGraphAsset::SetCharacterHandle(AssetHandle handle)
	{
		myAnimatedCharacter = handle;
		auto stateMachines = GetNodesOfType("StateMachineNode");
		for (const auto& node : stateMachines)
		{
			auto stateNodeType = std::reinterpret_pointer_cast<GraphKey::StateMachineNode>(node);
			stateNodeType->GetStateMachine()->SetCharacterHandle(handle);
		}
	}
}

