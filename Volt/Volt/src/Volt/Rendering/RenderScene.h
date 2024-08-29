#pragma once

#include "Volt/Asset/Mesh/Mesh.h"

#include "Volt/Rendering/RenderObject.h"
#include "Volt/Rendering/BindlessResource.h"

#include "Volt/Rendering/RendererCommon.h"

#include <CoreUtilities/Containers/Map.h>

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
		BindlessResourceRef<RHI::StorageBuffer> meshesBuffer;
		BindlessResourceRef<RHI::StorageBuffer> sdfMeshesBuffer;
		BindlessResourceRef<RHI::StorageBuffer> materialsBuffer;
		BindlessResourceRef<RHI::StorageBuffer> primitiveDrawDataBuffer;
		BindlessResourceRef<RHI::StorageBuffer> sdfPrimitiveDrawDataBuffer;
		BindlessResourceRef<RHI::StorageBuffer> bonesBuffer;
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

		VT_INLINE VT_NODISCARD const bool IsInvalid() const { return m_isInvalid; }
		VT_INLINE VT_NODISCARD const uint32_t GetRenderObjectCount() const { return static_cast<uint32_t>(m_renderObjects.size()); }
		VT_INLINE VT_NODISCARD const uint32_t GetIndividualMeshCount() const { return m_currentIndividualMeshCount; }
		VT_INLINE VT_NODISCARD const uint32_t GetIndividualMaterialCount() const { return static_cast<uint32_t>(m_individualMaterials.size()); }
		VT_INLINE VT_NODISCARD const uint32_t GetMeshletCount() const { return m_currentMeshletCount; }
		VT_INLINE VT_NODISCARD const uint32_t GetDrawCount() const { return m_renderObjects.empty() ? 0u : static_cast<uint32_t>(m_primitiveDrawData.size()); }
		VT_INLINE VT_NODISCARD const uint32_t GetSDFPrimitiveCount() const { return static_cast<uint32_t>(m_sdfPrimitiveDrawData.size()); }

		VT_NODISCARD Weak<Material> GetMaterialFromID(const uint32_t materialId) const;

		VT_NODISCARD const uint32_t GetMeshID(Weak<Mesh> mesh, uint32_t subMeshIndex) const;
		VT_NODISCARD const uint32_t GetMaterialIndex(Weak<Material> material) const;
		VT_NODISCARD const uint32_t GetMeshIndex(Weak<Mesh> mesh) const;

		VT_INLINE VT_NODISCARD const GPUSceneBuffers GetGPUSceneBuffers() const { return m_buffers; }

		VT_INLINE VT_NODISCARD const BindlessResource<RHI::StorageBuffer>& GetGPUMeshesBuffer() const { return *m_buffers.meshesBuffer; }
		VT_INLINE VT_NODISCARD const BindlessResource<RHI::StorageBuffer>& GetGPUMaterialsBuffer() const { return *m_buffers.materialsBuffer; }
		VT_INLINE VT_NODISCARD const BindlessResource<RHI::StorageBuffer>& GetPrimitiveDrawDataBuffer() const { return *m_buffers.primitiveDrawDataBuffer; }

		VT_NODISCARD Vector<RenderObject>::iterator begin() { return m_renderObjects.begin(); }
		VT_NODISCARD Vector<RenderObject>::iterator end() { return m_renderObjects.end(); }

		VT_NODISCARD const Vector<RenderObject>::const_iterator cbegin() const { return m_renderObjects.cbegin(); }
		VT_NODISCARD const Vector<RenderObject>::const_iterator cend() const { return m_renderObjects.cend(); }

		VT_INLINE VT_NODISCARD const RenderObject& GetRenderObjectAt(const size_t index) const { return m_renderObjects.at(index); }
		VT_NODISCARD const RenderObject& GetRenderObjectFromID(UUID64 id) const;

		VT_INLINE VT_NODISCARD std::span<const GPUMesh> GetGPUMeshes() const { return m_gpuMeshes; }
		VT_INLINE VT_NODISCARD std::span<const PrimitiveDrawData> GetPrimitiveDrawData() const { return m_primitiveDrawData; }
	private:
		void UploadGPUMeshes(const Vector<GPUMesh>& gpuMeshes);
		void UploadGPUMeshSDFs(const Vector<GPUMeshSDF>& sdfMeshes);
		void UploadPrimitiveDrawData(const Vector<PrimitiveDrawData>& primitiveDrawData);
		void UploadSDFPrimitiveDrawData(const Vector<SDFPrimitiveDrawData>& primitiveDrawData);

		void UploadGPUMaterials();
		void BuildGPUMaterial(Weak<Material> material, GPUMaterial& gpuMaterial);

		Vector<GPUMesh> BuildGPUMeshes();
		Vector<GPUMeshSDF> BuildGPUMeshSDFs();

		Vector<PrimitiveDrawData> BuildPrimitiveDrawData();
		void BuildSinglePrimitiveDrawData(PrimitiveDrawData& primitiveDrawData, const RenderObject& renderObject);

		Vector<SDFPrimitiveDrawData> BuildSDFPrimitiveDrawData();
		void BuildSingleSDFPrimitiveDrawData(SDFPrimitiveDrawData& primtiveDrawData, const RenderObject& renderObject);


		void TryAddMesh(Ref<Mesh> mesh);
		void TryAddMaterial(Ref<Material> material);

		void UpdateInvalidMaterials(RenderGraph& renderGraph);
		void UpdateInvalidMeshes(RenderGraph& renderGraph);
		void UpdateInvalidPrimitiveData(RenderGraph& renderGraph);

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

		struct InvalidPrimitiveData
		{
			UUID64 renderObjectId;
			size_t index;
		};

		Vector<UUID64> m_animatedRenderObjects;
		Vector<RenderObject> m_renderObjects;

		Vector<GPUMesh> m_gpuMeshes;
		Vector<GPUMeshSDF> m_gpuSDFMeshes;
		Vector<GPUMaterial> m_gpuMaterials;
		Vector<PrimitiveDrawData> m_primitiveDrawData;
		Vector<SDFPrimitiveDrawData> m_sdfPrimitiveDrawData;

		Vector<InvalidPrimitiveData> m_invalidPrimitiveDataIndices;
		Vector<InvalidPrimitiveData> m_invalidSDFPrimitiveDataIndices;

		Vector<InvalidMaterial> m_invalidMaterials;
		Vector<InvalidMesh> m_invalidMeshes;

		vt::map<UUID64, uint32_t> m_primitiveIndexFromRenderObjectID;
		vt::map<UUID64, uint32_t> m_sdfPrimitiveIndexFromRenderObjectID;

		vt::map<AssetHandle, size_t> m_materialIndexFromAssetHandle;
		vt::map<size_t, size_t> m_gpuMeshIndexFromMeshAssetHash;
		vt::map<size_t, uint32_t> m_meshSubMeshToGPUMeshIndex;
		vt::map<size_t, uint32_t> m_meshSubMeshToGPUMeshSDFIndex;
		
		Vector<Weak<Mesh>> m_individualMeshes;
		Vector<Weak<Material>> m_individualMaterials;
		Vector<glm::mat4> m_animationBufferStorage;

		GPUSceneBuffers m_buffers;

		Scene* m_scene = nullptr;
		uint32_t m_currentIndividualMeshCount = 0;
		uint32_t m_currentBoneCount = 0;
		uint32_t m_currentMeshletCount = 0;

		UUID64 m_materialChangedCallbackID;
		UUID64 m_meshChangedCallbackID;

		bool m_isInvalid = false;
	};
}
