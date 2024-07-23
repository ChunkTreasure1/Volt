#include "vtpch.h"
#include "RenderScene.h"

#include "Volt/Asset/Mesh/Mesh.h"
#include "Volt/Asset/Rendering/Material.h"
#include "Volt/Asset/AssetManager.h"
#include "Volt/Rendering/Texture/Texture2D.h"

#include "Volt/Scene/Entity.h"
#include "Volt/Rendering/GPUScene.h"
#include "Volt/Rendering/Renderer.h"
#include "Volt/Rendering/Utility/ScatteredBufferUpload.h"
#include "Volt/Rendering/Camera/Camera.h"

#include "Volt/Animation/MotionWeaver.h"
#include "Volt/Asset/Animation/Skeleton.h"

#include "Volt/Math/Math.h"
#include "Volt/Utility/Algorithms.h"

#include <VoltRHI/Buffers/StorageBuffer.h>
#include <VoltRHI/Buffers/CommandBuffer.h>

namespace Volt
{
	RenderScene::RenderScene(Scene* sceneRef)
		: m_scene(sceneRef)
	{
		m_gpuMeshesBuffer = BindlessResource<RHI::StorageBuffer>::CreateScope(1, sizeof(GPUMesh), "GPU Meshes", RHI::BufferUsage::StorageBuffer, RHI::MemoryUsage::GPU);
		m_gpuMaterialsBuffer = BindlessResource<RHI::StorageBuffer>::CreateScope(1, sizeof(GPUMaterial), "GPU Materials", RHI::BufferUsage::StorageBuffer, RHI::MemoryUsage::GPU);
		m_objectDrawDataBuffer = BindlessResource<RHI::StorageBuffer>::CreateScope(1, sizeof(ObjectDrawData), "Object Draw Data", RHI::BufferUsage::StorageBuffer, RHI::MemoryUsage::GPU);
		m_gpuBonesBuffer = BindlessResource<RHI::StorageBuffer>::CreateScope(1, sizeof(glm::mat4), "GPU Bones", RHI::BufferUsage::StorageBuffer, RHI::MemoryUsage::GPU);

		m_materialChangedCallbackID = AssetManager::RegisterAssetChangedCallback(AssetType::Material, [&](AssetHandle handle, AssetChangedState state) 
		{
			if (!m_materialIndexFromAssetHandle.contains(handle) || state != AssetChangedState::Updated)
			{
				return;
			}

			Ref<Material> material = AssetManager::GetAsset<Material>(handle);
			m_invalidMaterials.emplace_back(material, m_materialIndexFromAssetHandle.at(handle));
		});

		m_meshChangedCallbackID = AssetManager::RegisterAssetChangedCallback(AssetType::Mesh, [&](AssetHandle handle, AssetChangedState state)
		{
			if (state != AssetChangedState::Updated)
			{
				return;
			}

			Ref<Mesh> mesh = AssetManager::GetAsset<Mesh>(handle);
			for (uint32_t subMeshIndex = 0; subMeshIndex < static_cast<uint32_t>(mesh->GetSubMeshes().size()); subMeshIndex++)
			{
				const size_t assetHash = Math::HashCombine(handle, std::hash<uint32_t>()(subMeshIndex));
				if (m_meshIndexFromMeshAssetHash.contains(assetHash))
				{
					m_invalidMeshes.emplace_back(mesh, subMeshIndex, m_meshIndexFromMeshAssetHash.at(assetHash));
				}
			}
		});
	}

	RenderScene::~RenderScene()
	{
		AssetManager::UnregisterAssetChangedCallback(AssetType::Material, m_materialChangedCallbackID);
		AssetManager::UnregisterAssetChangedCallback(AssetType::Mesh, m_meshChangedCallbackID);
	}

	void RenderScene::PrepareForUpdate()
	{
		VT_PROFILE_FUNCTION();

		std::sort(std::execution::par, m_renderObjects.begin(), m_renderObjects.end(), [](const auto& lhs, const auto& rhs)
		{
			if (lhs.mesh.GetHash() < rhs.mesh.GetHash())
			{
				return true;
			}

			if (lhs.mesh.GetHash() > rhs.mesh.GetHash())
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
		m_currentMeshletCount = 0;
		m_individualMeshes.clear();
		m_individualMaterials.clear();
		m_materialIndexFromAssetHandle.clear();
		m_objectIndexFromRenderObjectID.clear();

		m_invalidRenderObjectIndices.clear();
		m_invalidMaterials.clear();
		m_gpuMeshes.clear();

		for (size_t i = 0; i < m_renderObjects.size(); i++)
		{
			const uint32_t materialIndex = GetMaterialIndex(m_renderObjects[i].material);
			if (materialIndex == std::numeric_limits<uint32_t>::max())
			{
				m_individualMaterials.push_back(m_renderObjects[i].material);
			}

			if (i == 0)
			{
				m_currentIndividualMeshCount += static_cast<uint32_t>(m_renderObjects[i].mesh->GetSubMeshes().size());
				m_individualMeshes.push_back(m_renderObjects[i].mesh);
			}
			else
			{
				if (m_renderObjects[i].mesh != m_renderObjects[i - 1].mesh)
				{
					m_currentIndividualMeshCount += static_cast<uint32_t>(m_renderObjects[i].mesh->GetSubMeshes().size());
					m_individualMeshes.push_back(m_renderObjects[i].mesh);
				}
			}
		}

		if (m_renderObjects.empty())
		{
			return;
		}

		BuildGPUMeshes(m_gpuMeshes);

		m_objectDrawData.clear();
		BuildObjectDrawData(m_objectDrawData);
		
		m_currentBoneCount = 0;
		for (const auto& animatedObject : m_animatedRenderObjects)
		{
			auto& objectDrawData = m_objectDrawData.at(m_objectIndexFromRenderObjectID.at(animatedObject));
			objectDrawData.boneOffset = m_currentBoneCount;
		
			const auto& renderObject = GetRenderObjectFromID(animatedObject);
			m_currentBoneCount += static_cast<uint32_t>(renderObject.motionWeaver->GetSkeleton()->GetJointCount());
		}
		
		UploadGPUMeshes(m_gpuMeshes);
		UploadObjectDrawData(m_objectDrawData);
		UploadGPUMaterials();
	}

	void RenderScene::Update(RenderGraph& renderGraph)
	{
		VT_PROFILE_FUNCTION();

		for (const auto& material : m_individualMaterials)
		{
			if (material->ClearAndGetIsDirty())
			{
				m_invalidMaterials.emplace_back(material, m_materialIndexFromAssetHandle.at(material->handle));
			}
		}

		if (!m_invalidRenderObjectIndices.empty())
		{
			ScatteredBufferUpload<ObjectDrawData> bufferUpload{ m_invalidRenderObjectIndices.size() };

			for (const auto& invalidObjectIndex : m_invalidRenderObjectIndices)
			{
				const auto& renderObject = GetRenderObjectAt(invalidObjectIndex);
				auto& data = bufferUpload.AddUploadItem(invalidObjectIndex);
				BuildSinlgeObjectDrawData(data, renderObject);
			}

			bufferUpload.UploadTo(renderGraph, *m_objectDrawDataBuffer);
			m_invalidRenderObjectIndices.clear();
		}

		if (!m_invalidMaterials.empty())
		{
			ScatteredBufferUpload<GPUMaterial> bufferUpload{ m_invalidMaterials.size() };

			for (const auto& invalidMaterial : m_invalidMaterials)
			{
				auto& data = bufferUpload.AddUploadItem(invalidMaterial.index);
				BuildGPUMaterial(invalidMaterial.material, data);
			}

			bufferUpload.UploadTo(renderGraph, *m_gpuMaterialsBuffer);
			m_invalidMaterials.clear();
		}

		if (!m_invalidMeshes.empty())
		{
			ScatteredBufferUpload<GPUMesh> bufferUpload{ m_invalidMeshes.size() };

			for (const auto& invalidMesh : m_invalidMeshes)
			{
				const uint32_t meshIndex = GetMeshIndex(invalidMesh.mesh);
				if (meshIndex == std::numeric_limits<uint32_t>::max())
				{
					continue;
				}

				auto& data = bufferUpload.AddUploadItem(invalidMesh.index);

				for (uint32_t subMeshIndex = 0; const auto & gpuMesh : invalidMesh.mesh->GetGPUMeshes())
				{
					data = gpuMesh;

					const size_t hash = Math::HashCombine(invalidMesh.mesh.GetHash(), std::hash<uint32_t>()(subMeshIndex));

					m_meshSubMeshToGPUMeshIndex[hash] = meshIndex;

					const size_t assetHash = Math::HashCombine(invalidMesh.mesh->handle, std::hash<uint32_t>()(subMeshIndex));
					m_meshIndexFromMeshAssetHash[assetHash] = meshIndex;

					subMeshIndex++;
				}
			}
		}

		// Temporary animation sampling
		m_animationBufferStorage.resize(m_currentBoneCount);
		for (const auto& animatedObject : m_animatedRenderObjects)
		{
			const auto& objectDrawData = m_objectDrawData.at(m_objectIndexFromRenderObjectID.at(animatedObject));
			const auto& renderObject = GetRenderObjectFromID(animatedObject);
		
			const auto sample = renderObject.motionWeaver->Sample();
			memcpy_s(m_animationBufferStorage.data() + objectDrawData.boneOffset, sizeof(glm::mat4) * sample.size(), sample.data(), sizeof(glm::mat4) * sample.size());
		}

		if (m_currentBoneCount > 0)
		{
			if (m_gpuBonesBuffer->GetResource()->GetCount() < m_animationBufferStorage.size())
			{
				m_gpuBonesBuffer->GetResource()->ResizeWithCount(static_cast<uint32_t>(m_animationBufferStorage.size()));
				m_gpuBonesBuffer->MarkAsDirty();
			}

			m_gpuBonesBuffer->GetResource()->SetData(m_animationBufferStorage.data(), m_animationBufferStorage.size() * sizeof(glm::mat4));
			m_animationBufferStorage.clear();
		}
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

		m_invalidRenderObjectIndices.emplace_back(std::distance(m_renderObjects.begin(), it));
	}

	const UUID64 RenderScene::Register(EntityID entityId, Ref<Mesh> mesh, Ref<Material> material, uint32_t subMeshIndex)
	{
		m_isInvalid = true;

		UUID64 newId = {};
		auto& newObj = m_renderObjects.emplace_back();

		newObj.id = newId;
		newObj.entity = entityId;
		newObj.mesh = mesh;
		newObj.material = material;
		newObj.subMeshIndex = subMeshIndex;

		return newId;
	}

	const UUID64 RenderScene::Register(EntityID entityId, Ref<MotionWeaver> motionWeaver, Ref<Mesh> mesh, Ref<Material> material, uint32_t subMeshIndex)
	{
		m_isInvalid = true;

		UUID64 newId = {};
		auto& newObj = m_renderObjects.emplace_back();
		m_animatedRenderObjects.emplace_back(newId);

		newObj.id = newId;
		newObj.entity = entityId;
		newObj.mesh = mesh;
		newObj.material = material;
		newObj.subMeshIndex = subMeshIndex;
		newObj.motionWeaver = motionWeaver;

		return newId;
	}

	void RenderScene::Unregister(UUID64 id)
	{
		m_isInvalid = true;

		auto it = std::find_if(m_renderObjects.begin(), m_renderObjects.end(), [id](const auto& obj)
		{
			return obj.id == id;
		});

		const bool isAnimated = (*it).IsAnimated();

		if (it != m_renderObjects.end())
		{
			m_renderObjects.erase(it);
		}

		if (isAnimated)
		{
			auto animIt = std::find_if(m_animatedRenderObjects.begin(), m_animatedRenderObjects.end(), [id](const auto& obj)
			{
				return obj == id;
			});

			if (animIt != m_animatedRenderObjects.end())
			{
				m_animatedRenderObjects.erase(animIt);
			}
		}
	}

	Weak<Material> RenderScene::GetMaterialFromID(const uint32_t materialId) const
	{
		if (static_cast<size_t>(materialId) >= m_individualMaterials.size())
		{
			return {};
		}

		return m_individualMaterials.at(materialId);
	}

	const uint32_t RenderScene::GetMeshID(Weak<Mesh> mesh, uint32_t subMeshIndex) const
	{
		const size_t hash = Math::HashCombine(mesh.GetHash(), std::hash<uint32_t>()(subMeshIndex));
		return m_meshSubMeshToGPUMeshIndex.contains(hash) ? m_meshSubMeshToGPUMeshIndex.at(hash) : std::numeric_limits<uint32_t>::max();
	}

	const uint32_t RenderScene::GetMaterialIndex(Weak<Material> material) const
	{
		auto it = std::find_if(m_individualMaterials.begin(), m_individualMaterials.end(), [&](Weak<Material> lhs)
		{
			return lhs.Get() == material.Get();
		});

		if (it != m_individualMaterials.end())
		{
			return static_cast<uint32_t>(std::distance(m_individualMaterials.begin(), it));
		}

		return std::numeric_limits<uint32_t>::max();
	}

	const uint32_t RenderScene::GetMeshIndex(Weak<Mesh> mesh) const
	{
		auto it = std::find_if(m_individualMeshes.begin(), m_individualMeshes.end(), [&](Weak<Mesh> lhs)
		{
			return lhs.Get() == mesh.Get();
		});

		if (it != m_individualMeshes.end())
		{
			return static_cast<uint32_t>(std::distance(m_individualMeshes.begin(), it));
		}

		return std::numeric_limits<uint32_t>::max();
	}

	const GPUSceneBuffers RenderScene::GetGPUSceneBuffers() const
	{
		GPUSceneBuffers gpuScene{};
		gpuScene.meshesBuffer = GetGPUMeshesBuffer().GetResource();
		gpuScene.materialsBuffer = GetGPUMaterialsBuffer().GetResource();
		gpuScene.objectDrawDataBuffer = GetObjectDrawDataBuffer().GetResource();
		gpuScene.bonesBuffer = m_gpuBonesBuffer->GetResource();
		return gpuScene;
	}

	const RenderObject& RenderScene::GetRenderObjectFromID(UUID64 id) const
	{
		auto it = std::find_if(m_renderObjects.begin(), m_renderObjects.end(), [id](const auto& renderObject)
		{
			return renderObject.id == id;
		});

		if (it == m_renderObjects.end())
		{
			static RenderObject nullObject;
			return nullObject;
		}

		return *it;
	}

	void RenderScene::UploadGPUMeshes(Vector<GPUMesh>& gpuMeshes)
	{
		if (m_gpuMeshesBuffer->GetResource()->GetCount() < static_cast<uint32_t>(gpuMeshes.size()))
		{
			m_gpuMeshesBuffer->GetResource()->ResizeWithCount(static_cast<uint32_t>(gpuMeshes.size()));
			m_gpuMeshesBuffer->MarkAsDirty();
		}
		m_gpuMeshesBuffer->GetResource()->SetData(gpuMeshes.data(), sizeof(GPUMesh) * gpuMeshes.size());
	}

	void RenderScene::UploadObjectDrawData(Vector<ObjectDrawData>& objectDrawData)
	{
		if (m_objectDrawDataBuffer->GetResource()->GetCount() < static_cast<uint32_t>(objectDrawData.size()))
		{
			m_objectDrawDataBuffer->GetResource()->ResizeWithCount(static_cast<uint32_t>(objectDrawData.size()));
			m_objectDrawDataBuffer->MarkAsDirty();
		}

		m_objectDrawDataBuffer->GetResource()->SetData(objectDrawData.data(), sizeof(ObjectDrawData) * objectDrawData.size());
	}

	void RenderScene::UploadGPUMaterials()
	{
		if (m_gpuMaterialsBuffer->GetResource()->GetCount() < static_cast<uint32_t>(m_individualMaterials.size()))
		{
			m_gpuMaterialsBuffer->GetResource()->ResizeWithCount(static_cast<uint32_t>(m_individualMaterials.size()));
			m_gpuMaterialsBuffer->MarkAsDirty();
		}

		Vector<GPUMaterial> gpuMaterials;

		for (const auto& material : m_individualMaterials)
		{
			m_materialIndexFromAssetHandle[material->handle] = gpuMaterials.size();

			GPUMaterial& gpuMat = gpuMaterials.emplace_back();
			BuildGPUMaterial(material, gpuMat);
		}

		m_gpuMaterialsBuffer->GetResource()->SetData(gpuMaterials.data(), sizeof(GPUMaterial) * gpuMaterials.size());
	}

	void RenderScene::BuildGPUMaterial(Weak<Material> material, GPUMaterial& gpuMaterial)
	{
		gpuMaterial.textureCount = 0;
		if (!material->IsValid())
		{
			material = Renderer::GetDefaultResources().defaultMaterial;
		}

		for (const auto& texture : material->GetTextures())
		{
			ResourceHandle textureHandle = ResourceHandle(0u);
			if (texture->IsValid())
			{
				textureHandle = texture->GetResourceHandle();
			}

			gpuMaterial.textures[gpuMaterial.textureCount] = textureHandle;
			gpuMaterial.samplers[gpuMaterial.textureCount] = Renderer::GetSampler<RHI::TextureFilter::Linear, RHI::TextureFilter::Linear, RHI::TextureFilter::Linear, RHI::TextureWrap::Repeat, RHI::AnisotropyLevel::X16>()->GetResourceHandle();
			gpuMaterial.textureCount++;
		}
	}

	void RenderScene::BuildGPUMeshes(Vector<GPUMesh>& gpuMeshes)
	{
		for (uint32_t currentIndex = 0; const auto & mesh : m_individualMeshes)
		{
			for (uint32_t subMeshIndex = 0; const auto & gpuMesh : mesh->GetGPUMeshes())
			{
				gpuMeshes.push_back(gpuMesh);

				const size_t hash = Math::HashCombine(mesh.GetHash(), std::hash<uint32_t>()(subMeshIndex));
				m_meshSubMeshToGPUMeshIndex[hash] = currentIndex;

				const size_t assetHash = Math::HashCombine(mesh->handle, std::hash<uint32_t>()(subMeshIndex));
				m_meshIndexFromMeshAssetHash[assetHash] = currentIndex;

				currentIndex++;
				subMeshIndex++;
			}
		}
	}

	void RenderScene::BuildObjectDrawData(Vector<ObjectDrawData>& objectDrawData)
	{
		m_objectIndexFromRenderObjectID.clear();

		for (const auto& renderObject : m_renderObjects)
		{
			BuildSinlgeObjectDrawData(objectDrawData.emplace_back(), renderObject);
			m_objectIndexFromRenderObjectID[renderObject.id] = static_cast<uint32_t>(objectDrawData.size() - 1);
		}
	}

	void RenderScene::BuildSinlgeObjectDrawData(ObjectDrawData& objectDrawData, const RenderObject& renderObject)
	{
		Entity entity = m_scene->GetEntityFromUUID(renderObject.entity);
		if (!entity)
		{
			return;
		}

		const size_t hash = Math::HashCombine(renderObject.mesh.GetHash(), std::hash<uint32_t>()(renderObject.subMeshIndex));
		const uint32_t meshId = m_meshSubMeshToGPUMeshIndex.contains(hash) ? m_meshSubMeshToGPUMeshIndex.at(hash) : std::numeric_limits<uint32_t>::max();

		objectDrawData.position = entity.GetPosition();
		objectDrawData.scale = entity.GetScale();
		objectDrawData.rotation = entity.GetRotation();
		objectDrawData.meshId = meshId;
		objectDrawData.entityId = entity.GetID();
		objectDrawData.materialId = GetMaterialIndex(renderObject.material);
		objectDrawData.meshletStartOffset = renderObject.meshletStartOffset;
		objectDrawData.isAnimated = renderObject.IsAnimated();

		m_currentMeshletCount += m_gpuMeshes.at(meshId).meshletCount;
	}
}
