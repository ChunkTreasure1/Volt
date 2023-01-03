#include "vtpch.h"
#include "NavigationSystem.h"

#include "Volt/Asset/AssetManager.h"
#include "Volt/AI/SteeringBehavior.h"

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
		myAgentPositions.clear(); // This might not be needed.
		myCurrentScene->GetRegistry().ForEach<Volt::AgentComponent>([&](Wire::EntityId id, Volt::AgentComponent& agentComp)
			{
				myAgentPositions[id] = Entity(id, myCurrentScene.get()).GetPosition();
			});

		myCurrentScene->GetRegistry().ForEach<Volt::AgentComponent>([&](Wire::EntityId id, Volt::AgentComponent& agentComp)
			{
				UpdateAgent(id, agentComp, aTimestep);
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

	void NavigationSystem::UpdateAgent(const uint32_t& id, AgentComponent& comp, const float& ts)
	{
		if (!myNavMesh || !comp.myActive) return;

		Entity e(id, myCurrentScene.get());

		// #SAMUEL_TODO: Change this to use PathFollow seek behaviour later
		//if (myAgentTargets.find(id) == myAgentTargets.end()) { myAgentTargets[id] = comp.target; }
		//if (myAgentTargets.at(id) != comp.target)
		//{
		//	if (comp.myPath.empty() || gem::distance(comp.target, comp.myPath.front()) > 10.f) // Ignore small movements
		//	{
		//		auto path = myNavMesh->GetNavMeshData().findPath(VTtoPF(e.GetPosition()), VTtoPF(comp.target));
		//		if (!path.empty()) { comp.myPath.clear(); }
		//		for (const auto& p : path)
		//		{
		//			comp.myPath.emplace_back(PFtoVT(p));
		//		}
		//		comp.myPath.pop_back();
		//		myAgentTargets[id] = comp.target;
		//	}
		//}

		//if (comp.myPath.empty()) return;

		// Move to current milestone
		{
			auto currentPos = e.GetPosition();

			// Negative maxVelocity values will cause movement in y.
			if (comp.kinematic)
			{
				auto acceleration = comp.steeringForce;
				acceleration = gem::clamp(acceleration, gem::vec3(-1.f) * comp.maxVelocity, gem::vec3(1.f) * comp.maxVelocity);
				e.SetPosition(currentPos + acceleration * ts);
			}
			else
			{
				auto acceleration = comp.steeringForce; // Divide with mass if want to use later on.
				comp.myVelocity = comp.myVelocity + acceleration * ts;
				comp.myVelocity = gem::clamp(comp.myVelocity, gem::vec3(-1.f) * comp.maxVelocity, gem::vec3(1.f) * comp.maxVelocity);
				e.SetPosition(currentPos + comp.myVelocity * ts);
			}
		}
	}

}
