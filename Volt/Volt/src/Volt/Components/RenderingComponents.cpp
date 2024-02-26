#include "vtpch.h"
#include "RenderingComponents.h"

#include "Volt/RenderingNew/RenderScene.h"
#include "Volt/Asset/AssetManager.h"
#include "Volt/Asset/Mesh/Mesh.h"
#include "Volt/Asset/Rendering/Material.h"

#include "Volt/Components/CoreComponents.h"

namespace Volt
{
	void MeshComponent::OnMemberChanged(MeshComponent& data, Entity entity)
	{
		auto scene = entity.GetScene();
		auto renderScene = scene->GetRenderScene();

		if (data.handle != data.m_oldHandle)
		{
			if (data.m_oldHandle != Asset::Null())
			{
				for (const auto& uuid : data.renderObjectIds)
				{
					renderScene->Unregister(uuid);
				}
			}

			data.renderObjectIds.clear();

			Ref<Mesh> mesh = AssetManager::GetAsset<Mesh>(data.handle);
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
					data.renderObjectIds.emplace_back(uuid);
				}

				data.materials.clear();

				for (const auto& material : materialTable)
				{
					data.materials.emplace_back(material);
				}
			}

			data.m_oldHandle = data.handle;
			data.m_oldMaterials = data.materials;
		}
		else
		{ 
			for (uint32_t index = 0; const auto& materialHandle : data.materials)
			{
				if (materialHandle == data.m_oldMaterials.at(index))
				{
					continue;
				}

				Ref<Material> material = AssetManager::GetAsset<Material>(materialHandle);
				if (!material || !material->IsValid())
				{
					continue;
				}

				// Find meshes using material and reregister them
				Ref<Mesh> mesh = AssetManager::GetAsset<Mesh>(data.handle);
				if (mesh && mesh->IsValid())
				{
					for (uint32_t subMeshIndex = 0; const auto& subMesh : mesh->GetSubMeshes())
					{
						if (subMesh.materialIndex != index)
						{
							continue;
						}

						auto id = data.renderObjectIds.at(subMeshIndex);
						renderScene->Unregister(id);

						data.renderObjectIds.at(subMeshIndex) = renderScene->Register(entity.GetID(), mesh, material, subMeshIndex);
						subMeshIndex++;
					}
				}

				index++;
			}
		}
	}
}
