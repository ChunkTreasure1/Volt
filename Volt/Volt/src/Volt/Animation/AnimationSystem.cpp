#include "vtpch.h"
#include "AnimationSystem.h"

#include "Volt/Asset/AssetManager.h"
#include "Volt/Asset/Animation/AnimationGraphAsset.h"
#include "Volt/Animation/AnimationController.h"
#include "Volt/Animation/AnimationStateMachine.h"

#include "Volt/Components/RenderingComponents.h"

namespace Volt
{
	AnimationSystem::AnimationSystem(Scene* scene)
		: myScene(scene)
	{}

	void AnimationSystem::OnRuntimeStart(entt::registry& registry)
	{
		auto view = registry.view<AnimationControllerComponent>();
		view.each([&](const entt::entity id, AnimationControllerComponent& comp) 
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

	void AnimationSystem::OnRuntimeEnd(entt::registry&)
	{
	}

	void AnimationSystem::Update(entt::registry&, float)
	{
	}
}
