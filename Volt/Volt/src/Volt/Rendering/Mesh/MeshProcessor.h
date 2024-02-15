#pragma once

#include "Volt/Asset/Rendering/MaterialTable.h"
#include "Volt/Asset/Mesh/SubMesh.h"

#include "Volt/Rendering/Vertex.h"
#include "Volt/Rendering/Mesh/MeshCommon.h"

#include "Volt/RenderingNew/Resources/GlobalResource.h"

#include <vector>

namespace Volt
{
	namespace RHI
	{
		class StorageBuffer;
	}

	struct ProcessedMeshResult
	{
		std::vector<Meshlet> meshlets;
		std::vector<uint32_t> meshletIndices;

		Ref<GlobalResource<RHI::StorageBuffer>> vertexPositionsBuffer;
	};

	struct MeshletGenerationResult
	{
		std::vector<Meshlet> meshlets;
		std::vector<uint32_t> meshletIndices; 
	};

	class MeshProcessor
	{
	public:
		static ProcessedMeshResult ProcessMesh(const std::vector<Vertex>& vertices, const std::vector<uint32_t>& indices, const MaterialTable& materialTable, const std::vector<SubMesh>& subMeshes);

	private:
		struct MeshletGroup
		{
			std::vector<uint32_t> meshletIndices;
		};

		struct GroupedMeshletResult
		{
			std::vector<uint32_t> groupedMeshletIndices;
		};

		struct SimplifiedGroupResult
		{
			std::vector<uint32_t> simplifiedIndices;
		};

		static std::vector<MeshletGroup> GroupMeshlets(const MeshletGenerationResult& meshletGenerationResult);
		static std::vector<GroupedMeshletResult> MergeMeshletGroups(std::span<const MeshletGroup> groups, std::span<const Meshlet> parentMeshlets, std::span<const uint32_t> indices);
		static std::vector<SimplifiedGroupResult> SimplifyGroups(std::span<const GroupedMeshletResult> groups, std::span<const Vertex> vertices);
		static MeshletGenerationResult SplitGroups(std::span<const SimplifiedGroupResult> groups, const std::span<const Vertex> vertices);

		static MeshletGenerationResult GenerateMeshlets(std::span<const Vertex> vertices, std::span<const uint32_t> indices);
		static MeshletGenerationResult GenerateMeshlets2(std::span<const Vertex> vertices, std::span<const uint32_t> indices);

		MeshProcessor() = delete;
	};
}
