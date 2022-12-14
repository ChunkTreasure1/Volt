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
		myCurrentScene->GetRegistry().ForEach<Volt::AgentComponent>([&](Wire::EntityId id, Volt::AgentComponent& agentComp)
			{
				//agentComp.agent.Update(aTimestep, Entity(id, myCurrentScene.get()));
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

		// #SAMUEL_TODO: Somehow make this only happen when new target is set
		{
			if (comp.myPath.empty() || gem::distance(comp.target, comp.myPath.front()) > 10.f) // Ignore small movements
			{
				auto path = myNavMesh->GetNavMeshData().findPath(VTtoPF(e.GetWorldPosition()), VTtoPF(comp.target));
				if (!path.empty()) { comp.myPath.clear(); }
				for (const auto& p : path)
				{
					comp.myPath.emplace_back(PFtoVT(p));
				}
				comp.myPath.pop_back();
			}
		}

		if (comp.myPath.empty()) return;

		// Move to current milestone
		{
			auto currentPos = e.GetWorldPosition();
			auto steeringForce = GetSteeringForce(e);

			if (comp.kinematic)
			{
				comp.myVelocity = gem::vec3(0.f);
				e.SetWorldPosition(currentPos + steeringForce * ts);
			}
			else
			{
				auto acceleration = steeringForce;
				comp.myVelocity = comp.myVelocity + acceleration * ts;
				comp.myVelocity = gem::clamp(comp.myVelocity, gem::vec3(-1.f) * comp.maxVelocity, gem::vec3(1.f) * comp.maxVelocity);

				e.SetWorldPosition(currentPos + comp.myVelocity * ts);
			}

			// #SAMUEL_TODO: this needs to be better implemented
			if (auto milestone = comp.GetCurrentMilestone(); milestone.has_value())
			{
				float dist = gem::distance(currentPos, milestone.value());
				if (dist < 10.f) // continue to next milestone if close enough to current milestone
				{
					comp.myPath.pop_back();
				}
			}
		}
	}

	gem::vec3 NavigationSystem::GetSteeringForce(const Entity& aEntity) const
	{
		auto& comp = aEntity.GetComponent<AgentComponent>();

		if (auto milestone = comp.GetCurrentMilestone(); milestone.has_value())
		{
			switch (comp.steering)
			{
			case AgentSteeringBehavior::Seek:
				return SteeringBehavior::Seek(aEntity, milestone.value());

			case AgentSteeringBehavior::Flee:
				return SteeringBehavior::Flee(aEntity, milestone.value());

			case AgentSteeringBehavior::Arrive:
				return SteeringBehavior::Arrive(aEntity, milestone.value());

			case AgentSteeringBehavior::Align:
				return SteeringBehavior::Align(aEntity, milestone.value());

			case AgentSteeringBehavior::Pursue:
				return SteeringBehavior::Pursue(aEntity, milestone.value());

			case AgentSteeringBehavior::Evade:
				return SteeringBehavior::Evade(aEntity, milestone.value());

			case AgentSteeringBehavior::Wander:
				return SteeringBehavior::Wander(aEntity, milestone.value());

			case AgentSteeringBehavior::PathFollowing:
				return SteeringBehavior::PathFollowing(aEntity, milestone.value());

			case AgentSteeringBehavior::Separation:
				return SteeringBehavior::Separation(aEntity, milestone.value());

			case AgentSteeringBehavior::CollisionAvoidance:
				return SteeringBehavior::CollisionAvoidance(aEntity, milestone.value());

			default:
				return SteeringBehavior::Seek(aEntity, milestone.value());
			}
		}
	}

}
