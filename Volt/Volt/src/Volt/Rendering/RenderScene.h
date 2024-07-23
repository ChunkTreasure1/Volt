#pragma once

#include "Volt/Asset/Mesh/Mesh.h"

#include "Volt/Rendering/RenderObject.h"
#include "Volt/Rendering/Resources/BindlessResource.h"

#include "Volt/Rendering/RendererCommon.h"

#include <span>

namespace Volt
{
	namespace RHI
	{
		class BufferView;
		class StorageBuffer;
	}

	class Scene;
	class Material;
	class RenderGraph;
	class MotionWeaver;

	struct GPUSceneBuffers
	{
		RefPtr<RHI::StorageBuffer> meshesBuffer;
		RefPtr<RHI::StorageBuffer> materialsBuffer;
		RefPtr<RHI::StorageBuffer> objectDrawDataBuffer;
		RefPtr<RHI::StorageBuffer> bonesBuffer;
	};

	class RenderScene
	{
	public:
		RenderScene(Scene* sceneRef);
		~RenderScene();

		void PrepareForUpdate();
		void Update(RenderGraph& renderGraph);

		void SetValid();
		void Invalidate();
		void InvalidateRenderObject(UUID64 renderObject);

		const UUID64 Register(EntityID entityId, Ref<Mesh> mesh, Ref<Material> material, uint32_t subMeshIndex);
		const UUID64 Register(EntityID entityId, Ref<MotionWeaver> motionWeaver, Ref<Mesh> mesh, Ref<Material> material, uint32_t subMeshIndex);

		void Unregister(UUID64 id);

		inline const bool IsInvalid() const { return m_isInvalid; }
		inline const uint32_t GetRenderObjectCount() const { return static_cast<uint32_t>(m_renderObjects.size()); }
		inline const uint32_t GetIndividualMeshCount() const { return m_currentIndividualMeshCount; }
		inline const uint32_t GetIndividualMaterialCount() const { return static_cast<uint32_t>(m_individualMaterials.size()); }
		inline const uint32_t GetIndexCount() const { return m_currentIndexCount; }
		inline const uint32_t GetMeshletCount() const { return m_currentMeshletCount; }
		inline const uint32_t GetDrawCount() const { return static_cast<uint32_t>(m_objectDrawData.size()); }

		Weak<Material> GetMaterialFromID(const uint32_t materialId) const;

		const uint32_t GetMeshID(Weak<Mesh> mesh, uint32_t subMeshIndex) const;
		const uint32_t GetMaterialIndex(Weak<Material> material) const;
		const uint32_t GetMeshIndex(Weak<Mesh> mesh) const;

		const GPUSceneBuffers GetGPUSceneBuffers() const;

		inline const BindlessResource<RHI::StorageBuffer>& GetGPUMeshesBuffer() const { return *m_gpuMeshesBuffer; }
		inline const BindlessResource<RHI::StorageBuffer>& GetGPUMaterialsBuffer() const { return *m_gpuMaterialsBuffer; }
		inline const BindlessResource<RHI::StorageBuffer>& GetObjectDrawDataBuffer() const { return *m_objectDrawDataBuffer; }

		Vector<RenderObject>::iterator begin() { return m_renderObjects.begin(); }
		Vector<RenderObject>::iterator end() { return m_renderObjects.end(); }

		const Vector<RenderObject>::const_iterator cbegin() const { return m_renderObjects.cbegin(); }
		const Vector<RenderObject>::const_iterator cend() const { return m_renderObjects.cend(); }

		inline const RenderObject& GetRenderObjectAt(const size_t index) const { return m_renderObjects.at(index); }
		const RenderObject& GetRenderObjectFromID(UUID64 id) const;

		inline std::span<const GPUMesh> GetGPUMeshes() const { return m_gpuMeshes; }
		inline std::span<const ObjectDrawData> GetObjectDrawData() const { return m_objectDrawData; }
	private:
		void UploadGPUMeshes(Vector<GPUMesh>& gpuMeshes);
		void UploadObjectDrawData(Vector<ObjectDrawData>& objectDrawData);

		void UploadGPUMaterials();
		void BuildGPUMaterial(Weak<Material> material, GPUMaterial& gpuMaterial);

		void BuildGPUMeshes(Vector<GPUMesh>& gpuMeshes);

		void BuildObjectDrawData(Vector<ObjectDrawData>& objectDrawData);
		void BuildSinlgeObjectDrawData(ObjectDrawData& objectDrawData, const RenderObject& renderObject);

		Vector<UUID64> m_animatedRenderObjects;
		Vector<RenderObject> m_renderObjects;

		Vector<GPUMesh> m_gpuMeshes;
		Vector<ObjectDrawData> m_objectDrawData;

		Vector<size_t> m_invalidRenderObjectIndices;

		struct InvalidMaterial
		{
			Weak<Material> material;
			size_t index;
		};

		struct InvalidMesh
		{
			Weak<Mesh> mesh;
			uint32_t subMeshIndex;
			size_t index;
		};

		Vector<InvalidMaterial> m_invalidMaterials;
		Vector<InvalidMesh> m_invalidMeshes;

		std::unordered_map<UUID64, uint32_t> m_objectIndexFromRenderObjectID;
		std::unordered_map<AssetHandle, size_t> m_materialIndexFromAssetHandle;
		std::unordered_map<size_t, size_t> m_meshIndexFromMeshAssetHash;

		Vector<Weak<Mesh>> m_individualMeshes;
		Vector<Weak<Material>> m_individualMaterials;
		std::unordered_map<size_t, uint32_t> m_meshSubMeshToGPUMeshIndex;

		Vector<glm::mat4> m_animationBufferStorage;

		BindlessResourceScope<RHI::StorageBuffer> m_gpuMeshesBuffer;
		BindlessResourceScope<RHI::StorageBuffer> m_gpuMaterialsBuffer;
		BindlessResourceScope<RHI::StorageBuffer> m_objectDrawDataBuffer;
		BindlessResourceScope<RHI::StorageBuffer> m_gpuBonesBuffer;

		Scene* m_scene = nullptr;
		uint32_t m_currentIndividualMeshCount = 0;
		uint32_t m_currentIndexCount = 0;
		uint32_t m_currentBoneCount = 0;
		uint32_t m_currentMeshletCount = 0;

		UUID64 m_materialChangedCallbackID;
		UUID64 m_meshChangedCallbackID;

		bool m_isInvalid = false;
	};
}
