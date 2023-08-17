#include "sbpch.h"
#include "Sandbox.h"

#include "Sandbox/UserSettingsManager.h"
#include "Sandbox/Utility/SelectionManager.h"
#include "Sandbox/Utility/EditorResources.h"

#include <Volt/Core/Application.h>

#include <Volt/Asset/AssetManager.h>
#include <Volt/Asset/Mesh/Material.h>

#include <Volt/Physics/MeshColliderCache.h>

#include <Volt/Components/Components.h>
#include <Volt/Components/LightComponents.h>
#include <Volt/Components/PhysicsComponents.h>

#include <Volt/Rendering/DebugRenderer.h>
#include <Volt/RenderingNew/SceneRendererNew.h>

#include <NavigationEditor/Tools/NavMeshDebugDrawer.h>
#include <NavigationEditor/Builder/RecastBuilder.h>

void Sandbox::RenderSelection(Ref<Volt::Camera> camera)
{
	VT_PROFILE_FUNCTION();

	auto& registry = myRuntimeScene->GetRegistry();

	for (const auto& ent : SelectionManager::GetSelectedEntities())
	{
		if (!registry.HasComponent<Volt::TransformComponent>(ent))
		{
			continue;
		}

		auto& transComp = registry.GetComponent<Volt::TransformComponent>(ent);

		if (!transComp.visible || !registry.HasComponent<Volt::MeshComponent>(ent))
		{
			continue;
		}

		auto& meshComp = registry.GetComponent<Volt::MeshComponent>(ent);
		auto mesh = Volt::AssetManager::GetAsset<Volt::Mesh>(meshComp.GetHandle());
		if (!mesh || !mesh->IsValid())
		{
			continue;
		}

		//mySceneRenderer->SubmitOutlineMesh(mesh, myRuntimeScene->GetWorldSpaceTransform(Volt::Entity{ent, myRuntimeScene.get()}));
	}
}

void Sandbox::RenderGizmos(Ref<Volt::Scene> scene, Ref<Volt::Camera> camera)
{
	VT_PROFILE_FUNCTION();

	const auto& settings = UserSettingsManager::GetSettings().sceneSettings;

	if (!settings.showGizmos)
	{
		return;
	}

	auto& registry = scene->GetRegistry();

	//Sandbox::Get().GetSceneRenderer()->SetHideStaticMeshes(settings.colliderViewMode == ColliderViewMode::AllHideMesh || settings.navMeshViewMode == NavMeshViewMode::Only);

	if (settings.showEntityGizmos)
	{
		registry.ForEach<Volt::TransformComponent>([&](Wire::EntityId id, const Volt::TransformComponent& transformComp)
		{
			if (registry.HasComponent<Volt::CameraComponent>(id) || registry.HasComponent<Volt::PointLightComponent>(id) || registry.HasComponent<Volt::SpotLightComponent>(id))
			{
				return;
			}

			if (!transformComp.visible)
			{
				return;
			}

			Volt::Entity entity{ id, myRuntimeScene.get() };

			glm::vec3 p = entity.GetPosition();

			const float maxDist = 5000.f * 5000.f;
			const float lerpStartDist = 4000.f * 4000.f;
			const float maxScale = 1.f;
			const float minScale = 0.3f;
			const float distance = glm::distance2(camera->GetPosition(), p);

			float alpha = 1.f;

			if (distance >= lerpStartDist)
			{
				alpha = glm::mix(1.f, 0.f, (distance - lerpStartDist) / (maxDist - lerpStartDist));
			}

			if (distance < maxDist)
			{
				float scale = glm::max(glm::min(distance / maxDist * 2.f, maxScale), minScale);
				Volt::DebugRenderer::DrawBillboard(EditorResources::GetEditorIcon(EditorIcon::EntityGizmo), p, scale, glm::vec4{ 1.f, 1.f, 1.f, alpha }, id);
			}
		});
	}

	if (settings.showEntityGizmos || settings.showLightSpheres)
	{
		registry.ForEach<Volt::PointLightComponent, Volt::TransformComponent>([&](Wire::EntityId id, const Volt::PointLightComponent& lightComp, const Volt::TransformComponent& transformComp)
		{
			if (!transformComp.visible)
			{
				return;
			}

			Volt::Entity entity{ id, myRuntimeScene.get() };

			glm::vec3 p = entity.GetPosition();

			const float maxDist = 5000.f;
			const float lerpStartDist = 4000.f;
			const float maxScale = 1.f;
			const float distance = glm::distance(camera->GetPosition(), p);

			float alpha = 1.f;

			if (distance >= lerpStartDist)
			{
				alpha = glm::mix(1.f, 0.f, (distance - lerpStartDist) / (maxDist - lerpStartDist));
			}

			if (distance < maxDist)
			{
				float scale = glm::min(distance / maxDist, maxScale);
				Volt::DebugRenderer::DrawBillboard(EditorResources::GetEditorIcon(EditorIcon::LightGizmo), p, scale, glm::vec4{ 1.f, 1.f, 1.f, alpha }, id);
			}
		});

		registry.ForEach<Volt::SpotLightComponent, Volt::TransformComponent>([&](Wire::EntityId id, const Volt::SpotLightComponent& lightComp, const Volt::TransformComponent& transformComp)
		{
			if (!transformComp.visible)
			{
				return;
			}

			Volt::Entity entity{ id, myRuntimeScene.get() };

			glm::vec3 p = entity.GetPosition();

			const float maxDist = 5000.f;
			const float lerpStartDist = 4000.f;
			const float maxScale = 1.f;
			const float distance = glm::distance(camera->GetPosition(), p);

			float alpha = 1.f;

			if (distance >= lerpStartDist)
			{
				alpha = glm::mix(1.f, 0.f, (distance - lerpStartDist) / (maxDist - lerpStartDist));
			}

			if (distance < maxDist)
			{
				float scale = glm::min(distance / maxDist, maxScale);
				Volt::DebugRenderer::DrawBillboard(EditorResources::GetEditorIcon(EditorIcon::LightGizmo), p, scale, glm::vec4{ 1.f, 1.f, 1.f, alpha }, id);
			}
		});

		if (settings.showLightSpheres)
		{
			registry.ForEach<Volt::PointLightComponent>([&](Wire::EntityId id, const Volt::PointLightComponent& comp)
			{
				Volt::Entity entity{ id, myRuntimeScene.get() };
				Volt::DebugRenderer::DrawLineSphere(entity.GetPosition(), comp.radius);
			});
		}

		///// Sphere Bounds Visualization /////
		if (settings.showBoundingSpheres)
		{
			registry.ForEach<Volt::MeshComponent>([&](Wire::EntityId id, const Volt::MeshComponent& comp)
			{
				if (comp.GetHandle() == Volt::Asset::Null())
				{
					return;
				}

				const auto mesh = Volt::AssetManager::GetAsset<Volt::Mesh>(comp.GetHandle());
				if (!mesh || !mesh->IsValid())
				{
					return;
				}

				Volt::Entity entity{ id, myRuntimeScene.get() };

				const auto& boundingSphere = mesh->GetBoundingSphere();
				const auto transform = myRuntimeScene->GetWorldSpaceTransform(entity);

				const glm::vec3 globalScale = { glm::length(transform[0]), glm::length(transform[1]), glm::length(transform[2]) };
				const float maxScale = std::max(std::max(globalScale.x, globalScale.y), globalScale.z);

				Volt::DebugRenderer::DrawLineSphere(entity.GetPosition() + boundingSphere.center, boundingSphere.radius * maxScale);
			});
		}
	}

	///// Sphere Bounds Visualization /////
	switch (settings.navMeshViewMode)
	{
		case NavMeshViewMode::Only:
		case NavMeshViewMode::All:
		{
			auto& myNavigationSystem = Volt::Application::Get().GetNavigationSystem();

			if (myNavigationSystem.GetVTNavMesh())
			{
				// Draw NavMesh
				{
					NavMeshDebugDrawer::DrawNavMesh();
				}

				// Draw NavLinks
				{
					std::vector<Volt::AI::NavLinkConnection> links;
					auto entitiesWithNavLink = myRuntimeScene->GetRegistry().GetComponentView<Volt::NavLinkComponent>();

					for (auto ent : entitiesWithNavLink)
					{
						Volt::AI::NavLinkConnection link;
						auto& transformComp = myRuntimeScene->GetRegistry().GetComponent<Volt::TransformComponent>(ent);
						auto& linkComp = myRuntimeScene->GetRegistry().GetComponent<Volt::NavLinkComponent>(ent);

						link.start = transformComp.position + linkComp.start;
						link.end = transformComp.position + linkComp.end;
						link.bidirectional = linkComp.bidirectional;

						links.emplace_back(link);
					}

					NavMeshDebugDrawer::DrawLinks(links);
				}
			}
		}
	}

	///////////////////////////////////////

	///// Camera Gizmo /////
	registry.ForEach<Volt::CameraComponent, Volt::TransformComponent>([&](Wire::EntityId id, const Volt::CameraComponent& cameraComponent, const Volt::TransformComponent& transComp)
	{
		if (!transComp.visible)
		{
			return;
		}

		const auto cameraMesh = Volt::AssetManager::GetAsset<Volt::Mesh>("Editor/Meshes/Gizmos/SM_Camera_Gizmo.vtmesh");
		const auto material = Volt::AssetManager::GetAsset<Volt::Material>("Editor/Materials/M_Camera_Gizmo.vtmat");

		if (!cameraMesh || !cameraMesh->IsValid() || !material || !material->IsValid())
		{
			return;
		}

		const auto transform = myRuntimeScene->GetWorldSpaceTransform(Volt::Entity{ id, myRuntimeScene.get() });
		Volt::DebugRenderer::DrawMesh(cameraMesh, material, transform);

		// Frustums
		{
			//const auto frustumPoints = cameraComponent.camera->GetFrustumPoints();

			//Volt::DebugRenderer::DrawLine(frustumPoints[4], frustumPoints[0]);
			//Volt::DebugRenderer::DrawLine(frustumPoints[5], frustumPoints[1]);

			//Volt::DebugRenderer::DrawLine(frustumPoints[2], frustumPoints[6]);
			//Volt::DebugRenderer::DrawLine(frustumPoints[3], frustumPoints[7]);
		}
	});
	////////////////////////

	///// Collider Visualization /////
	switch (settings.colliderViewMode)
	{
		case ColliderViewMode::AllHideMesh:
		case ColliderViewMode::All:
		{
			auto collisionMaterial = Volt::AssetManager::GetAsset<Volt::Material>("Editor/Materials/M_ColliderDebug.vtmat");
			registry.ForEach<Volt::BoxColliderComponent>([&](Wire::EntityId id, const Volt::BoxColliderComponent& collider)
			{
				Volt::Entity entity{ id, myRuntimeScene.get() };
				auto transform = myRuntimeScene->GetWorldSpaceTransform(entity);

				auto cubeMesh = Volt::AssetManager::GetAsset<Volt::Mesh>("Engine/Meshes/Primitives/SM_Cube.vtmesh");

				const glm::mat4 colliderTransform = glm::translate(glm::mat4(1.f), collider.offset) * glm::scale(glm::mat4(1.f), collider.halfSize * 2.f * 0.01f);
				Volt::DebugRenderer::DrawMesh(cubeMesh, collisionMaterial, transform * colliderTransform, id);
			});

			registry.ForEach<Volt::SphereColliderComponent>([&](Wire::EntityId id, const Volt::SphereColliderComponent& collider)
			{
				Volt::Entity entity{ id, myRuntimeScene.get() };
				auto transform = myRuntimeScene->GetWorldSpaceTransform(entity);

				auto sphereMesh = Volt::AssetManager::GetAsset<Volt::Mesh>("Engine/Meshes/Primitives/SM_Sphere.vtmesh");

				const glm::mat4 colliderTransform = glm::translate(glm::mat4(1.f), collider.offset) * glm::scale(glm::mat4(1.f), { collider.radius * 2.f * 0.01f });
				Volt::DebugRenderer::DrawMesh(sphereMesh, collisionMaterial, transform * colliderTransform, id);
			});

			registry.ForEach<Volt::CapsuleColliderComponent>([&](Wire::EntityId id, const Volt::CapsuleColliderComponent& collider)
			{
				Volt::Entity entity{ id, myRuntimeScene.get() };
				auto transform = myRuntimeScene->GetWorldSpaceTransform(entity);

				const glm::mat4 colliderTransform = glm::translate(glm::mat4(1.f), collider.offset) * glm::scale(glm::mat4(1.f), { collider.radius * 2.f * 0.01f, collider.height * 0.01f, collider.radius * 2.f * 0.01f });
				auto capsuleMesh = Volt::AssetManager::GetAsset<Volt::Mesh>("Engine/Meshes/Primitives/SM_Capsule.vtmesh");

				Volt::DebugRenderer::DrawMesh(capsuleMesh, collisionMaterial, transform * colliderTransform, id);
			});

			registry.ForEach<Volt::MeshColliderComponent>([&](Wire::EntityId id, const Volt::MeshColliderComponent& collider)
			{
				Volt::Entity entity{ id, myRuntimeScene.get() };
				auto transform = myRuntimeScene->GetWorldSpaceTransform(entity);

				Ref<Volt::Mesh> debugMesh;

				debugMesh = Volt::AssetManager::GetAsset<Volt::Mesh>(collider.colliderMesh);

				if (!debugMesh)
				{
					return;
				}

				Volt::DebugRenderer::DrawMesh(debugMesh, collisionMaterial, transform, id);
			});

			break;
		}

		case ColliderViewMode::Selected:
		{
			auto collisionMaterial = Volt::AssetManager::GetAsset<Volt::Material>("Editor/Materials/M_ColliderDebug.vtmat");

			for (const auto& id : SelectionManager::GetSelectedEntities())
			{
				Volt::Entity entity{ id, myRuntimeScene.get() };

				if (entity.HasComponent<Volt::BoxColliderComponent>())
				{
					const auto& collider = entity.GetComponent<Volt::BoxColliderComponent>();

					auto transform = myRuntimeScene->GetWorldSpaceTransform(entity);
					auto cubeMesh = Volt::AssetManager::GetAsset<Volt::Mesh>("Engine/Meshes/Primitives/SM_Cube.vtmesh");

					const glm::mat4 colliderTransform = glm::translate(glm::mat4(1.f), collider.offset) * glm::scale(glm::mat4(1.f), collider.halfSize * 2.f * 0.01f);
					Volt::DebugRenderer::DrawMesh(cubeMesh, collisionMaterial, transform * colliderTransform, id);
				}

				if (entity.HasComponent<Volt::SphereColliderComponent>())
				{
					const auto& collider = entity.GetComponent<Volt::SphereColliderComponent>();

					auto transform = myRuntimeScene->GetWorldSpaceTransform(entity);
					auto sphereMesh = Volt::AssetManager::GetAsset<Volt::Mesh>("Engine/Meshes/Primitives/SM_Sphere.vtmesh");

					const glm::mat4 colliderTransform = glm::translate(glm::mat4(1.f), collider.offset) * glm::scale(glm::mat4(1.f), { collider.radius * 2.f * 0.01f });
					Volt::DebugRenderer::DrawMesh(sphereMesh, collisionMaterial, transform * colliderTransform, id);
				}

				if (entity.HasComponent<Volt::CapsuleColliderComponent>())
				{
					const auto& collider = entity.GetComponent<Volt::CapsuleColliderComponent>();
					auto transform = myRuntimeScene->GetWorldSpaceTransform(entity);

					const glm::mat4 colliderTransform = glm::translate(glm::mat4(1.f), collider.offset) * glm::scale(glm::mat4(1.f), { collider.radius * 2.f * 0.01f, collider.height * 0.01f, collider.radius * 2.f * 0.01f });
					auto capsuleMesh = Volt::AssetManager::GetAsset<Volt::Mesh>("Engine/Meshes/Primitives/SM_Capsule.vtmesh");

					Volt::DebugRenderer::DrawMesh(capsuleMesh, collisionMaterial, transform * colliderTransform, id);
				}

				if (entity.HasComponent<Volt::MeshColliderComponent>())
				{
					const auto& collider = entity.GetComponent<Volt::MeshColliderComponent>();
					auto transform = myRuntimeScene->GetWorldSpaceTransform(entity);

					Ref<Volt::Mesh> debugMesh;

					debugMesh = Volt::AssetManager::GetAsset<Volt::Mesh>(collider.colliderMesh);

					if (!debugMesh)
					{
						return;
					}

					Volt::DebugRenderer::DrawMesh(debugMesh, collisionMaterial, transform, id);
				}
			}

			break;
		}
	}
	//////////////////////////////////
}
