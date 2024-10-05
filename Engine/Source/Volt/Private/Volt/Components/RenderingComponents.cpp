#include "vtpch.h"
#include "Volt/Components/RenderingComponents.h"

#include "Volt/Rendering/RenderScene/ScenePrimitiveData.h"

#include "Volt/Rendering/RenderScene.h"
#include "Volt/Rendering/Camera/Camera.h"
#include "Volt/Asset/Mesh/Mesh.h"
#include "Volt/Asset/Rendering/Material.h"

#include "Volt/Scene/SceneManager.h"

#include "Volt/Animation/MotionWeaver.h"

#include <AssetSystem/AssetManager.h>

namespace Volt
{
	void MeshComponent::OnCreate(MeshEntity entity)
	{
		auto& meshComponent = entity.GetComponent<MeshComponent>();
		meshComponent.m_scenePrimitiveData = new ScenePrimitiveData(entity.GetID(), entity.GetRenderScene());
	}

	void MeshComponent::OnDestroy(MeshEntity entity)
	{
		auto& component = entity.GetComponent<MeshComponent>();
		delete component.m_scenePrimitiveData;
	}

	void MeshComponent::OnMemberChanged(MeshEntity entity)
	{
		auto& component = entity.GetComponent<MeshComponent>();

		if (component.handle == Asset::Null())
		{
			return;
		}

		ScenePrimitiveDescription description{};
		description.primitiveMesh = component.handle;
		description.materials = component.materials;

		component.m_scenePrimitiveData->InitializeFromDescription(description);
	}

	void MeshComponent::OnComponentCopied(MeshEntity entity)
	{
		auto& component = entity.GetComponent<MeshComponent>();

		if (component.handle == Asset::Null())
		{
			return;
		}

		ScenePrimitiveDescription description{};
		description.primitiveMesh = component.handle;
		description.materials = component.materials;

		component.m_scenePrimitiveData->InitializeFromDescription(description);
	}

	void MeshComponent::OnTransformChanged(MeshEntity entity)
	{
		auto& meshComponent = entity.GetComponent<MeshComponent>();
		meshComponent.m_scenePrimitiveData->Invalidate();
	}

	void CameraComponent::OnCreate(CameraEntity entity)
	{
		auto& component = entity.GetComponent<CameraComponent>();
		component.camera = CreateRef<Camera>(component.fieldOfView, 1.f, 16.f / 9.f, component.nearPlane, component.farPlane);
	}

	void MotionWeaveComponent::OnStart(WeaveEntity entity)
	{
		const auto& meshComponent = entity.GetComponent<MeshComponent>();
		auto& weaveComponent = entity.GetComponent<MotionWeaveComponent>();

		auto scene = SceneManager::GetActiveScene();

		auto sceneEntity = scene->GetSceneEntityFromScriptingEntity(entity);

		weaveComponent.MotionWeaver = MotionWeaver::Create(weaveComponent.motionWeaveDatabase);

		Ref<Mesh> mesh = AssetManager::GetAsset<Mesh>(meshComponent.handle);
		if (mesh && mesh->IsValid())
		{
			const auto& materialTable = mesh->GetMaterialTable();

			for (size_t i = 0; i < mesh->GetSubMeshes().size(); i++)
			{
				auto material = AssetManager::QueueAsset<Material>(materialTable.GetMaterial(mesh->GetSubMeshes().at(i).materialIndex));
				if (!material->IsValid())
				{
				}

				auto uuid = scene->GetRenderScene()->Register(entity.GetID(), weaveComponent.MotionWeaver, mesh, material, static_cast<uint32_t>(i));
				weaveComponent.renderObjectIds.emplace_back(uuid);
			}
		}
	}
}
