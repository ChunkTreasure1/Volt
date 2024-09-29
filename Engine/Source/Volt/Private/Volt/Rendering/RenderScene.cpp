#include "vtpch.h"
#include "Volt/Rendering/RenderScene.h"

#include "Volt/Asset/Mesh/Mesh.h"
#include "Volt/Asset/Rendering/Material.h"
#include <AssetSystem/AssetManager.h>
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

#include <RHIModule/Buffers/StorageBuffer.h>
#include <RHIModule/Buffers/CommandBuffer.h>

namespace Volt
{
	RenderScene::RenderScene(Scene* sceneRef)
		: m_scene(sceneRef)
	{
		m_buffers.meshesBuffer = BindlessResource<RHI::StorageBuffer>::CreateScope(5, sizeof(GPUMesh), "GPU Meshes", RHI::BufferUsage::StorageBuffer, RHI::MemoryUsage::GPU);
		m_buffers.sdfMeshesBuffer = BindlessResource<RHI::StorageBuffer>::CreateScope(5, sizeof(GPUMeshSDF), "SDF GPU Meshes", RHI::BufferUsage::StorageBuffer, RHI::MemoryUsage::GPU);
		m_buffers.materialsBuffer = BindlessResource<RHI::StorageBuffer>::CreateScope(5, sizeof(GPUMaterial), "GPU Materials", RHI::BufferUsage::StorageBuffer, RHI::MemoryUsage::GPU);
		m_buffers.primitiveDrawDataBuffer = BindlessResource<RHI::StorageBuffer>::CreateScope(5, sizeof(PrimitiveDrawData), "Primitive Draw Data", RHI::BufferUsage::StorageBuffer, RHI::MemoryUsage::GPU);
		m_buffers.sdfPrimitiveDrawDataBuffer = BindlessResource<RHI::StorageBuffer>::CreateScope(5, sizeof(SDFPrimitiveDrawData), "SDF Primitive Draw Data", RHI::BufferUsage::StorageBuffer, RHI::MemoryUsage::GPU);
		m_buffers.bonesBuffer = BindlessResource<RHI::StorageBuffer>::CreateScope(1, sizeof(glm::mat4), "GPU Bones", RHI::BufferUsage::StorageBuffer, RHI::MemoryUsage::GPU);

		// Setup invalid mesh
		{
			GPUMesh& mesh = m_gpuMeshes.emplace_back();
			memset(&mesh, 0, sizeof(GPUMesh));
		}

		m_materialChangedCallbackID = AssetManager::RegisterAssetChangedCallback(AssetTypes::Material, [&](AssetHandle handle, AssetChangedState state)
		{
			if (!m_materialIndexFromAssetHandle.contains(handle) || state != AssetChangedState::Updated)
			{
				return;
			}

			Ref<Material> material = AssetManager::GetAsset<Material>(handle);
			m_invalidMaterials.emplace_back(material, m_materialIndexFromAssetHandle.at(handle));
		});

		m_meshChangedCallbackID = AssetManager::RegisterAssetChangedCallback(AssetTypes::Mesh, [&](AssetHandle handle, AssetChangedState state)
		{
			if (state != AssetChangedState::Updated)
			{
				return;
			}

			Ref<Mesh> mesh = AssetManager::GetAsset<Mesh>(handle);
			for (uint32_t subMeshIndex = 0; subMeshIndex < static_cast<uint32_t>(mesh->GetSubMeshes().size()); subMeshIndex++)
			{
				const size_t assetHash = Math::HashCombine(handle, std::hash<uint32_t>()(subMeshIndex));
				if (m_gpuMeshIndexFromMeshAssetHash.contains(assetHash))
				{
					m_invalidMeshes.emplace_back(mesh, subMeshIndex, m_gpuMeshIndexFromMeshAssetHash.at(assetHash));
				}
			}
		});
	}

	RenderScene::~RenderScene()
	{
		AssetManager::UnregisterAssetChangedCallback(AssetTypes::Material, m_materialChangedCallbackID);
		AssetManager::UnregisterAssetChangedCallback(AssetTypes::Mesh, m_meshChangedCallbackID);
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

		m_invalidPrimitiveDataIndices.clear();
		m_invalidMaterials.clear();

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

		m_gpuMeshes = BuildGPUMeshes();
		m_gpuSDFMeshes = BuildGPUMeshSDFs();
		m_primitiveDrawData = BuildPrimitiveDrawData();
		m_sdfPrimitiveDrawData = BuildSDFPrimitiveDrawData();

		UploadGPUMeshes(m_gpuMeshes);
		UploadGPUMeshSDFs(m_gpuSDFMeshes);
		UploadPrimitiveDrawData(m_primitiveDrawData);
		UploadSDFPrimitiveDrawData(m_sdfPrimitiveDrawData);
		UploadGPUMaterials();
	}

	void RenderScene::Update(RenderGraph& renderGraph)
	{
		VT_PROFILE_FUNCTION();

		UpdateInvalidMaterials(renderGraph);
		UpdateInvalidMeshes(renderGraph);
		UpdateInvalidPrimitiveData(renderGraph);

		// Temporary animation sampling
		m_currentBoneCount = 0;
		for (const auto& animatedObject : m_animatedRenderObjects)
		{
			auto& primitiveDrawData = m_primitiveDrawData.at(m_primitiveIndexFromRenderObjectID.at(animatedObject));
			primitiveDrawData.boneOffset = m_currentBoneCount;

			const auto& renderObject = GetRenderObjectFromID(animatedObject);
			m_currentBoneCount += static_cast<uint32_t>(renderObject.motionWeaver->GetSkeleton()->GetJointCount());
		}

		m_animationBufferStorage.resize(m_currentBoneCount);
		if (m_currentBoneCount > 0)
		{
			for (const auto& animatedObject : m_animatedRenderObjects)
			{
				const auto& primitiveDrawData = m_primitiveDrawData.at(m_primitiveIndexFromRenderObjectID.at(animatedObject));
				const auto& renderObject = GetRenderObjectFromID(animatedObject);

				const auto sample = renderObject.motionWeaver->Sample();
				memcpy_s(m_animationBufferStorage.data() + primitiveDrawData.boneOffset, sizeof(glm::mat4) * sample.size(), sample.data(), sizeof(glm::mat4) * sample.size());
			}
		}

		if (m_currentBoneCount > 0)
		{
			auto bonesBuffer = m_buffers.bonesBuffer;

			if (bonesBuffer->GetResource()->GetCount() < m_animationBufferStorage.size())
			{
				bonesBuffer->GetResource()->ResizeWithCount(static_cast<uint32_t>(m_animationBufferStorage.size()));
				bonesBuffer->MarkAsDirty();
			}

			bonesBuffer->GetResource()->SetData(m_animationBufferStorage.data(), m_animationBufferStorage.size() * sizeof(glm::mat4));
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
		if (m_primitiveIndexFromRenderObjectID.contains(renderObject))
		{
			m_invalidPrimitiveDataIndices.emplace_back(renderObject, m_primitiveIndexFromRenderObjectID.at(renderObject));
		}

		if (m_sdfPrimitiveIndexFromRenderObjectID.contains(renderObject))
		{
			m_invalidSDFPrimitiveDataIndices.emplace_back(renderObject, m_sdfPrimitiveIndexFromRenderObjectID.at(renderObject));
		}
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

		TryAddMaterial(material);
		TryAddMesh(mesh);

		BuildSinglePrimitiveDrawData(m_primitiveDrawData.emplace_back(), newObj);
		m_primitiveIndexFromRenderObjectID[newObj.id] = static_cast<uint32_t>(m_primitiveDrawData.size() - 1);

		InvalidateRenderObject(newId);

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

		TryAddMaterial(material);
		TryAddMesh(mesh);

		BuildSinglePrimitiveDrawData(m_primitiveDrawData.emplace_back(), newObj);
		m_primitiveIndexFromRenderObjectID[newObj.id] = static_cast<uint32_t>(m_primitiveDrawData.size() - 1);

		InvalidateRenderObject(newId);

		return newId;
	}

	void RenderScene::Unregister(UUID64 id)
	{
		m_isInvalid = true;

		auto it = std::find_if(m_renderObjects.begin(), m_renderObjects.end(), [id](const auto& obj)
		{
			return obj.id == id;
		});

		if (it == m_renderObjects.end())
		{
			return;
		}

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

	void RenderScene::UploadGPUMeshes(const Vector<GPUMesh>& gpuMeshes)
	{
		auto meshesBuffer = m_buffers.meshesBuffer;

		if (meshesBuffer->GetResource()->GetCount() < static_cast<uint32_t>(gpuMeshes.size()))
		{
			meshesBuffer->GetResource()->ResizeWithCount(static_cast<uint32_t>(gpuMeshes.size()));
			meshesBuffer->MarkAsDirty();
		}
		meshesBuffer->GetResource()->SetData(gpuMeshes.data(), sizeof(GPUMesh) * gpuMeshes.size());
	}

	void RenderScene::UploadGPUMeshSDFs(const Vector<GPUMeshSDF>& sdfMeshes)
	{
		auto sdfMeshesBuffer = m_buffers.sdfMeshesBuffer;

		if (sdfMeshesBuffer->GetResource()->GetCount() < static_cast<uint32_t>(sdfMeshes.size()))
		{
			sdfMeshesBuffer->GetResource()->ResizeWithCount(static_cast<uint32_t>(sdfMeshes.size()));
			sdfMeshesBuffer->MarkAsDirty();
		}
		sdfMeshesBuffer->GetResource()->SetData(sdfMeshes.data(), sizeof(GPUMeshSDF) * sdfMeshes.size());
	}

	void RenderScene::UploadPrimitiveDrawData(const Vector<PrimitiveDrawData>& primitiveDrawData)
	{
		auto drawDataBuffer = m_buffers.primitiveDrawDataBuffer;

		if (drawDataBuffer->GetResource()->GetCount() < static_cast<uint32_t>(primitiveDrawData.size()))
		{
			drawDataBuffer->GetResource()->ResizeWithCount(static_cast<uint32_t>(primitiveDrawData.size()));
			drawDataBuffer->MarkAsDirty();
		}

		drawDataBuffer->GetResource()->SetData(primitiveDrawData.data(), sizeof(PrimitiveDrawData) * primitiveDrawData.size());
	}

	void RenderScene::UploadSDFPrimitiveDrawData(const Vector<SDFPrimitiveDrawData>& primitiveDrawData)
	{
		auto drawDataBuffer = m_buffers.sdfPrimitiveDrawDataBuffer;

		if (drawDataBuffer->GetResource()->GetCount() < static_cast<uint32_t>(primitiveDrawData.size()))
		{
			drawDataBuffer->GetResource()->ResizeWithCount(static_cast<uint32_t>(primitiveDrawData.size()));
			drawDataBuffer->MarkAsDirty();
		}

		drawDataBuffer->GetResource()->SetData(primitiveDrawData.data(), sizeof(SDFPrimitiveDrawData) * primitiveDrawData.size());
	}

	void RenderScene::UploadGPUMaterials()
	{
		auto materialsBuffer = m_buffers.materialsBuffer;

		if (materialsBuffer->GetResource()->GetCount() < static_cast<uint32_t>(m_individualMaterials.size()))
		{
			materialsBuffer->GetResource()->ResizeWithCount(static_cast<uint32_t>(m_individualMaterials.size()));
			materialsBuffer->MarkAsDirty();
		}

		Vector<GPUMaterial> gpuMaterials;

		for (const auto& material : m_individualMaterials)
		{
			m_materialIndexFromAssetHandle[material->handle] = gpuMaterials.size();

			GPUMaterial& gpuMat = gpuMaterials.emplace_back();
			BuildGPUMaterial(material, gpuMat);
		}

		materialsBuffer->GetResource()->SetData(gpuMaterials.data(), sizeof(GPUMaterial) * gpuMaterials.size());
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

	Vector<GPUMesh> RenderScene::BuildGPUMeshes()
	{
		Vector<GPUMesh> result;
		result.reserve(m_currentIndividualMeshCount);

		// Setup invalid mesh
		{
			GPUMesh& mesh = result.emplace_back();
			memset(&mesh, 0, sizeof(GPUMesh));
		}

		for (uint32_t currentIndex = 1; const auto & mesh : m_individualMeshes)
		{
			for (uint32_t subMeshIndex = 0; const auto & gpuMesh : mesh->GetGPUMeshes())
			{
				result.push_back(gpuMesh);

				const size_t hash = Math::HashCombine(mesh.GetHash(), std::hash<uint32_t>()(subMeshIndex));
				m_meshSubMeshToGPUMeshIndex[hash] = currentIndex;

				const size_t assetHash = Math::HashCombine(mesh->handle, std::hash<uint32_t>()(subMeshIndex));
				m_gpuMeshIndexFromMeshAssetHash[assetHash] = currentIndex;

				currentIndex++;
				subMeshIndex++;
			}
		}

		return result;
	}

	Vector<GPUMeshSDF> RenderScene::BuildGPUMeshSDFs()
	{
		Vector<GPUMeshSDF> result;
		result.reserve(m_currentIndividualMeshCount);

		// Setup invalid mesh
		{
			GPUMeshSDF& mesh = result.emplace_back();
			memset(&mesh, 0, sizeof(GPUMeshSDF));
		}

		for (uint32_t currentIndex = 1; const auto & mesh : m_individualMeshes)
		{
			for (uint32_t subMeshIndex = 0; const auto & gpuMeshSDF : mesh->GetGPUMeshSDFs())
			{
				result.emplace_back(gpuMeshSDF);

				const size_t hash = Math::HashCombine(mesh.GetHash(), std::hash<uint32_t>()(subMeshIndex));
				m_meshSubMeshToGPUMeshSDFIndex[hash] = currentIndex;

				currentIndex++;
				subMeshIndex++;
			}
		}

		return result;
	}

	Vector<PrimitiveDrawData> RenderScene::BuildPrimitiveDrawData()
	{
		m_primitiveIndexFromRenderObjectID.clear();

		Vector<PrimitiveDrawData> result;
		result.reserve(m_renderObjects.size());

		for (const auto& renderObject : m_renderObjects)
		{
			BuildSinglePrimitiveDrawData(result.emplace_back(), renderObject);
			m_primitiveIndexFromRenderObjectID[renderObject.id] = static_cast<uint32_t>(result.size() - 1);
		}

		return result;
	}

	void RenderScene::BuildSinglePrimitiveDrawData(PrimitiveDrawData& primitiveDrawData, const RenderObject& renderObject)
	{
		Entity entity = m_scene->GetEntityFromID(renderObject.entity);
		if (!entity)
		{
			return;
		}

		const size_t hash = Math::HashCombine(renderObject.mesh.GetHash(), std::hash<uint32_t>()(renderObject.subMeshIndex));
		const uint32_t meshId = m_meshSubMeshToGPUMeshIndex.contains(hash) ? m_meshSubMeshToGPUMeshIndex.at(hash) : 0;

		primitiveDrawData.position = entity.GetPosition();
		primitiveDrawData.scale = entity.GetScale();
		primitiveDrawData.rotation = entity.GetRotation();
		primitiveDrawData.meshId = meshId;
		primitiveDrawData.entityId = entity.GetID();
		primitiveDrawData.materialId = GetMaterialIndex(renderObject.material);
		primitiveDrawData.meshletStartOffset = renderObject.meshletStartOffset;
		primitiveDrawData.isAnimated = renderObject.IsAnimated();

		m_currentMeshletCount += m_gpuMeshes.at(meshId).meshletCount;
	}

	Vector<SDFPrimitiveDrawData> RenderScene::BuildSDFPrimitiveDrawData()
	{
		m_sdfPrimitiveIndexFromRenderObjectID.clear();

		Vector<SDFPrimitiveDrawData> result;
		result.reserve(m_renderObjects.size());

		for (const auto& renderObject : m_renderObjects)
		{
			BuildSingleSDFPrimitiveDrawData(result.emplace_back(), renderObject);
			m_sdfPrimitiveIndexFromRenderObjectID[renderObject.id] = static_cast<uint32_t>(result.size() - 1);
		}

		return result;
	}

	void RenderScene::BuildSingleSDFPrimitiveDrawData(SDFPrimitiveDrawData& primtiveDrawData, const RenderObject& renderObject)
	{
		Entity entity = m_scene->GetEntityFromID(renderObject.entity);
		if (!entity)
		{
			return;
		}

		const size_t hash = Math::HashCombine(renderObject.mesh.GetHash(), std::hash<uint32_t>()(renderObject.subMeshIndex));
		const uint32_t meshId = m_meshSubMeshToGPUMeshSDFIndex.contains(hash) ? m_meshSubMeshToGPUMeshSDFIndex.at(hash) : 0;

		primtiveDrawData.position = entity.GetPosition();
		primtiveDrawData.scale = entity.GetScale();
		primtiveDrawData.rotation = entity.GetRotation();
		primtiveDrawData.primtiveId = m_primitiveIndexFromRenderObjectID.at(renderObject.id);
		primtiveDrawData.meshSDFId = meshId;
	}

	void RenderScene::TryAddMesh(Ref<Mesh> mesh)
	{
		VT_PROFILE_FUNCTION();

		auto it = std::find(m_individualMeshes.begin(), m_individualMeshes.end(), mesh);
		if (it != m_individualMeshes.end())
		{
			return;
		}

		m_individualMeshes.emplace_back(mesh);
		m_currentIndividualMeshCount += static_cast<uint32_t>(mesh->GetSubMeshes().size());

		size_t currentIndex = m_gpuMeshes.size();

		for (uint32_t subMeshIndex = 0; const auto & gpuMesh : mesh->GetGPUMeshes())
		{
			m_gpuMeshes.emplace_back(gpuMesh);

			const size_t hash = Math::HashCombine(std::hash<void*>()(mesh.get()), std::hash<uint32_t>()(subMeshIndex));
			m_meshSubMeshToGPUMeshIndex[hash] = static_cast<uint32_t>(currentIndex);

			const size_t assetHash = Math::HashCombine(mesh->handle, std::hash<uint32_t>()(subMeshIndex));
			m_gpuMeshIndexFromMeshAssetHash[assetHash] = currentIndex;

			m_invalidMeshes.emplace_back(mesh, subMeshIndex, m_gpuMeshIndexFromMeshAssetHash.at(assetHash));

			currentIndex++;
			subMeshIndex++;
		}
	}

	void RenderScene::TryAddMaterial(Ref<Material> material)
	{
		VT_PROFILE_FUNCTION();

		auto it = std::find(m_individualMaterials.begin(), m_individualMaterials.end(), material);
		if (it != m_individualMaterials.end())
		{
			return;
		}

		m_individualMaterials.emplace_back(material);
		m_materialIndexFromAssetHandle[material->handle] = m_gpuMaterials.size();

		GPUMaterial& gpuMaterial = m_gpuMaterials.emplace_back();
		BuildGPUMaterial(material, gpuMaterial);

		m_invalidMaterials.emplace_back(material, m_materialIndexFromAssetHandle[material->handle]);
	}	

	void RenderScene::UpdateInvalidMaterials(RenderGraph& renderGraph)
	{
		VT_PROFILE_FUNCTION();

		auto materialsBuffer = m_buffers.materialsBuffer;

		if (materialsBuffer->GetResource()->GetCount() < static_cast<uint32_t>(m_individualMaterials.size()))
		{
			materialsBuffer->GetResource()->ResizeWithCount(static_cast<uint32_t>(m_individualMaterials.size()));
			materialsBuffer->MarkAsDirty();
		}

		for (const auto& material : m_individualMaterials)
		{
			if (material->ClearAndGetIsDirty())
			{
				m_invalidMaterials.emplace_back(material, m_materialIndexFromAssetHandle.at(material->handle));
			}
		}

		if (!m_invalidMaterials.empty())
		{
			ScatteredBufferUpload<GPUMaterial> bufferUpload{ m_invalidMaterials.size() };

			for (const auto& invalidMaterial : m_invalidMaterials) 
			{
				auto& data = bufferUpload.AddUploadItem(invalidMaterial.index);
				BuildGPUMaterial(invalidMaterial.material, data);
			}

			bufferUpload.UploadTo(renderGraph, *materialsBuffer);
			m_invalidMaterials.clear();
		}
	}

	void RenderScene::UpdateInvalidMeshes(RenderGraph& renderGraph)
	{
		VT_PROFILE_FUNCTION();

		auto meshesBuffer = m_buffers.meshesBuffer;

		if (meshesBuffer->GetResource()->GetCount() < static_cast<uint32_t>(m_gpuMeshes.size()))
		{
			meshesBuffer->GetResource()->ResizeWithCount(static_cast<uint32_t>(m_gpuMeshes.size()));
			meshesBuffer->MarkAsDirty();
		}

		if (!m_invalidMeshes.empty())
		{
			ScatteredBufferUpload<GPUMesh> bufferUpload{ m_invalidMeshes.size() };

			for (const auto& invalidMesh : m_invalidMeshes)
			{
				auto& data = bufferUpload.AddUploadItem(invalidMesh.index);
				data = invalidMesh.mesh->GetGPUMeshes().at(invalidMesh.subMeshIndex);

				m_gpuMeshes[invalidMesh.index] = data;
			}

			bufferUpload.UploadTo(renderGraph, *meshesBuffer);
			m_invalidMeshes.clear();
		}
	}

	void RenderScene::UpdateInvalidPrimitiveData(RenderGraph& renderGraph)
	{
		VT_PROFILE_FUNCTION();

		auto drawDataBuffer = m_buffers.primitiveDrawDataBuffer;

		if (drawDataBuffer->GetResource()->GetCount() < static_cast<uint32_t>(m_primitiveDrawData.size()))
		{
			drawDataBuffer->GetResource()->ResizeWithCount(static_cast<uint32_t>(m_primitiveDrawData.size()));
			drawDataBuffer->MarkAsDirty();
		}

		if (!m_invalidPrimitiveDataIndices.empty())
		{
			ScatteredBufferUpload<PrimitiveDrawData> bufferUpload{ m_invalidPrimitiveDataIndices.size() };

			for (const auto& invalidPrimitive : m_invalidPrimitiveDataIndices)
			{
				const auto& renderObject = GetRenderObjectFromID(invalidPrimitive.renderObjectId);
				auto& data = bufferUpload.AddUploadItem(invalidPrimitive.index);
				BuildSinglePrimitiveDrawData(data, renderObject);
			}

			bufferUpload.UploadTo(renderGraph, *drawDataBuffer);
			m_invalidPrimitiveDataIndices.clear();
		}

		if (!m_invalidSDFPrimitiveDataIndices.empty())
		{
			ScatteredBufferUpload<SDFPrimitiveDrawData> bufferUpload{ m_invalidSDFPrimitiveDataIndices.size() };

			for (const auto& invalidPrimitive : m_invalidSDFPrimitiveDataIndices)
			{
				const auto& renderObject = GetRenderObjectFromID(invalidPrimitive.renderObjectId);
				auto& data = bufferUpload.AddUploadItem(invalidPrimitive.index);
				BuildSingleSDFPrimitiveDrawData(data, renderObject);
			}

			bufferUpload.UploadTo(renderGraph, *m_buffers.sdfPrimitiveDrawDataBuffer);
			m_invalidSDFPrimitiveDataIndices.clear();
		}
	}
}
