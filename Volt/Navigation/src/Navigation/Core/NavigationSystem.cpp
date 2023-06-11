#include "nvpch.h"
#include "NavigationSystem.h"

#include "NavigationEditor/Tools/NavMeshDebugDrawer.h"

#include <Volt/Asset/AssetManager.h>
#include <Volt/Scene/Entity.h>
#include <Volt/Scene/Scene.h>

#include <Volt/Physics/Physics.h>
#include <Volt/Physics/PhysicsScene.h>
#include <Volt/Physics/PhysicsActor.h>
#include <Volt/Physics/PhysicsControllerActor.h>
#include <Volt/Components/PhysicsComponents.h>

namespace Volt
{
	namespace AI
	{
		NavigationSystem::NavigationSystem() 
		{
			myNavMesh = CreateRef<NavMesh>();
		}

		void NavigationSystem::OnEvent(Event& event)
		{
			EventDispatcher dispatcher(event);
			dispatcher.Dispatch<Volt::AppUpdateEvent>(VT_BIND_EVENT_FN(NavigationSystem::OnAppUpdateEvent));
			dispatcher.Dispatch<Volt::OnSceneLoadedEvent>(VT_BIND_EVENT_FN(NavigationSystem::OnSceneLoadedEvent));
		}

		void NavigationSystem::SetVTNavMesh(Ref<NavMesh> navmesh)
		{
			myNavMesh = navmesh;
			NavMeshDebugDrawer::CompileDebugMesh(myNavMesh.get());
		}

		bool NavigationSystem::OnAppUpdateEvent(Volt::AppUpdateEvent& e)
		{
			VT_PROFILE_FUNCTION()
			if (myNavMesh && myActiveScene->IsPlaying())
			{
				auto& crowd = myNavMesh->GetCrowd();

				auto& agentMap = crowd->GetAgentMap();
				{
					VT_PROFILE_SCOPE("Remove old agents");

					for (auto [entityId, agentId] : agentMap)
					{
						const auto& registry = myActiveScene->GetRegistry();
						auto entity = Volt::Entity(entityId, myActiveScene.get());

						if (!registry.Exists(entityId) || !registry.HasComponent<Volt::NavAgentComponent>(entityId))
						{
							crowd->RemoveAgent(entity);
						}

						if (myEntityIdToTargetPosMap.contains(entityId))
						{
							myEntityIdToTargetPosMap.erase(entityId);
						}
					}
				}

				{
					VT_PROFILE_SCOPE("Add new agents & disable movement on inactive agents");

					auto& registry = myActiveScene->GetRegistry();

					auto agentEntities = registry.GetComponentView<Volt::NavAgentComponent>();
					for (auto entityId : agentEntities)
					{
						auto entity = Volt::Entity(entityId, myActiveScene.get());

						if (!crowd->GetAgentMap().contains(entityId))
						{
							crowd->AddAgent(entity);
							crowd->UpdateAgentParams(entity);
						}
						else if (!registry.GetComponent<Volt::NavAgentComponent>(entityId).active)
						{
							PauseAgent(entity, e.GetTimestep());
						}
						else if (registry.GetComponent<Volt::NavAgentComponent>(entityId).active)
						{
							UnpauseAgent(entity);
						}
					}
				}

				{
					VT_PROFILE_SCOPE("Detour crowd update");
					myNavMesh->Update(e.GetTimestep());
				}

				{
					VT_PROFILE_SCOPE("Update agent positions");
					for (auto [entityId, agentId] : agentMap)
					{
						Volt::Entity entity(entityId, myActiveScene.get());
						if (entity.GetComponent<NavAgentComponent>().active)
						{
							SyncDetourPosition(entity, e.GetTimestep());
						}
					}
				}
			}

			return false;
		}

		bool NavigationSystem::OnSceneLoadedEvent(Volt::OnSceneLoadedEvent& e)
		{
			myActiveScene = e.GetScene();
			if (myActiveScene->path.empty() && myActiveScene->GetName() != "New Scene") { return false; }

			auto scenePath = myActiveScene->path;
			scenePath.replace_extension(".vtnavmesh");

			SetVTNavMesh(Volt::AssetManager::Get().GetAsset<Volt::AI::NavMesh>(scenePath));

			if (myActiveScene->IsPlaying())
			{
				ClearAgents();
				InitAgents();
			}
			return false;
		}

		bool NavigationSystem::OnRuntimeStart()
		{
			if (myNavMesh)
			{
				ClearAgents();
				InitAgents();
			}

			return false;
		}

		void NavigationSystem::PauseAgent(Volt::Entity entity, float deltaTime)
		{
			if (myEntityIdToTargetPosMap.contains(entity.GetId())) { return; }
			auto& crowd = myNavMesh->GetCrowd();

			myEntityIdToTargetPosMap[entity.GetId()] = *(glm::vec3*)&crowd->GetAgent(entity.GetId())->targetPos;
			crowd->ResetAgentTarget(entity);

			SyncDetourPosition(entity, deltaTime);
		}

		void NavigationSystem::UnpauseAgent(Volt::Entity entity)
		{
			if (!myEntityIdToTargetPosMap.contains(entity.GetId())) { return; }

			auto& crowd = myNavMesh->GetCrowd();
			crowd->SetAgentTarget(entity, myEntityIdToTargetPosMap.at(entity.GetId()));

			myEntityIdToTargetPosMap.erase(entity.GetId());
		}

		void NavigationSystem::SyncDetourPosition(Volt::Entity entity, float deltaTime)
		{
			auto& crowd = myNavMesh->GetCrowd();
			auto agent = crowd->GetAgent(entity);

			if (entity.HasComponent<Volt::CharacterControllerComponent>())
			{
				auto physicsScene = Physics::GetScene();

				if (!physicsScene)
				{
					VT_CORE_ERROR("No valid physics scene found!");
				}
				else
				{
					auto actorController = physicsScene->GetControllerActor(entity);

					if (actorController)
					{
						actorController->SetFootPosition(*(glm::vec3*)&agent->npos);
						//actorController->Move(*(glm::vec3*)&agent->vel * deltaTime);
					}
					else
					{
						VT_CORE_ERROR("No valid actor controller found for entity {0}!", entity.GetId());
					}
				}
			}
			else
			{
				entity.SetPosition(*(glm::vec3*)&agent->npos);
			}
		}

		void NavigationSystem::InitAgents()
		{
			if (myNavMesh)
			{
				auto& crowd = myNavMesh->GetCrowd();

				auto agentEntities = myActiveScene->GetRegistry().GetComponentView<Volt::NavAgentComponent>();
				for (auto entityId : agentEntities)
				{
					Volt::Entity entity(entityId, myActiveScene.get());
					crowd->AddAgent(entity);
				}
			}
		}

		void NavigationSystem::ClearAgents()
		{
			if (myNavMesh)
			{
				auto& crowd = myNavMesh->GetCrowd();
				crowd->ClearAgents();
			}
		}
	}
}
