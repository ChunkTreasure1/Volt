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
		void Unregister(UUID64 id);

		inline const bool IsInvalid() const { return m_isInvalid; }
		inline const uint32_t GetRenderObjectCount() const { return static_cast<uint32_t>(m_renderObjects.size()); }
		inline const uint32_t GetMeshCommandCount() const { return static_cast<uint32_t>(m_meshCommands.size()); }
		inline const uint32_t GetMeshShaderCommandCount() const { return static_cast<uint32_t>(m_meshShaderCommands.size()); }
		inline const uint32_t GetIndividualMeshCount() const { return m_currentIndividualMeshCount; }
		inline const uint32_t GetIndividualMaterialCount() const { return static_cast<uint32_t>(m_individualMaterials.size()); }
		inline const uint32_t GetMeshletCount() const { return static_cast<uint32_t>(m_sceneMeshlets.size()); }
		inline const uint32_t GetIndexCount() const { return m_currentIndexCount; }

		Weak<Material> GetMaterialFromID(const uint32_t materialId) const;

		const uint32_t GetMeshID(Weak<Mesh> mesh, uint32_t subMeshIndex) const;
		const uint32_t GetMaterialIndex(Weak<Material> material) const;
		const uint32_t GetMeshIndex(Weak<Mesh> mesh) const;

		inline const BindlessResource<RHI::StorageBuffer>& GetGPUSceneBuffer() const { return *m_gpuSceneBuffer; }
		inline const BindlessResource<RHI::StorageBuffer>& GetGPUMeshesBuffer() const { return *m_gpuMeshesBuffer; }
		inline const BindlessResource<RHI::StorageBuffer>& GetGPUMaterialsBuffer() const { return *m_gpuMaterialsBuffer; }
		inline const BindlessResource<RHI::StorageBuffer>& GetGPUMeshletsBuffer() const { return *m_gpuMeshletsBuffer; }
		inline const BindlessResource<RHI::StorageBuffer>& GetObjectDrawDataBuffer() const { return *m_objectDrawDataBuffer; }

		std::vector<RenderObject>::iterator begin() { return m_renderObjects.begin(); }
		std::vector<RenderObject>::iterator end() { return m_renderObjects.end(); }

		const std::vector<RenderObject>::const_iterator cbegin() const { return m_renderObjects.cbegin(); }
		const std::vector<RenderObject>::const_iterator cend() const { return m_renderObjects.cend(); }

		inline const RenderObject& GetRenderObjectAt(const size_t index) const { return m_renderObjects.at(index); }
		const RenderObject& GetRenderObjectFromID(UUID64 id) const;

		inline std::span<const IndirectGPUCommandNew> GetMeshCommands() const { return m_meshCommands; }
		inline std::span<const IndirectMeshTaskCommand> GetMeshShaderCommands() const { return m_meshShaderCommands; }

	private:
		void UploadGPUMeshes(std::vector<GPUMesh>& gpuMeshes);
		void UploadObjectDrawData(std::vector<ObjectDrawData>& objectDrawData);
		void UploadGPUScene();
		void UploadGPUMeshlets();

		void UploadGPUMaterials();
		void BuildGPUMaterial(Weak<Material> material, GPUMaterialNew& gpuMaterial);

		void BuildGPUMeshes(std::vector<GPUMesh>& gpuMeshes);

		void BuildObjectDrawData(std::vector<ObjectDrawData>& objectDrawData);
		void BuildSinlgeObjectDrawData(ObjectDrawData& objectDrawData, const RenderObject& renderObject);

		void BuildMeshCommands();
		void BuildMeshletBuffer(std::vector<ObjectDrawData>& gpuMeshes);

		std::vector<Meshlet> m_sceneMeshlets;

		std::vector<RenderObject> m_renderObjects;
		std::vector<ObjectDrawData> m_objectDrawData;

		std::vector<size_t> m_invalidRenderObjectIndices;

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

		std::vector<InvalidMaterial> m_invalidMaterials;
		std::vector<InvalidMesh> m_invalidMeshes;

		std::unordered_map<UUID64, uint32_t> m_objectIndexFromRenderObjectID;
		std::unordered_map<AssetHandle, size_t> m_materialIndexFromAssetHandle;
		std::unordered_map<size_t, size_t> m_meshIndexFromMeshAssetHash;

		std::vector<IndirectGPUCommandNew> m_meshCommands;
		std::vector<Weak<Mesh>> m_individualMeshes;
		std::vector<Weak<Material>> m_individualMaterials;
		std::unordered_map<size_t, uint32_t> m_meshSubMeshToGPUMeshIndex;

		// Mesh Shader rendering
		std::vector<IndirectMeshTaskCommand> m_meshShaderCommands;

		BindlessResourceScope<RHI::StorageBuffer> m_gpuSceneBuffer;
		BindlessResourceScope<RHI::StorageBuffer> m_gpuMeshesBuffer;
		BindlessResourceScope<RHI::StorageBuffer> m_gpuMaterialsBuffer;
		BindlessResourceScope<RHI::StorageBuffer> m_objectDrawDataBuffer;
		BindlessResourceScope<RHI::StorageBuffer> m_gpuMeshletsBuffer;

		Scene* m_scene = nullptr;
		uint32_t m_currentIndividualMeshCount = 0;
		uint32_t m_currentIndexCount = 0;

		UUID64 m_materialChangedCallbackID;
		UUID64 m_meshChangedCallbackID;

		bool m_isInvalid = false;
	};
}
