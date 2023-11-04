#include "vtpch.h"
#include "RenderScene.h"

#include "Volt/Asset/Mesh/Mesh.h"
#include "Volt/Asset/Mesh/Material.h"

#include "Volt/Scene/Entity.h"
#include "Volt/RenderingNew/GPUScene.h"
#include "Volt/RenderingNew/Resources/GlobalResourceManager.h"

#include "Volt/Math/Math.h"

#include <VoltRHI/Buffers/StorageBuffer.h>

namespace Volt
{
	RenderScene::RenderScene(Scene* sceneRef)
		: m_scene(sceneRef)
	{
		m_gpuSceneBuffer = GlobalResource<RHI::StorageBuffer>::Create(RHI::StorageBuffer::Create(1, sizeof(GPUScene), "GPU Scene", RHI::BufferUsage::StorageBuffer, RHI::MemoryUsage::CPUToGPU));
		m_gpuMeshesBuffer = GlobalResource<RHI::StorageBuffer>::Create(RHI::StorageBuffer::Create(1, sizeof(GPUMesh), "GPU Meshes", RHI::BufferUsage::StorageBuffer, RHI::MemoryUsage::CPUToGPU));
		m_gpuMaterialsBuffer = GlobalResource<RHI::StorageBuffer>::Create(RHI::StorageBuffer::Create(1, sizeof(GPUMaterialNew), "GPU Materials", RHI::BufferUsage::StorageBuffer, RHI::MemoryUsage::CPUToGPU));
		m_objectDrawDataBuffer = GlobalResource<RHI::StorageBuffer>::Create(RHI::StorageBuffer::Create(1, sizeof(ObjectDrawData), "Object Draw Data", RHI::BufferUsage::StorageBuffer, RHI::MemoryUsage::CPUToGPU));
	}

	RenderScene::~RenderScene()
	{
	}

	void RenderScene::PrepareForUpdate()
	{
		std::sort(std::execution::par, m_renderObjects.begin(), m_renderObjects.end(), [](const auto& lhs, const auto& rhs)
		{
			if (lhs.mesh < rhs.mesh)
			{
				return true;
			}

			if (lhs.mesh > rhs.mesh)
			{
				return false;
			}

			if (lhs.subMeshIndex < rhs.subMeshIndex)
			{
				return true;
			}

			if (lhs.subMeshIndex > rhs.subMeshIndex)
			{
				return false;
			}

			return false;
		});

		m_currentIndividualMeshCount = 0;
		m_individualMeshes.clear();
		m_individualMaterials.clear();

		for (size_t i = 0; i < m_renderObjects.size(); i++)
		{
			if (i == 0)
			{
				m_currentIndividualMeshCount += static_cast<uint32_t>(m_renderObjects[i].mesh->GetSubMeshes().size());
				m_individualMeshes.push_back(m_renderObjects[i].mesh);

				for (const auto& subMaterial : m_renderObjects[i].mesh->GetMaterial()->GetSubMaterials())
				{
					const uint32_t materialIndex = GetMaterialIndex(subMaterial.second);
					if (materialIndex == std::numeric_limits<uint32_t>::max())
					{
						m_individualMaterials.push_back(subMaterial.second);
					}
				}
			}
			else
			{
				if (m_renderObjects[i].mesh != m_renderObjects[i - 1].mesh)
				{
					m_currentIndividualMeshCount += static_cast<uint32_t>(m_renderObjects[i].mesh->GetSubMeshes().size());
					m_individualMeshes.push_back(m_renderObjects[i].mesh);

					for (const auto& subMaterial : m_renderObjects[i].mesh->GetMaterial()->GetSubMaterials())
					{
						const uint32_t materialIndex = GetMaterialIndex(subMaterial.second);
						if (materialIndex == std::numeric_limits<uint32_t>::max())
						{
							m_individualMaterials.push_back(subMaterial.second);
						}
					}
				}
			}
		}

		UploadGPUMeshes();
		UploadGPUMaterials();
		UploadObjectDrawData();
		UploadGPUScene();
	}

	void RenderScene::SetValid()
	{
		m_isInvalid = false;
	}

	void RenderScene::Invalidate()
	{
		m_isInvalid = true;
	}

	const UUID RenderScene::Register(entt::entity entityId, Ref<Mesh> mesh, uint32_t subMeshIndex)
	{
		m_isInvalid = true;

		UUID newId = {};
		auto& newObj = m_renderObjects.emplace_back();

		newObj.id = newId;
		newObj.entity = entityId;
		newObj.mesh = mesh;
		newObj.subMeshIndex = subMeshIndex;

		return newId;
	}

	void RenderScene::Unregister(UUID id)
	{
		m_isInvalid = true;

		auto it = std::find_if(m_renderObjects.begin(), m_renderObjects.end(), [id](const auto& obj)
		{
			return obj.id == id;
		});

		if (it != m_renderObjects.end())
		{
			m_renderObjects.erase(it);
		}
	}

	const uint32_t RenderScene::GetMeshID(Weak<Mesh> mesh, uint32_t subMeshIndex) const
	{
		const size_t hash = Math::HashCombine(mesh.GetHash(), std::hash<uint32_t>()(subMeshIndex));
		return m_meshSubMeshToGPUMeshIndex.at(hash);
	}

	const uint32_t RenderScene::GetMaterialIndex(Ref<SubMaterial> material) const
	{
		auto it = std::find_if(m_individualMaterials.begin(), m_individualMaterials.end(), [&](Weak<SubMaterial> lhs)
		{
			return lhs.Get() == material.get();
		});

		if (it != m_individualMaterials.end())
		{
			return static_cast<uint32_t>(std::distance(m_individualMaterials.begin(), it));
		}

		return std::numeric_limits<uint32_t>::max();
	}

	void RenderScene::UploadGPUMeshes()
	{
		uint32_t gpuMeshCount = 0;
		for (const auto& mesh : m_individualMeshes)
		{
			gpuMeshCount += static_cast<uint32_t>(mesh->GetGPUMeshes().size());
		}

		if (m_gpuMeshesBuffer->GetResource()->GetSize() < gpuMeshCount)
		{
			m_gpuMeshesBuffer->GetResource()->Resize(gpuMeshCount);
			m_gpuMeshesBuffer->MarkAsDirty();
		}

		GPUMesh* gpuMeshes = m_gpuMeshesBuffer->GetResource()->Map<GPUMesh>();
		uint32_t currentIndex = 0;

		for (const auto& mesh : m_individualMeshes)
		{
			for (uint32_t subMeshIndex = 0; const auto& gpuMesh : mesh->GetGPUMeshes())
			{
				gpuMeshes[currentIndex] = gpuMesh;

				const size_t hash = Math::HashCombine(mesh.GetHash(), std::hash<uint32_t>()(subMeshIndex));
				m_meshSubMeshToGPUMeshIndex[hash] = currentIndex;

				currentIndex++;
				subMeshIndex++;
			}
		}

		m_gpuMeshesBuffer->GetResource()->Unmap();
	}

	void RenderScene::UploadObjectDrawData()
	{
		if (m_objectDrawDataBuffer->GetResource()->GetSize() < static_cast<uint32_t>(m_renderObjects.size()))
		{
			m_objectDrawDataBuffer->GetResource()->Resize(static_cast<uint32_t>(m_renderObjects.size()));
			m_objectDrawDataBuffer->MarkAsDirty();
		}

		ObjectDrawData* objectDrawDatas = m_objectDrawDataBuffer->GetResource()->Map<ObjectDrawData>();

		for (uint32_t currentIndex = 0; const auto & renderObject : m_renderObjects)
		{
			Entity entity{ renderObject.entity, m_scene };
			if (!entity)
			{
				continue;
			}

			const auto& subMesh = renderObject.mesh->GetSubMeshes().at(renderObject.subMeshIndex);
			const size_t hash = Math::HashCombine(renderObject.mesh.GetHash(), std::hash<uint32_t>()(renderObject.subMeshIndex));

			const uint32_t meshId = m_meshSubMeshToGPUMeshIndex.at(hash);

			ObjectDrawData data{};
			data.transform = entity.GetTransform() * subMesh.transform;
			data.meshId = meshId;
			data.materialId = GetMaterialIndex(renderObject.mesh->GetMaterial()->GetSubMaterialAt(subMesh.materialIndex));

			objectDrawDatas[currentIndex] = data;
			currentIndex++;
		}

		m_objectDrawDataBuffer->GetResource()->Unmap();
	}

	void RenderScene::UploadGPUScene()
	{
		GPUScene* gpuScene = m_gpuSceneBuffer->GetResource()->Map<GPUScene>();

		gpuScene->meshesBuffer = m_gpuMeshesBuffer->GetResourceHandle();
		gpuScene->materialsBuffer = m_gpuMaterialsBuffer->GetResourceHandle();
		gpuScene->objectDrawDataBuffer = m_objectDrawDataBuffer->GetResourceHandle();

		m_gpuSceneBuffer->GetResource()->Unmap();
	}

	void RenderScene::UploadGPUMaterials()
	{
		if (m_gpuMaterialsBuffer->GetResource()->GetSize() < static_cast<uint32_t>(m_individualMaterials.size()))
		{
			m_gpuMaterialsBuffer->GetResource()->Resize(static_cast<uint32_t>(m_individualMaterials.size()));
			m_gpuMaterialsBuffer->MarkAsDirty();
		}

		GPUMaterialNew* gpuMaterials = m_gpuMaterialsBuffer->GetResource()->Map<GPUMaterialNew>();

		for (uint32_t currentIndex = 0; const auto & material : m_individualMaterials)
		{
			material;

			GPUMaterialNew gpuMat{};
			gpuMat.textureCount = 0;

			gpuMaterials[currentIndex] = gpuMat;
			currentIndex++;
		}

		m_gpuMaterialsBuffer->GetResource()->Unmap();
	}
}
