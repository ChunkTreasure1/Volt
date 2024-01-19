#pragma once

#include "Volt/Asset/Mesh/Mesh.h"

#include "Volt/RenderingNew/RenderObject.h"
#include "Volt/RenderingNew/Resources/GlobalResource.h"

#include "Volt/RenderingNew/RendererCommon.h"

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

	class RenderScene
	{
	public:
		RenderScene(Scene* sceneRef);
		~RenderScene();

		void PrepareForUpdate();
		void Update();

		void SetValid();
		void Invalidate();
		void InvalidateRenderObject(UUID64 renderObject);

		const UUID64 Register(EntityID entityId, Ref<Mesh> mesh, uint32_t subMeshIndex);
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

		inline const GlobalResource<RHI::StorageBuffer>& GetGPUSceneBuffer() const { return *m_gpuSceneBuffer; }
		inline const GlobalResource<RHI::StorageBuffer>& GetGPUMeshesBuffer() const { return *m_gpuMeshesBuffer; }
		inline const GlobalResource<RHI::StorageBuffer>& GetGPUMaterialsBuffer() const { return *m_gpuMaterialsBuffer; }
		inline const GlobalResource<RHI::StorageBuffer>& GetGPUMeshletsBuffer() const { return *m_gpuMeshletsBuffer; }
		inline const GlobalResource<RHI::StorageBuffer>& GetObjectDrawDataBuffer() const { return *m_objectDrawDataBuffer; }

		std::vector<RenderObject>::iterator begin() { return m_renderObjects.begin(); }
		std::vector<RenderObject>::iterator end() { return m_renderObjects.end(); }

		const std::vector<RenderObject>::const_iterator cbegin() const { return m_renderObjects.cbegin(); }
		const std::vector<RenderObject>::const_iterator cend() const { return m_renderObjects.cend(); }

		inline const RenderObject& GetRenderObjectAt(const uint32_t index) const { return m_renderObjects.at(index); }

		inline std::span<const IndirectGPUCommandNew> GetMeshCommands() const { return m_meshCommands; }
		inline std::span<const IndirectMeshTaskCommand> GetMeshShaderCommands() const { return m_meshShaderCommands; }

	private:
		void UploadGPUMeshes(std::vector<GPUMesh>& gpuMeshes);
		void UploadObjectDrawData(std::vector<ObjectDrawData>& objectDrawData);
		void UploadGPUScene();
		void UploadGPUMaterials();
		void UploadGPUMeshlets();

		void BuildGPUMeshes(std::vector<GPUMesh>& gpuMeshes);
		void BuildObjectDrawData(std::vector<ObjectDrawData>& objectDrawData);

		void BuildMeshCommands();
		void BuildMeshletBuffer(std::vector<ObjectDrawData>& gpuMeshes);

		std::vector<Meshlet> m_sceneMeshlets;

		std::vector<RenderObject> m_renderObjects;
		std::vector<ObjectDrawData> m_objectDrawData;

		std::vector<UUID64> m_invalidRenderObjects;

		std::vector<IndirectGPUCommandNew> m_meshCommands;
		std::vector<Weak<Mesh>> m_individualMeshes;
		std::vector<Weak<Material>> m_individualMaterials;
		std::unordered_map<size_t, uint32_t> m_meshSubMeshToGPUMeshIndex;

		// Mesh Shader rendering
		std::vector<IndirectMeshTaskCommand> m_meshShaderCommands;

		Scope<GlobalResource<RHI::StorageBuffer>> m_gpuSceneBuffer;
		Scope<GlobalResource<RHI::StorageBuffer>> m_gpuMeshesBuffer;
		Scope<GlobalResource<RHI::StorageBuffer>> m_gpuMaterialsBuffer;
		Scope<GlobalResource<RHI::StorageBuffer>> m_objectDrawDataBuffer;
		Scope<GlobalResource<RHI::StorageBuffer>> m_gpuMeshletsBuffer;

		Scene* m_scene = nullptr;
		uint32_t m_currentIndividualMeshCount = 0;
		uint32_t m_currentIndexCount = 0;

		bool m_isInvalid = false;
	};
}
