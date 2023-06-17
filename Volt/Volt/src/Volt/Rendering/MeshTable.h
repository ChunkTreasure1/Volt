#pragma once

#include "Volt/Rendering/RendererStructs.h"
#include "Volt/Core/Base.h"

#include <unordered_map>

namespace Volt
{
	class Mesh;
	class ShaderStorageBufferSet;
	class ShaderStorageBuffer;
	class MeshTable
	{
	public:
		MeshTable();
		~MeshTable();

		const uint32_t AddMesh(Mesh* mesh, const uint32_t subMeshIndex);
		void RemoveMesh(Mesh* mesh, const uint32_t subMeshIndex);
		void UpdateMesh(Mesh* mesh, const uint32_t subMeshIndex);

		void UpdateMeshData(uint32_t index);

		const uint32_t GetMeshIndex(Mesh* mesh, const uint32_t subMeshIndex);

		const Ref<ShaderStorageBuffer> GetStorageBuffer(uint32_t index) const;
		const Ref<ShaderStorageBufferSet> GetStorageBufferSet() const;

		static Ref<MeshTable> Create();

	private:
		void SetDirty(bool state);

		const size_t GetHash(Mesh* mesh, const uint32_t subMeshIndex);

		std::vector<bool> myIsDirty;

		std::vector<uint32_t> myAvailableIndices;
		uint32_t myCurrentIndex = 0;

		std::vector<GPUMesh> myGPUMeshes;
		std::unordered_map<size_t, uint32_t> myMeshMap;
		Ref<ShaderStorageBufferSet> myStorageBuffer;
	};
}
