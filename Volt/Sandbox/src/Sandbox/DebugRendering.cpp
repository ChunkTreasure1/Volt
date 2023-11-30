#include "sbpch.h"
#include "Sandbox.h"

#include "Sandbox/UserSettingsManager.h"
#include "Sandbox/Utility/SelectionManager.h"
#include "Sandbox/Utility/EditorResources.h"

#include <Volt/Core/Application.h>

#include <Volt/Asset/AssetManager.h>
#include <Volt/Asset/Mesh/Material.h>

#include <Volt/Physics/MeshColliderCache.h>

#include <Volt/Components/RenderingComponents.h>
#include <Volt/Components/CoreComponents.h>
#include <Volt/Components/LightComponents.h>
#include <Volt/Components/PhysicsComponents.h>

#include <Volt/Rendering/DebugRenderer.h>
#include <Volt/Rendering/SceneRenderer.h>
#include <Volt/Rendering/Camera/Camera.h>

#include <NavigationEditor/Tools/NavMeshDebugDrawer.h>
#include <NavigationEditor/Builder/RecastBuilder.h>

void Sandbox::RenderSelection(Ref<Volt::Camera> camera)
{
	VT_PROFILE_FUNCTION();

	for (const auto& id : SelectionManager::GetSelectedEntities())
	{
		Volt::Entity entity = myRuntimeScene->GetEntityFromUUID(id);

		if (!entity.HasComponent<Volt::TransformComponent>())
		{
			continue;
		}

		auto& transComp = entity.GetComponent<Volt::TransformComponent>();

		if (!transComp.visible || !entity.HasComponent<Volt::MeshComponent>())
		{
			continue;
		}

		auto& meshComp = entity.GetComponent<Volt::MeshComponent>();
		auto mesh = Volt::AssetManager::GetAsset<Volt::Mesh>(meshComp.handle);
		if (!mesh || !mesh->IsValid())
		{
			continue;
		}

		mySceneRenderer->SubmitOutlineMesh(mesh, entity.GetTransform());
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

	Sandbox::Get().GetSceneRenderer()->SetHideStaticMeshes(settings.colliderViewMode == ColliderViewMode::AllHideMesh || settings.navMeshViewMode == NavMeshViewMode::Only);

	if (settings.showEntityGizmos)
	{
		myRuntimeScene->ForEachWithComponents<const Volt::TransformComponent>([&](entt::entity id, const Volt::TransformComponent& transformComp)
		{
			Volt::Entity entity{ id, myRuntimeScene };

			if (entity.HasComponent<Volt::CameraComponent>() || entity.HasComponent<Volt::PointLightComponent>() || entity.HasComponent<Volt::SpotLightComponent>())
			{
				return;
			}

			if (!transformComp.visible)
			{
				return;
			}

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
				Volt::DebugRenderer::DrawBillboard(EditorResources::GetEditorIcon(EditorIcon::EntityGizmo), p, scale, glm::vec4{ 1.f, 1.f, 1.f, alpha }, entity.GetID());
			}
		});
	}

	if (settings.showEntityGizmos || settings.showLightSpheres)
	{
		myRuntimeScene->ForEachWithComponents<const Volt::PointLightComponent, const Volt::TransformComponent>([&](entt::entity id, const Volt::PointLightComponent& lightComp, const Volt::TransformComponent& transformComp)
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
				Volt::DebugRenderer::DrawBillboard(EditorResources::GetEditorIcon(EditorIcon::LightGizmo), p, scale, glm::vec4{ 1.f, 1.f, 1.f, alpha }, entity.GetID());
			}
		});

		myRuntimeScene->ForEachWithComponents<const Volt::SpotLightComponent, const Volt::TransformComponent>([&](entt::entity id, const Volt::SpotLightComponent& lightComp, const Volt::TransformComponent& transformComp)
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
				Volt::DebugRenderer::DrawBillboard(EditorResources::GetEditorIcon(EditorIcon::LightGizmo), p, scale, glm::vec4{ 1.f, 1.f, 1.f, alpha }, entity.GetID());
			}
		});

		if (settings.showLightSpheres)
		{
			myRuntimeScene->ForEachWithComponents<const Volt::PointLightComponent>([&](entt::entity id, const Volt::PointLightComponent& comp)
			{
				Volt::Entity entity{ id, myRuntimeScene.get() };
				Volt::DebugRenderer::DrawLineSphere(entity.GetPosition(), comp.radius);
			});
		}

		///// Sphere Bounds Visualization /////
		if (settings.showBoundingSpheres)
		{
			myRuntimeScene->ForEachWithComponents<const Volt::MeshComponent>([&](entt::entity id, const Volt::MeshComponent& comp)
			{
				if (comp.handle == Volt::Asset::Null())
				{
					return;
				}

				const auto mesh = Volt::AssetManager::GetAsset<Volt::Mesh>(comp.handle);
				if (!mesh || !mesh->IsValid())
				{
					return;
				}

				Volt::Entity entity{ id, myRuntimeScene.get() };

				const auto& boundingSphere = mesh->GetBoundingSphere();
				const auto transform = entity.GetTransform();

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
					myRuntimeScene->ForEachWithComponents<const Volt::NavLinkComponent>([&](const entt::entity id, const Volt::NavLinkComponent& comp)
					{
						Volt::Entity entity{ id, myRuntimeScene.get() };
						Volt::AI::NavLinkConnection link;

						link.start = entity.GetPosition() + comp.start;
						link.end = entity.GetPosition() + comp.end;
						link.bidirectional = comp.bidirectional;

						links.emplace_back(link);
					});

					NavMeshDebugDrawer::DrawLinks(links);
				}
			}
		}
	}

	///////////////////////////////////////

	///// Camera Gizmo /////
	myRuntimeScene->ForEachWithComponents<Volt::CameraComponent>([&](entt::entity id, const Volt::CameraComponent& cameraComponent)
	{
		Volt::Entity entity{ id, myRuntimeScene.get() };

		if (!entity.IsVisible())
		{
			return;
		}

		const auto cameraMesh = Volt::AssetManager::GetAsset<Volt::Mesh>("Editor/Meshes/Gizmos/SM_Camera_Gizmo.vtmesh");
		const auto material = Volt::AssetManager::GetAsset<Volt::Material>("Editor/Materials/M_Camera_Gizmo.vtmat");

		if (!cameraMesh || !cameraMesh->IsValid() || !material || !material->IsValid())
		{
			return;
		}

		Volt::DebugRenderer::DrawMesh(cameraMesh, material, entity.GetTransform());

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
			myRuntimeScene->ForEachWithComponents<const Volt::BoxColliderComponent>([&](entt::entity id, const Volt::BoxColliderComponent& collider)
			{
				Volt::Entity entity{ id, myRuntimeScene.get() };

				auto cubeMesh = Volt::AssetManager::GetAsset<Volt::Mesh>("Engine/Meshes/Primitives/SM_Cube.vtmesh");

				const glm::mat4 colliderTransform = glm::translate(glm::mat4(1.f), collider.offset) * glm::scale(glm::mat4(1.f), collider.halfSize * 2.f * 0.01f);
				Volt::DebugRenderer::DrawMesh(cubeMesh, collisionMaterial, entity.GetTransform() * colliderTransform, entity.GetID());
			});

			myRuntimeScene->ForEachWithComponents<const Volt::SphereColliderComponent>([&](entt::entity id, const Volt::SphereColliderComponent& collider)
			{
				Volt::Entity entity{ id, myRuntimeScene.get() };

				auto sphereMesh = Volt::AssetManager::GetAsset<Volt::Mesh>("Engine/Meshes/Primitives/SM_Sphere.vtmesh");

				const glm::mat4 colliderTransform = glm::translate(glm::mat4(1.f), collider.offset) * glm::scale(glm::mat4(1.f), { collider.radius * 2.f * 0.01f });
				Volt::DebugRenderer::DrawMesh(sphereMesh, collisionMaterial, entity.GetTransform() * colliderTransform, entity.GetID());
			});

			myRuntimeScene->ForEachWithComponents<const Volt::CapsuleColliderComponent>([&](entt::entity id, const Volt::CapsuleColliderComponent& collider)
			{
				Volt::Entity entity{ id, myRuntimeScene.get() };

				const glm::mat4 colliderTransform = glm::translate(glm::mat4(1.f), collider.offset) * glm::scale(glm::mat4(1.f), { collider.radius * 2.f * 0.01f, collider.height * 0.01f, collider.radius * 2.f * 0.01f });
				auto capsuleMesh = Volt::AssetManager::GetAsset<Volt::Mesh>("Engine/Meshes/Primitives/SM_Capsule.vtmesh");

				Volt::DebugRenderer::DrawMesh(capsuleMesh, collisionMaterial, entity.GetTransform() * colliderTransform, entity.GetID());
			});

			myRuntimeScene->ForEachWithComponents<const Volt::MeshColliderComponent>([&](entt::entity id, const Volt::MeshColliderComponent& collider)
			{
				Volt::Entity entity{ id, myRuntimeScene.get() };

				Ref<Volt::Mesh> debugMesh;

				debugMesh = Volt::AssetManager::GetAsset<Volt::Mesh>(collider.colliderMesh);

				if (!debugMesh)
				{
					return;
				}

				Volt::DebugRenderer::DrawMesh(debugMesh, collisionMaterial, entity.GetTransform(), entity.GetID());
			});

			break;
		}

		case ColliderViewMode::Selected:
		{
			auto collisionMaterial = Volt::AssetManager::GetAsset<Volt::Material>("Editor/Materials/M_ColliderDebug.vtmat");

			for (const auto& id : SelectionManager::GetSelectedEntities())
			{
				Volt::Entity entity = myRuntimeScene->GetEntityFromUUID(id);

				if (entity.HasComponent<Volt::BoxColliderComponent>())
				{
					const auto& collider = entity.GetComponent<Volt::BoxColliderComponent>();
					auto cubeMesh = Volt::AssetManager::GetAsset<Volt::Mesh>("Engine/Meshes/Primitives/SM_Cube.vtmesh");

					const glm::mat4 colliderTransform = glm::translate(glm::mat4(1.f), collider.offset) * glm::scale(glm::mat4(1.f), collider.halfSize * 2.f * 0.01f);
					Volt::DebugRenderer::DrawMesh(cubeMesh, collisionMaterial, entity.GetTransform() * colliderTransform, entity.GetID());
				}

				if (entity.HasComponent<Volt::SphereColliderComponent>())
				{
					const auto& collider = entity.GetComponent<Volt::SphereColliderComponent>();
					auto sphereMesh = Volt::AssetManager::GetAsset<Volt::Mesh>("Engine/Meshes/Primitives/SM_Sphere.vtmesh");

					const glm::mat4 colliderTransform = glm::translate(glm::mat4(1.f), collider.offset) * glm::scale(glm::mat4(1.f), { collider.radius * 2.f * 0.01f });
					Volt::DebugRenderer::DrawMesh(sphereMesh, collisionMaterial, entity.GetTransform() * colliderTransform, entity.GetID());
				}

				if (entity.HasComponent<Volt::CapsuleColliderComponent>())
				{
					const auto& collider = entity.GetComponent<Volt::CapsuleColliderComponent>();

					const glm::mat4 colliderTransform = glm::translate(glm::mat4(1.f), collider.offset) * glm::scale(glm::mat4(1.f), { collider.radius * 2.f * 0.01f, collider.height * 0.01f, collider.radius * 2.f * 0.01f });
					auto capsuleMesh = Volt::AssetManager::GetAsset<Volt::Mesh>("Engine/Meshes/Primitives/SM_Capsule.vtmesh");

					Volt::DebugRenderer::DrawMesh(capsuleMesh, collisionMaterial, entity.GetTransform() * colliderTransform, entity.GetID());
				}

				if (entity.HasComponent<Volt::MeshColliderComponent>())
				{
					const auto& collider = entity.GetComponent<Volt::MeshColliderComponent>();

					Ref<Volt::Mesh> debugMesh;

					debugMesh = Volt::AssetManager::GetAsset<Volt::Mesh>(collider.colliderMesh);

					if (!debugMesh)
					{
						return;
					}

					Volt::DebugRenderer::DrawMesh(debugMesh, collisionMaterial, entity.GetTransform(), entity.GetID());
				}
			}

			break;
		}
	}
	//////////////////////////////////
}
