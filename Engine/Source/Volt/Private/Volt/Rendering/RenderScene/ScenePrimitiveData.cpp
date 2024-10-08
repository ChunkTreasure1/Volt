#include "vtpch.h"
#include "Volt/Asset/Rendering/Material.h"
#include "Volt/Rendering/RenderScene/ScenePrimitiveData.h"

#include "Volt/Rendering/RenderScene.h"
#include "Volt/Rendering/Renderer.h"

#include <AssetSystem/AssetManager.h>

VT_DEFINE_LOG_CATEGORY(LogScenePrimitiveData);

namespace Volt
{
	ScenePrimitiveData::ScenePrimitiveData(const EntityID& relatedEntity, RenderScene* renderScene)
		: m_relatedEntity(relatedEntity),
		m_renderScene(renderScene)
	{
	}

	ScenePrimitiveData::~ScenePrimitiveData()
	{
		DestroyScenePrimitives();
	}

	void ScenePrimitiveData::InitializeFromDescription(const ScenePrimitiveDescription& description)
	{
		VT_ENSURE(m_renderScene);
		VT_ENSURE(description.primitiveMesh != Asset::Null());
	
		if (!m_renderObjects.empty())
		{
			DestroyScenePrimitives();
		}

		m_primitiveMesh = AssetManager::QueueAsset<Mesh>(description.primitiveMesh);
		VT_ENSURE(m_primitiveMesh);

		for (uint32_t index = 0; const auto& materialHandle : description.materials)
		{
			m_primitiveMaterialTable.SetMaterial(materialHandle, index);
		}

		// If the mesh isn't valid (i.e not loaded) we won't add it yet.
		if (!m_primitiveMesh->IsValid())
		{
			// Create a callback which will add the primitive to the render scene.
			m_meshChangedCallbackID = AssetManager::RegisterAssetChangedCallback(AssetTypes::Mesh, [&](AssetHandle handle, AssetChangedState state) 
			{
				if (state != AssetChangedState::Updated || handle != m_primitiveMesh->handle)
				{
					return;
				}

				CreateScenePrimitives();
				AssetManager::UnregisterAssetChangedCallback(AssetTypes::Mesh, m_meshChangedCallbackID);
			});

			return;
		}

		CreateScenePrimitives();
	}

	void ScenePrimitiveData::Invalidate()
	{
		for (const auto& id : m_renderObjects)
		{
			m_renderScene->InvalidateRenderObject(id);
		}
	}

	void ScenePrimitiveData::CreateScenePrimitives()
	{
		VT_ENSURE(m_primitiveMesh && m_primitiveMesh->IsValid());

		const auto& meshMaterialTable = m_primitiveMesh->GetMaterialTable();

		MaterialTable finalMaterialTable;
		for (uint32_t i = 0; i < meshMaterialTable.GetSize(); i++)
		{
			if (m_primitiveMaterialTable.ContainsMaterialIndex(i))
			{
				finalMaterialTable.SetMaterial(m_primitiveMaterialTable.GetMaterial(i), i);
			}
			else
			{
				finalMaterialTable.SetMaterial(meshMaterialTable.GetMaterial(i), i);
			}
		}

		const auto& subMeshes = m_primitiveMesh->GetSubMeshes();
		for (size_t i = 0; i < subMeshes.size(); i++)
		{
			const uint32_t materialIndex = subMeshes.at(i).materialIndex;
			VT_ENSURE(finalMaterialTable.ContainsMaterialIndex(materialIndex));

			Ref<Material> material = AssetManager::GetAsset<Material>(finalMaterialTable.GetMaterial(materialIndex));
			if (!material)
			{
				VT_LOGC(Warning, LogScenePrimitiveData, "Mesh {} has an invalid material at index {}! Assigning a default material.", m_primitiveMesh->assetName, materialIndex);
				material = Renderer::GetDefaultResources().defaultMaterial;
			}

			RenderObjectID renderObjectId = m_renderScene->Register(m_relatedEntity, m_primitiveMesh, material, static_cast<uint32_t>(i));
			m_renderObjects.emplace_back(renderObjectId);
		}
	}

	void ScenePrimitiveData::DestroyScenePrimitives()
	{
		VT_ENSURE(m_renderScene);

		for (const auto& id : m_renderObjects)
		{
			m_renderScene->Unregister(id);
		}

		m_renderObjects.clear();
	}
}
