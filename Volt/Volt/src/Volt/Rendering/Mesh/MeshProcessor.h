#pragma once

#include "Volt/Asset/Rendering/MaterialTable.h"
#include "Volt/Asset/Mesh/SubMesh.h"

#include "Volt/Rendering/Vertex.h"
#include "Volt/Rendering/Mesh/MeshCommon.h"

#include <vector>

namespace Volt
{
	struct ProcessedMeshResult
	{

	};

	struct MeshletGenerationResult
	{
		std::vector<MeshletNew> meshlets;
		std::vector<uint32_t> meshletData; // Index and vertex remapping

		std::vector<uint32_t> nonOffsetedIndices;
	};

	class MeshProcessor
	{
	public:
		static ProcessedMeshResult ProcessMesh(const std::vector<Vertex>& vertices, const std::vector<uint32_t>& indices, const MaterialTable& materialTable, const std::vector<SubMesh>& subMeshes);

	private:
		static MeshletGenerationResult GenerateMeshlets(const Vertex* vertices, const uint32_t* indices, uint32_t indexCount, uint32_t vertexCount);

		MeshProcessor() = delete;
	};
}
