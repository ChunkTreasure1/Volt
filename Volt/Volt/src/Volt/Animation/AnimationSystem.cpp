#include "vtpch.h"
#include "AnimationSystem.h"

#include "Volt/Asset/AssetManager.h"
#include "Volt/Asset/Animation/AnimationGraphAsset.h"
#include "Volt/Animation/AnimationController.h"
#include "Volt/Animation/AnimationStateMachine.h"

#include "Volt/Components/Components.h"

namespace Volt
{
	AnimationSystem::AnimationSystem(Scene* scene)
		: myScene(scene)
	{}

	void AnimationSystem::OnRuntimeStart(Wire::Registry& registry)
	{
		registry.ForEach<AnimationControllerComponent>([=](Wire::EntityId id, AnimationControllerComponent& comp)
		{
			if (comp.animationGraph != Asset::Null())
			{
				auto graph = AssetManager::GetAsset<AnimationGraphAsset>(comp.animationGraph);
				if (graph && graph->IsValid())
				{
					comp.controller = CreateRef<AnimationController>(graph, Volt::Entity{ id, myScene });
				}
			}
		});
	}

	void AnimationSystem::OnRuntimeEnd(Wire::Registry& registry)
	{
	}

	void AnimationSystem::Update(Wire::Registry& registry, float deltaTime)
	{
		//myRegistry.ForEach<AnimationControllerComponent>([](Wire::EntityId, AnimationControllerComponent& comp)
		//	{
		//	});
	}
}
