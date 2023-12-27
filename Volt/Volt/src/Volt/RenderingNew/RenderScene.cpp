#include "vtpch.h"
#include "RenderScene.h"

#include "Volt/Asset/Mesh/Mesh.h"
#include "Volt/Asset/Mesh/Material.h"
#include "Volt/Asset/Mesh/SubMaterial.h"
#include "Volt/Rendering/Texture/Texture2D.h"

#include "Volt/Scene/Entity.h"
#include "Volt/RenderingNew/GPUScene.h"
#include "Volt/RenderingNew/RendererNew.h"
#include "Volt/RenderingNew/Resources/GlobalResourceManager.h"
#include "Volt/RenderingNew/Utility/ScatteredBufferUpload.h"
#include "Volt/Rendering/Camera/Camera.h"

#include "Volt/Math/Math.h"
#include "Volt/Utility/Algorithms.h"

#include <VoltRHI/Buffers/StorageBuffer.h>
#include <VoltRHI/Buffers/CommandBuffer.h>

namespace Volt
{
	RenderScene::RenderScene(Scene* sceneRef)
		: m_scene(sceneRef)
	{
		m_gpuSceneBuffer = GlobalResource<RHI::StorageBuffer>::Create(RHI::StorageBuffer::Create(1, sizeof(GPUScene), "GPU Scene", RHI::BufferUsage::StorageBuffer, RHI::MemoryUsage::GPU));
		m_gpuMeshesBuffer = GlobalResource<RHI::StorageBuffer>::Create(RHI::StorageBuffer::Create(1, sizeof(GPUMesh), "GPU Meshes", RHI::BufferUsage::StorageBuffer, RHI::MemoryUsage::GPU));
		m_gpuMaterialsBuffer = GlobalResource<RHI::StorageBuffer>::Create(RHI::StorageBuffer::Create(1, sizeof(GPUMaterialNew), "GPU Materials", RHI::BufferUsage::StorageBuffer, RHI::MemoryUsage::GPU));
		m_objectDrawDataBuffer = GlobalResource<RHI::StorageBuffer>::Create(RHI::StorageBuffer::Create(1, sizeof(ObjectDrawData), "Object Draw Data", RHI::BufferUsage::StorageBuffer, RHI::MemoryUsage::GPU));
		m_gpuMeshletsBuffer = GlobalResource<RHI::StorageBuffer>::Create(RHI::StorageBuffer::Create(1, sizeof(Meshlet), "GPU Meshlets", RHI::BufferUsage::StorageBuffer, RHI::MemoryUsage::GPU));
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

		if (m_renderObjects.empty())
		{
			return;
		}

		std::vector<GPUMesh> gpuMeshes;
		BuildGPUMeshes(gpuMeshes);

		BuildObjectDrawData(m_objectDrawData);
		BuildMeshletBuffer(m_objectDrawData);

		UploadGPUMeshes(gpuMeshes);
		UploadObjectDrawData(m_objectDrawData);
		UploadGPUMeshlets();
		UploadGPUMaterials();
		UploadGPUScene();

		BuildMeshCommands();
	}

	void RenderScene::Update()
	{
		if (m_invalidRenderObjects.empty())
		{
			return;
		}

		//ScatteredBufferUpload<ObjectDrawData> bufferUpload{ m_objectDrawDataBuffer->GetResource() };

		//for (const auto& invalidObject : m_invalidRenderObjects)
		//{
		//}
	}

	void RenderScene::SetValid()
	{
		m_isInvalid = false;
	}

	void RenderScene::Invalidate()
	{
		m_isInvalid = true;
	}

	void RenderScene::InvalidateRenderObject(UUID64 renderObject)
	{
		auto it = std::find(m_renderObjects.begin(), m_renderObjects.end(), renderObject);
		if (it == m_renderObjects.end())
		{
			return;
		}

		m_invalidRenderObjects.emplace_back(renderObject);
	}

	const UUID64 RenderScene::Register(EntityID entityId, Ref<Mesh> mesh, uint32_t subMeshIndex)
	{
		m_isInvalid = true;

		UUID64 newId = {};
		auto& newObj = m_renderObjects.emplace_back();

		newObj.id = newId;
		newObj.entity = entityId;
		newObj.mesh = mesh;
		newObj.subMeshIndex = subMeshIndex;

		return newId;
	}

	void RenderScene::Unregister(UUID64 id)
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

	void RenderScene::UploadGPUMeshes(std::vector<GPUMesh>& gpuMeshes)
	{
		if (m_gpuMeshesBuffer->GetResource()->GetCount() < static_cast<uint32_t>(gpuMeshes.size()))
		{
			m_gpuMeshesBuffer->GetResource()->Resize(static_cast<uint32_t>(gpuMeshes.size()));
			m_gpuMeshesBuffer->MarkAsDirty();
		}
		m_gpuMeshesBuffer->GetResource()->SetData(gpuMeshes.data(), sizeof(GPUMesh) * gpuMeshes.size());
	}

	void RenderScene::UploadObjectDrawData(std::vector<ObjectDrawData>& objectDrawData)
	{
		if (m_objectDrawDataBuffer->GetResource()->GetCount() < static_cast<uint32_t>(objectDrawData.size()))
		{
			m_objectDrawDataBuffer->GetResource()->Resize(static_cast<uint32_t>(objectDrawData.size()));
			m_objectDrawDataBuffer->MarkAsDirty();
		}

		m_objectDrawDataBuffer->GetResource()->SetData(objectDrawData.data(), sizeof(ObjectDrawData) * objectDrawData.size());
	}

	void RenderScene::UploadGPUScene()
	{
		GPUScene gpuScene{};
		gpuScene.meshesBuffer = m_gpuMeshesBuffer->GetResourceHandle();
		gpuScene.materialsBuffer = m_gpuMaterialsBuffer->GetResourceHandle();
		gpuScene.objectDrawDataBuffer = m_objectDrawDataBuffer->GetResourceHandle();
		gpuScene.meshletsBuffer = m_gpuMeshletsBuffer->GetResourceHandle();

		m_gpuSceneBuffer->GetResource()->SetData(&gpuScene, sizeof(GPUScene));
	}

	void RenderScene::UploadGPUMaterials()
	{
		if (m_gpuMaterialsBuffer->GetResource()->GetCount() < static_cast<uint32_t>(m_individualMaterials.size()))
		{
			m_gpuMaterialsBuffer->GetResource()->Resize(static_cast<uint32_t>(m_individualMaterials.size()));
			m_gpuMaterialsBuffer->MarkAsDirty();
		}

		std::vector<GPUMaterialNew> gpuMaterials;

		for (const auto & material : m_individualMaterials)
		{
			GPUMaterialNew& gpuMat = gpuMaterials.emplace_back();
			gpuMat.textureCount = 0;

			for (const auto& texture : material->GetTextures())
			{
				gpuMat.textures[gpuMat.textureCount] = texture->GetResourceHandle();
				gpuMat.samplers[gpuMat.textureCount] = RendererNew::GetSampler<RHI::TextureFilter::Linear, RHI::TextureFilter::Linear, RHI::TextureFilter::Linear>()->GetResourceHandle();

				gpuMat.textureCount++;
			}
		}

		m_gpuMaterialsBuffer->GetResource()->SetData(gpuMaterials.data(), sizeof(GPUMaterialNew) * gpuMaterials.size());
	}

	void RenderScene::UploadGPUMeshlets()
	{
		if (m_gpuMeshletsBuffer->GetResource()->GetCount() < static_cast<uint32_t>(m_sceneMeshlets.size()))
		{
			m_gpuMeshletsBuffer->GetResource()->Resize(static_cast<uint32_t>(m_sceneMeshlets.size()));
			m_gpuMeshletsBuffer->MarkAsDirty();
		}	

		m_gpuMeshletsBuffer->GetResource()->SetData(m_sceneMeshlets.data(), sizeof(Meshlet) * m_sceneMeshlets.size());
	}

	void RenderScene::BuildGPUMeshes(std::vector<GPUMesh>& gpuMeshes)
	{
		for (uint32_t currentIndex = 0; const auto & mesh : m_individualMeshes)
		{
			for (uint32_t subMeshIndex = 0; const auto & gpuMesh : mesh->GetGPUMeshes())
			{
				gpuMeshes.push_back(gpuMesh);

				const size_t hash = Math::HashCombine(mesh.GetHash(), std::hash<uint32_t>()(subMeshIndex));
				m_meshSubMeshToGPUMeshIndex[hash] = currentIndex;

				currentIndex++;
				subMeshIndex++;
			}
		}
	}

	void RenderScene::BuildObjectDrawData(std::vector<ObjectDrawData>& objectDrawData)
	{
		for (const auto& renderObject : m_renderObjects)
		{
			Entity entity = m_scene->GetEntityFromUUID(renderObject.entity);
			if (!entity)
			{
				continue;
			}

			const auto& subMesh = renderObject.mesh->GetSubMeshes().at(renderObject.subMeshIndex);
			const size_t hash = Math::HashCombine(renderObject.mesh.GetHash(), std::hash<uint32_t>()(renderObject.subMeshIndex));
			const uint32_t meshId = m_meshSubMeshToGPUMeshIndex.at(hash);

			const glm::mat4 transform = entity.GetTransform() * subMesh.transform;

			BoundingSphere boundingSphere = renderObject.mesh->GetSubMeshBoundingSphere(renderObject.subMeshIndex);
			const glm::vec3 globalScale = { glm::length(transform[0]), glm::length(transform[1]), glm::length(transform[2]) };
			const float maxScale = glm::max(glm::max(globalScale.x, globalScale.y), globalScale.z);
			const glm::vec3 globalCenter = transform * glm::vec4(boundingSphere.center, 1.f);

			ObjectDrawData& data = objectDrawData.emplace_back();
			data.transform = transform;
			data.meshId = meshId;
			data.materialId = GetMaterialIndex(renderObject.mesh->GetMaterial()->GetSubMaterialAt(subMesh.materialIndex));
			data.meshletStartOffset = 0;
			data.boundingSphereCenter = globalCenter;
			data.boundingSphereRadius = boundingSphere.radius * maxScale;
		}
	}

	void RenderScene::BuildMeshCommands()
	{
		m_meshCommands.clear();

		for (uint32_t index = 0; const auto & obj : m_renderObjects)
		{
			Entity entity = m_scene->GetEntityFromUUID(obj.entity);
			if (!entity)
			{
				continue;
			}

			const auto& subMesh = obj.mesh->GetSubMeshes().at(obj.subMeshIndex);
			const auto& meshlets = obj.mesh->GetMeshlets();

			for (uint32_t meshletIdx = 0; meshletIdx < subMesh.meshletCount; meshletIdx++)
			{
				const auto& currentMeshlet = meshlets.at(subMesh.meshletStartOffset + meshletIdx);

				auto& newCommand = m_meshCommands.emplace_back();
				newCommand.command.vertexCount = currentMeshlet.triangleCount * 3;
				newCommand.command.instanceCount = 1;
				newCommand.command.firstVertex = currentMeshlet.triangleOffset;
				newCommand.command.firstInstance = 0;
				newCommand.objectId = index;
				newCommand.meshId = GetMeshID(obj.mesh, obj.subMeshIndex);
				newCommand.meshletId = subMesh.meshletStartOffset + meshletIdx;
			}

			index++;
		}

		// Mesh shader commands
		for (uint32_t index = 0; const auto & obj : m_renderObjects)
		{
			Entity entity = m_scene->GetEntityFromUUID(obj.entity);
			if (!entity)
			{
				continue;
			}

			const auto& subMesh = obj.mesh->GetSubMeshes().at(obj.subMeshIndex);
			
			auto& newCommand = m_meshShaderCommands.emplace_back();
			newCommand.command.x = subMesh.meshletCount;
			newCommand.command.y = 1;
			newCommand.command.z = 1;
			newCommand.objectId = index;
			newCommand.meshId = GetMeshID(obj.mesh, obj.subMeshIndex);

			index++;
		}
	}

	void RenderScene::BuildMeshletBuffer(std::vector<ObjectDrawData>& objectDrawData)
	{
		m_sceneMeshlets.clear();
		m_currentIndexCount = 0;

		for (uint32_t index = 0; const auto & obj : m_renderObjects)
		{
			const auto& subMesh = obj.mesh->GetSubMeshes().at(obj.subMeshIndex);
			const auto& meshlets = obj.mesh->GetMeshlets();

			const size_t meshletStartOffset = m_sceneMeshlets.size();
			const uint32_t meshId = GetMeshID(obj.mesh, obj.subMeshIndex);

			for (uint32_t meshletIdx = 0; meshletIdx < subMesh.meshletCount; meshletIdx++)
			{
				const auto& currentMeshlet = meshlets.at(subMesh.meshletStartOffset + meshletIdx);

				auto& newMeshlet = m_sceneMeshlets.emplace_back();
				newMeshlet.vertexOffset = currentMeshlet.vertexOffset;
				newMeshlet.triangleOffset = currentMeshlet.triangleOffset;
				newMeshlet.vertexCount = currentMeshlet.vertexCount;
				newMeshlet.triangleCount = currentMeshlet.triangleCount;
				newMeshlet.objectId = index;
				newMeshlet.meshId = meshId;
				newMeshlet.boundingSphereRadius = currentMeshlet.boundingSphereRadius;
				newMeshlet.boundingSphereCenter = currentMeshlet.boundingSphereCenter;
				newMeshlet.cone = currentMeshlet.cone;

				m_currentIndexCount += newMeshlet.triangleCount * 3;
			}

			objectDrawData[index].meshletStartOffset = static_cast<uint32_t>(meshletStartOffset);
			index++;
		}
	}
}
