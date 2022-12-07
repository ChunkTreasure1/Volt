#include "vtpch.h"
#include "NavigationsSystem.h"

#include "Volt/Utility/FileSystem.h"
#include "Volt/Scene/Scene.h"
#include "Volt/Asset/AssetManager.h"
#include "Volt/Components/NavigationComponents.h"
#include "Volt/Components/PhysicsComponents.h"
#include "Volt/AI/NavMesh/NavMeshGenerator.h"

#include "Volt/AI/NavMesh2/NavMesh2.h"

namespace Volt
{
	NavigationsSystem::NavigationsSystem(Ref<Scene>& aScene)
		: myCurrentScene(aScene)
	{
		if (!myInstance)
		{
			myInstance = this;
		}
	}

	NavigationsSystem::~NavigationsSystem()
	{

	}

	void NavigationsSystem::SetNavMesh(Ref<Mesh> aNavMesh)
	{
		if (aNavMesh)
		{
			std::vector<NavMeshVertex> nvVertices;

			for (const auto& v : aNavMesh->GetVertices())
			{
				NavMeshVertex newV;
				newV.Position = v.position;
				nvVertices.push_back(newV);
			}

			myNavMeshData = CreateRef<NavMesh>(nvVertices, aNavMesh->GetIndices());
			myNavMeshData->Cells = NavMeshGenerator::GenerateNavMeshCells(myNavMeshData);
		}
	}

	const Ref<Volt::Mesh> NavigationsSystem::GetNavMesh()
	{
		if (!myNavMeshEntity.IsNull() && myNavMeshEntity.HasComponent<NavMeshComponent>())
		{
			auto handle = myNavMeshEntity.GetComponent<NavMeshComponent>().handle;
			return AssetManager::GetAsset<Volt::Mesh>(handle);
		}
		return Ref<Volt::Mesh>();
	}

	const Ref<NavMesh> NavigationsSystem::GetNavMeshData() const
	{
		return myNavMeshData;
	}

	const Ref<NavMesh2> NavigationsSystem::GetNavMesh2() const
	{
		return myNavMesh2;
	}

	void NavigationsSystem::OnRuntimeUpdate(float aTimestep)
	{
		VT_PROFILE_FUNCTION();

		myCurrentScene->GetRegistry().ForEach<NavMeshAgentComponent>([&](Wire::EntityId id, NavMeshAgentComponent& agentComp)
			{
				agentComp.agent.Update(aTimestep, Entity(id, myCurrentScene.get()));
			});
	}

	void NavigationsSystem::Draw()
	{
		if (myNavMesh2)
		{
			myNavMesh2->Draw();
		}
	}

	void NavigationsSystem::OnSceneLoad()
	{
		if (myCurrentScene)
		{
			myNavMeshEntity = Entity(Wire::NullID, nullptr);
			myNavMeshData = nullptr;

			myCurrentScene->GetRegistry().ForEach<NavMeshComponent>([&](Wire::EntityId id, NavMeshComponent&)
				{
					myNavMeshEntity = Entity(id, myCurrentScene.get());
				});

			if (!myNavMeshEntity.IsNull())
			{
				auto asset = AssetManager::GetAsset<Mesh>(myNavMeshEntity.GetComponent<NavMeshComponent>().handle);

				if (asset)
				{
					/*std::vector<NavMeshVertex> nvVertices;

					for (const auto& v : asset->GetVertices())
					{
						NavMeshVertex newV;
						newV.Position = v.position;
						nvVertices.push_back(newV);
					}

					myNavMeshData = CreateRef<NavMesh>(nvVertices, asset->GetIndices());
					myNavMeshData->Cells = NavMeshGenerator::GenerateNavMeshCells(myNavMeshData);*/

					myNavMesh2 = CreateRef<NavMesh2>(asset);
				}
			}
		}
	}

	std::vector<Volt::Entity> NavigationsSystem::GetBridges()
	{
		std::vector<Volt::Entity> result;

		myCurrentScene->GetRegistry().ForEach<Volt::NavMeshBlockComponent>([&](Wire::EntityId id, Volt::NavMeshBlockComponent&)
			{
				result.emplace_back(Entity(id, myCurrentScene.get()));
			});

		return result;
	}
}
