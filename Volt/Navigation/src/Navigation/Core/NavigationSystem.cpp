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
			VT_PROFILE_FUNCTION();
			if (myNavMesh && myActiveScene && myActiveScene->IsPlaying())
			{
				auto& crowd = myNavMesh->GetCrowd();
				if (!crowd)
				{
					return false;
				}

				auto& agentMap = crowd->GetAgentMap();
				{
					VT_PROFILE_SCOPE("Remove old agents");

					for (auto [entityId, agentId] : agentMap)
					{
						auto entity = myActiveScene->GetEntityFromUUID(entityId);

						if (!entity || !entity.HasComponent<Volt::NavAgentComponent>())
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

					auto view = registry.view<const Volt::NavAgentComponent>();
					view.each([&](const entt::entity id, const Volt::NavAgentComponent& comp)
					{
						auto entity = Volt::Entity(id, myActiveScene);

						crowd->SetAgentPosition(entity, entity.GetPosition());

						if (!crowd->GetAgentMap().contains(entity.GetID()))
						{
							crowd->AddAgent(entity);
							crowd->UpdateAgentParams(entity);
						}
						else if (!comp.active)
						{
							PauseAgent(entity, e.GetTimestep());
						}
						else if (comp.active)
						{
							UnpauseAgent(entity);
						}
					});
				}

				{
					VT_PROFILE_SCOPE("Detour crowd update");
					myNavMesh->Update(e.GetTimestep());
				}

				{
					VT_PROFILE_SCOPE("Update agent positions");
					for (auto [entityId, agentId] : agentMap)
					{
						Volt::Entity entity = myActiveScene->GetEntityFromUUID(entityId);
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
			const auto& metadata = AssetManager::GetMetadataFromHandle(myActiveScene->handle);

			if (!metadata.IsValid())
			{
				return false;
			}

			auto scenePath = metadata.filePath;
			if (std::filesystem::is_directory(scenePath))
			{
				scenePath /= (myActiveScene->assetName + ".vtasset");
			}

			scenePath.replace_extension(".vtnavmesh");

			SetVTNavMesh(Volt::AssetManager::GetAsset<Volt::AI::NavMesh>(scenePath));

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
			if (myEntityIdToTargetPosMap.contains(entity.GetID())) { return; }
			auto& crowd = myNavMesh->GetCrowd();

			myEntityIdToTargetPosMap[entity.GetID()] = *(glm::vec3*)&crowd->GetAgent(entity.GetID())->targetPos;
			crowd->ResetAgentTarget(entity);

			SyncDetourPosition(entity, deltaTime);
		}

		void NavigationSystem::UnpauseAgent(Volt::Entity entity)
		{
			if (!myEntityIdToTargetPosMap.contains(entity.GetID())) { return; }

			auto& crowd = myNavMesh->GetCrowd();
			crowd->SetAgentTarget(entity, myEntityIdToTargetPosMap.at(entity.GetID()));

			myEntityIdToTargetPosMap.erase(entity.GetID());
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
					VT_LOG(Error, "No valid physics scene found!");
				}
				else
				{
					auto actorController = physicsScene->GetControllerActor(entity);

					if (actorController)
					{
						//actorController->SetFootPosition(*(glm::vec3*)&agent->npos);
						actorController->Move(*(glm::vec3*)&agent->vel * deltaTime);
					}
					else
					{
						VT_LOG(Error, "No valid actor controller found for entity {0}!", entity.GetID());
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
				if (!myActiveScene)
				{
					return;
				}

				auto& crowd = myNavMesh->GetCrowd();
				if (!myActiveScene)
				{
					VT_LOG(Error, "Could not initialize agents because active scene is null");
					return;
				}

				auto& registry = myActiveScene->GetRegistry();
				auto view = registry.view<const Volt::NavAgentComponent>();
				view.each([&](const entt::entity id, const Volt::NavAgentComponent& comp)
				{
					Volt::Entity entity(id, myActiveScene);
					crowd->AddAgent(entity);
				});
			}
		}

		void NavigationSystem::ClearAgents()
		{
			if (myNavMesh)
			{
				auto& crowd = myNavMesh->GetCrowd();
				if (crowd)
				{
					crowd->ClearAgents();
				}
			}
		}
	}
}
