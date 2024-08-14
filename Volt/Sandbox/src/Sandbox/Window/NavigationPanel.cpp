#include "sbpch.h"
#include "NavigationPanel.h"
#include "NavigationEditor/Tools/NavMeshDebugDrawer.h"

#include <Volt/Asset/AssetManager.h>
#include <Volt/Utility/MeshExporterUtilities.h>

#include <Volt/Components/PhysicsComponents.h>
#include <Volt/Components/NavigationComponents.h>

#include <Volt/Rendering/DebugRenderer.h>
#include <Volt/Physics/MeshColliderCache.h>

#include <Sandbox/Utility/EditorUtilities.h>
#include <Sandbox/UserSettingsManager.h>

#include <Volt/Discord/DiscordSDK.h>

void NavigationPanel::UpdateMainContent()
{
	if (ImGui::BeginTabBar("tabs"))
	{
		if (ImGui::BeginTabItem("Agent"))
		{
			AgentSettingsTab();
			ImGui::EndTabItem();
		}
		if (ImGui::BeginTabItem("Build"))
		{
			BuildSettingsTab();
			ImGui::EndTabItem();
		}
	}
}

void NavigationPanel::Bake()
{
	if (auto newMesh = CompileWorldMeshes())
	{
		myBuilder.SetInputGeom(newMesh);
		CompileNavLinks();

		if (myBuildSettings.useTileCache)
		{
			myNavigationSystem.SetVTNavMesh(myBuilder.BuildTiledNavMesh());
		}
		else
		{
			myNavigationSystem.SetVTNavMesh(myBuilder.BuildSingleNavMesh());
		}

		if (!myNavigationSystem.GetVTNavMesh())
		{
			return;
		}

		if (myNavigationSystem.GetVTNavMesh()->GetNavMesh() && !myBuildSettings.useTileCache)
		{
			const auto& sceneMeta = Volt::AssetManager::GetMetadataFromHandle(myScene->handle);

			auto outputPath = sceneMeta.filePath;
			outputPath.replace_extension(".vtnavmesh");

			if (!outputPath.stem().empty())
			{
				myNavigationSystem.GetVTNavMesh();
				Volt::AssetManager::Get().SaveAssetAs(myNavigationSystem.GetVTNavMesh(), outputPath);
			}
		}
	}
}

void NavigationPanel::AgentSettingsTab()
{
	ImGui::DragInt("Max Agent", &myBuildSettings.maxAgents, 1.f, 0, 100);
	ImGui::DragFloat("Agent Height", &myBuildSettings.agentHeight, 1.f, 10.f, 500.f);
	ImGui::DragFloat("Agent Radius", &myBuildSettings.agentRadius, 1.f, 0.f, 500.f);
	ImGui::DragFloat("Agent Max Climb", &myBuildSettings.agentMaxClimb, 1.f, 0.1f, 500.f);
	ImGui::DragFloat("Agent Max Slope", &myBuildSettings.agentMaxSlope, 1.f, 0.f, 90.f);
	if (ImGui::Button("Reset to Default"))
	{
		SetDefaultAgentSettings();
	}
}

void NavigationPanel::BuildSettingsTab()
{
	ImGui::DragFloat("Cell Size", &myBuildSettings.cellSize, 1.f, 1.f, 100.f);
	ImGui::DragFloat("Cell Height", &myBuildSettings.cellHeight, 1.f, 1.f, 10.f);
	ImGui::DragFloat("Region Min Size", &myBuildSettings.regionMinSize, 1.f, 0.f, 150.f);
	ImGui::DragFloat("Region Merge Size", &myBuildSettings.regionMergeSize, 1.f, 0.f, 150.f);
	ImGui::DragFloat("Edge Max Len", &myBuildSettings.edgeMaxLen, 1.f, 0.f, 50.f);
	ImGui::DragFloat("Edge Max Error", &myBuildSettings.edgeMaxError, 0.1f, 0.1f, 3.f);
	ImGui::DragFloat("Verts Per Poly", &myBuildSettings.vertsPerPoly, 1.f, 3.f, 6.f);
	ImGui::DragFloat("Detail Sample Dist", &myBuildSettings.detailSampleDist, 1.f, 0.f, 16.f);
	ImGui::DragFloat("Detail Sample Max Error", &myBuildSettings.detailSampleMaxError, 1.f, 0.f, 16.f);
	ImGui::DragFloat("Tile Size", &myBuildSettings.tileSize, 1.f, 0.f, 100.f);
	ImGui::Combo("Partition Type", &myBuildSettings.partitionType, "Watershed\0Monotone\0Layers");
	ImGui::Checkbox("Use TileCache", &myBuildSettings.useTileCache);
	ImGui::Checkbox("Auto-Baking", &myBuildSettings.useAutoBaking);
	if (ImGui::Button("Reset to Default"))
	{
		SetDefaultBuildSettings();
	}
}

Ref<Volt::Mesh> NavigationPanel::CompileWorldMeshes()
{
	Vector<Ref<Volt::Mesh>> srcMeshes;
	Vector<glm::mat4> srcTransforms;

	const bool navMeshComponentsExist = myScene->GetRegistry().view<Volt::NavMeshComponent>().empty();

	if (navMeshComponentsExist)
	{
		//myScene->ForEachWithComponents<const Volt::NavMeshComponent, const Volt::BoxColliderComponent>([&](const entt::entity id, const Volt::NavMeshComponent&, const Volt::BoxColliderComponent& collider) 
		//{
		//	Volt::Entity entity = { id, myScene };
		//	auto transform = entity.GetTransform();

		//	srcMeshes.emplace_back(Volt::AssetManager::GetAsset<Volt::Mesh>("Engine/Meshes/Primitives/SM_Cube_Mesh.vtasset"));
		//	srcTransforms.emplace_back(transform * glm::translate(glm::mat4(1.f), collider.offset) * glm::scale(glm::mat4(1.f), collider.halfSize * 2.f * 0.01f));
		//});

		//myScene->ForEachWithComponents<const Volt::NavMeshComponent, const Volt::CapsuleColliderComponent>([&](const entt::entity id, const Volt::NavMeshComponent&, const Volt::CapsuleColliderComponent& collider)
		//{
		//	Volt::Entity entity = { id, myScene };
		//	auto transform = entity.GetTransform();

		//	srcMeshes.emplace_back(Volt::AssetManager::GetAsset<Volt::Mesh>("Engine/Meshes/Primitives/SM_Capsule.vtasset"));
		//	srcTransforms.emplace_back(transform * glm::translate(glm::mat4(1.f), collider.offset) * glm::scale(glm::mat4(1.f), { collider.radius * 2.f * 0.01f, collider.height * 0.01f, collider.radius * 2.f * 0.01f }));
		//});

		//myScene->ForEachWithComponents<const Volt::NavMeshComponent, const Volt::SphereColliderComponent>([&](const entt::entity id, const Volt::NavMeshComponent&, const Volt::SphereColliderComponent& collider)
		//{
		//	Volt::Entity entity = { id, myScene };
		//	auto transform = entity.GetTransform();
		//
		//	srcMeshes.emplace_back(Volt::AssetManager::GetAsset<Volt::Mesh>("Engine/Meshes/Primitives/SM_Sphere.vtasset"));
		//	srcTransforms.emplace_back(transform * glm::translate(glm::mat4(1.f), collider.offset) * glm::scale(glm::mat4(1.f), { collider.radius * 2.f * 0.01f }));
		//});

		//myScene->ForEachWithComponents<const Volt::NavMeshComponent, const Volt::CapsuleColliderComponent>([&](const entt::entity id, const Volt::NavMeshComponent&, const Volt::MeshColliderComponent& collider)
		//{
		//	Volt::Entity entity = { id, myScene };
		//	auto transform = entity.GetTransform();

		//	Ref<Volt::Mesh> mesh;
		//	mesh = Volt::AssetManager::GetAsset<Volt::Mesh>(collider.colliderMesh);

		//	if (!mesh)
		//	{
		//		return;
		//	}

		//	srcMeshes.emplace_back(mesh);
		//	srcTransforms.emplace_back(transform);
		//});

		if (!srcMeshes.empty())
		{
			return Volt::MeshExporterUtilities::CombineMeshes(srcMeshes, srcTransforms, nullptr);
		}
	}

	return nullptr;
}

void NavigationPanel::CompileNavLinks()
{
	myBuilder.ClearNavLinkConnections();

	myScene->ForEachWithComponents<const Volt::NavLinkComponent>([&](const entt::entity id, const Volt::NavLinkComponent& comp) 
	{
		Volt::Entity entity = { id, myScene };
		Volt::AI::NavLinkConnection link;

		link.start = entity.GetPosition() + comp.start;
		link.end = entity.GetPosition() + comp.end;
		link.bidirectional = comp.bidirectional;
		link.active = comp.active;

		myBuilder.AddNavLinkConnection(link);
	});
}

void NavigationPanel::SetDefaultAgentSettings()
{
	auto& buildSettings = UserSettingsManager::GetSettings().navmeshBuildSettings;

	buildSettings.maxAgents = 100;
	buildSettings.agentHeight = 200.f;
	buildSettings.agentRadius = 60.f;
	buildSettings.agentMaxClimb = 90.f;
	buildSettings.agentMaxSlope = 45.f;
}

void NavigationPanel::SetDefaultBuildSettings()
{
	auto& buildSettings = UserSettingsManager::GetSettings().navmeshBuildSettings;

	buildSettings.cellSize = 30.f;
	buildSettings.cellHeight = 1.f;
	buildSettings.regionMinSize = 8.f;
	buildSettings.regionMergeSize = 20.f;
	buildSettings.edgeMaxLen = 12.f;
	buildSettings.edgeMaxError = 1.3f;
	buildSettings.vertsPerPoly = 6.f;
	buildSettings.detailSampleDist = 6.f;
	buildSettings.detailSampleMaxError = 1.f;
	buildSettings.partitionType = PartitionType::PARTITION_WATERSHED;
}
