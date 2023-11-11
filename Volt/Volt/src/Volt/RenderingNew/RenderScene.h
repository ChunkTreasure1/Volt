#pragma once

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
	class SubMaterial;

	class RenderScene
	{
	public:
		RenderScene(Scene* sceneRef);
		~RenderScene();

		void PrepareForUpdate();

		void SetValid();
		void Invalidate();

		const UUID Register(entt::entity entityId, Ref<Mesh> mesh, uint32_t subMeshIndex);
		void Unregister(UUID id);

		inline const bool IsInvalid() const { return m_isInvalid; }
		inline const uint32_t GetRenderObjectCount() const { return static_cast<uint32_t>(m_renderObjects.size()); }
		inline const uint32_t GetMeshCommandCount() const { return static_cast<uint32_t>(m_meshCommands.size()); }
		inline const uint32_t GetIndividualMeshCount() const { return m_currentIndividualMeshCount; }
		inline const uint32_t GetIndividualMaterialCount() const { return static_cast<uint32_t>(m_individualMaterials.size()); }
		const uint32_t GetMeshID(Weak<Mesh> mesh, uint32_t subMeshIndex) const;
		const uint32_t GetMaterialIndex(Ref<SubMaterial> material) const;

		inline const GlobalResource<RHI::StorageBuffer>& GetGPUSceneBuffer() const { return *m_gpuSceneBuffer; }
		inline const GlobalResource<RHI::StorageBuffer>& GetGPUMeshesBuffer() const { return *m_gpuMeshesBuffer; }
		inline const GlobalResource<RHI::StorageBuffer>& GetGPUMaterialsBuffer() const { return *m_gpuMaterialsBuffer; }
		inline const GlobalResource<RHI::StorageBuffer>& GetObjectDrawDataBuffer() const { return *m_objectDrawDataBuffer; }

		std::vector<RenderObject>::iterator begin() { return m_renderObjects.begin(); }
		std::vector<RenderObject>::iterator end() { return m_renderObjects.end(); }

		const std::vector<RenderObject>::const_iterator cbegin() const { return m_renderObjects.cbegin(); }
		const std::vector<RenderObject>::const_iterator cend() const { return m_renderObjects.cend(); }

		inline std::span<const IndirectGPUCommandNew> GetMeshCommands() const { return m_meshCommands; }

	private:
		void UploadGPUMeshes();
		void UploadObjectDrawData();
		void UploadGPUScene();
		void UploadGPUMaterials();

		void BuildMeshCommands();

		std::vector<RenderObject> m_renderObjects;
		std::vector<IndirectGPUCommandNew> m_meshCommands;
		std::vector<Weak<Mesh>> m_individualMeshes;
		std::vector<Weak<SubMaterial>> m_individualMaterials;
		std::unordered_map<size_t, uint32_t> m_meshSubMeshToGPUMeshIndex;

		Scope<GlobalResource<RHI::StorageBuffer>> m_gpuSceneBuffer;
		Scope<GlobalResource<RHI::StorageBuffer>> m_gpuMeshesBuffer;
		Scope<GlobalResource<RHI::StorageBuffer>> m_gpuMaterialsBuffer;
		Scope<GlobalResource<RHI::StorageBuffer>> m_objectDrawDataBuffer;

		Scene* m_scene = nullptr;
		uint32_t m_currentIndividualMeshCount = 0;

		bool m_isInvalid = false;
	};
}
