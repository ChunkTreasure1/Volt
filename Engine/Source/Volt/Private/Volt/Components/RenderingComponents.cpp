#include "vtpch.h"
#include "Volt/Components/RenderingComponents.h"

#include "Volt/Rendering/RenderScene.h"
#include "Volt/Rendering/Renderer.h"
#include "Volt/Rendering/Camera/Camera.h"
#include "Volt/Asset/Mesh/Mesh.h"
#include "Volt/Asset/Rendering/Material.h"

#include "Volt/Components/CoreComponents.h"
#include "Volt/Scene/SceneManager.h"

#include "Volt/Animation/MotionWeaver.h"

#include <AssetSystem/AssetManager.h>

namespace Volt
{
	void MeshComponent::OnDestroy(MeshEntity entity)
	{
		auto& component = entity.GetComponent<MeshComponent>();

		for (const auto& renderId : component.renderObjectIds)
		{
			SceneManager::GetActiveScene()->GetRenderScene()->Unregister(renderId);
		}
	}

	void MeshComponent::OnMemberChanged(MeshEntity entity)
	{
		auto& component = entity.GetComponent<MeshComponent>();

		auto scene = SceneManager::GetActiveScene();
		auto renderScene = scene->GetRenderScene();

		if (component.handle != component.m_oldHandle)
		{
			if (component.m_oldHandle != Asset::Null())
			{
				for (const auto& uuid : component.renderObjectIds)
				{
					renderScene->Unregister(uuid);
				}
			}

			component.renderObjectIds.clear();

			Ref<Mesh> mesh = AssetManager::GetAsset<Mesh>(component.handle);
			if (mesh && mesh->IsValid())
			{
				const auto& idComp = entity.GetComponent<IDComponent>();
				const auto& materialTable = mesh->GetMaterialTable();

				for (size_t i = 0; i < mesh->GetSubMeshes().size(); i++)
				{
					auto material = AssetManager::QueueAsset<Material>(materialTable.GetMaterial(mesh->GetSubMeshes().at(i).materialIndex));
					if (!material->IsValid())
					{
					}

					auto uuid = renderScene->Register(idComp.id, mesh, material, static_cast<uint32_t>(i));
					component.renderObjectIds.emplace_back(uuid);
				}

				component.materials.clear();

				for (const auto& material : materialTable)
				{
					component.materials.emplace_back(material);
				}
			}

			component.m_oldHandle = component.handle;
			component.m_oldMaterials = component.materials;
		}
		else
		{ 
			for (uint32_t index = 0; const auto& materialHandle : component.materials)
			{
				if (materialHandle == component.m_oldMaterials.at(index))
				{
					continue;
				}

				Ref<Material> material = AssetManager::GetAsset<Material>(materialHandle);
				if (!material || !material->IsValid())
				{
					continue;
				}

				// Find meshes using material and reregister them
				Ref<Mesh> mesh = AssetManager::GetAsset<Mesh>(component.handle);
				if (mesh && mesh->IsValid())
				{
					for (uint32_t subMeshIndex = 0; const auto& subMesh : mesh->GetSubMeshes())
					{
						if (subMesh.materialIndex != index)
						{
							continue;
						}

						auto id = component.renderObjectIds.at(subMeshIndex);
						renderScene->Unregister(id);

						component.renderObjectIds.at(subMeshIndex) = renderScene->Register(entity.GetID(), mesh, material, subMeshIndex);
						subMeshIndex++;
					}
				}

				index++;
			}
		}
	}

	void MeshComponent::OnComponentCopied(MeshEntity entity)
	{
		auto scene = SceneManager::GetActiveScene();
		auto renderScene = scene->GetRenderScene();

		auto& component = entity.GetComponent<MeshComponent>();

		for (const auto& uuid : component.renderObjectIds)
		{
			renderScene->Unregister(uuid);
		}

		if (component.handle == Asset::Null())
		{
			return;
		}

		Ref<Mesh> mesh = AssetManager::GetAsset<Mesh>(component.handle);
		if (!mesh)
		{
			return;
		}

		const auto& materialTable = mesh->GetMaterialTable();

		for (size_t i = 0; i < mesh->GetSubMeshes().size(); i++)
		{
			const auto materialIndex = mesh->GetSubMeshes().at(i).materialIndex;

			Ref<Material> mat = AssetManager::QueueAsset<Material>(materialTable.GetMaterial(materialIndex));
			if (!mat)
			{
				VT_LOG(Warning, "[MeshComponent]: Mesh {} has an invalid material at index {}!", mesh->assetName, materialIndex);
				mat = Renderer::GetDefaultResources().defaultMaterial;
			}

			if (static_cast<uint32_t>(component.materials.size()) > materialIndex)
			{
				if (component.materials.at(materialIndex) != mat->handle)
				{
					Ref<Material> tempMat = AssetManager::QueueAsset<Material>(component.materials.at(materialIndex));
					mat = tempMat;
				}
			}

			auto uuid = renderScene->Register(entity.GetID(), mesh, mat, static_cast<uint32_t>(i));
			component.renderObjectIds.emplace_back(uuid);
		}

		component.m_oldHandle = component.handle;
		component.m_oldMaterials = component.materials;
	}

	void MeshComponent::OnComponentDeserialized(MeshEntity entity)
	{
		auto& component = entity.GetComponent<MeshComponent>();

		component.m_oldHandle = component.handle;
		component.m_oldMaterials = component.materials;
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
