#pragma once

#include "Volt/Asset/Rendering/MaterialTable.h"
#include "Volt/Asset/Mesh/SubMesh.h"

#include "Volt/Rendering/Vertex.h"
#include "Volt/Rendering/Mesh/MeshCommon.h"

#include "Volt/RenderingNew/Resources/GlobalResource.h"

#include <CoreUtilities/Containers/Graph.h>

#include <vector>

namespace Volt
{
	namespace RHI
	{
		class StorageBuffer;
	}

	struct MeshLOD
	{
		uint32_t meshletOffset = 0;
		uint32_t meshletCount = 0;

		std::vector<UUID64> lodMeshletNodeIDs;
	};

	struct MeshLODGraphData
	{
		float clusterError = 0.f;
		uint32_t meshletIndex = 0;
	};

	struct MeshLODEdgeData {};

	struct ProcessedMeshResult
	{
		std::vector<Meshlet> meshlets;
		std::vector<uint32_t> meshletIndices;
		std::vector<MeshLOD> lods;

		Graph<MeshLODGraphData, MeshLODEdgeData> lodGraph;

		Ref<GlobalResource<RHI::StorageBuffer>> vertexPositionsBuffer;
	};

	struct MeshletGenerationResult
	{
		std::vector<Meshlet> meshlets;
		std::vector<uint32_t> meshletIndices;
		std::vector<uint32_t> meshletIndexToGroupID;
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
			float simplificationError = 0.f;
		};

		static std::vector<MeshletGroup> GroupMeshlets(const MeshletGenerationResult& meshletGenerationResult);
		static std::vector<GroupedMeshletResult> MergeMeshletGroups(std::span<const MeshletGroup> groups, std::span<const Meshlet> parentMeshlets, std::span<const uint32_t> indices);
		static std::vector<SimplifiedGroupResult> SimplifyGroups(std::span<const GroupedMeshletResult> groups, std::span<const Vertex> vertices);
		static MeshletGenerationResult SplitGroups(std::span<const SimplifiedGroupResult> groups, const std::span<const Vertex> vertices);

		static void AddLODLevel(const MeshletGenerationResult& meshletGenerationResult, ProcessedMeshResult& result);

		static MeshletGenerationResult GenerateMeshlets(std::span<const Vertex> vertices, std::span<const uint32_t> indices);
		static MeshletGenerationResult GenerateMeshlets2(std::span<const Vertex> vertices, std::span<const uint32_t> indices);

		MeshProcessor() = delete;
	};
}
