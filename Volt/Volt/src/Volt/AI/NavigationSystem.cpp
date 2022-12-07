#include "vtpch.h"
#include "NavigationSystem.h"

#include "Volt/Asset/AssetManager.h"

#include "Volt/Components/NavigationComponents.h"
#include "Volt/Scene/Scene.h"
#include "Volt/Log/Log.h"

namespace Volt
{
	NavigationSystem::NavigationSystem(Ref<Scene>& aScene)
		: myCurrentScene(aScene)
	{
		if (!myInstance)
		{
			myInstance = this;
		}
	}

	void NavigationSystem::OnRuntimeUpdate(float aTimestep)
	{
		myCurrentScene->GetRegistry().ForEach<Volt::AgentComponent>([&](Wire::EntityId id, Volt::AgentComponent& agentComp)
			{
				agentComp.agent.Update(aTimestep, Entity(id, myCurrentScene.get()));
			});
	}

	void NavigationSystem::OnSceneLoad(bool isRuntime)
	{
		if (!isRuntime)
		{
			myCurrentScenePath = myCurrentScene->path;
			myCurrentScenePath.remove_filename();
			myCurrentScenePath.append("NavMesh.vtnavmesh");
		}

		myNavMesh = AssetManager::GetAsset<NavMesh>(myCurrentScenePath);
	}
}
