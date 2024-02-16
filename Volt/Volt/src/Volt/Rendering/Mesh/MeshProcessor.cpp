#include "vtpch.h"
#include "MeshProcessor.h"

#include "Volt/Math/Math.h"
#include "Volt/Utility/Algorithms.h"

#include <VoltRHI/Buffers/StorageBuffer.h>

#include <meshoptimizer/meshoptimizer.h>

#include <metis.h>

namespace Volt
{
	namespace Utility
	{
		template<typename T>
		inline static void VertexHashCombine(size_t& s, const T& v)
		{
			std::hash<T> h;
			s ^= h(v) + 0x9e3779b9 + (s << 6) + (s >> 2);
		}

		inline static size_t GetHashFromVertex(const Vertex& vertex)
		{
			size_t result = 0;
			VertexHashCombine(result, vertex.position.x);
			VertexHashCombine(result, vertex.position.y);
			VertexHashCombine(result, vertex.position.z);

			return result;
		}
	}

	std::vector<MeshProcessor::MeshletGroup> MeshProcessor::GroupMeshlets(const MeshletGenerationResult& meshletGenerationResult)
	{
		// Find meshlets who have adjacent triangles
		// Find adjacent triangle edge count
		
		const auto& meshlets = meshletGenerationResult.meshlets;

		constexpr size_t GROUP_SIZE = 4;

		auto GenerateSingleGroup = [&]()
		{
			std::vector<MeshletGroup> result(1);
			result.back().meshletIndices.resize(meshlets.size());

			for (uint32_t i = 0; i < static_cast<uint32_t>(meshlets.size()); i++)
			{
				result.back().meshletIndices[i] = i;
			}

			return result;
		};

		if (meshlets.size() < GROUP_SIZE * 2)
		{
			return GenerateSingleGroup();
		}

		std::unordered_map<Edge, std::unordered_set<size_t>> edges2Meshlets;
		std::unordered_map<size_t, std::vector<Edge>> meshlets2Edges;

		for (size_t meshletIndex = 0; meshletIndex < meshlets.size(); meshletIndex++)
		{
			const auto& meshlet = meshlets.at(meshletIndex);

			auto GetVertexIndex = [&](size_t index) 
			{
				return meshletGenerationResult.meshletIndices[meshlet.triangleOffset + index];
			};

			const uint32_t triangleCount = meshlet.triangleCount;

			for (uint32_t i = 0; i < triangleCount * 3; i += 3)
			{
				for (uint32_t j = 0; j < 3; j++)
				{
					uint32_t v1 = GetVertexIndex(i + j);
					uint32_t v2 = GetVertexIndex(i + (j + 1) % 3);

					if (v1 > v2)         
					{
						std::swap(v1, v2);
					}

					Edge edge{ v1, v2 };
					edges2Meshlets[edge].insert(meshletIndex);
					meshlets2Edges[meshletIndex].push_back(edge);
				}
			}
		}

		std::erase_if(edges2Meshlets, [](const auto& pair)
		{
			return pair.second.size() <= 1;
		});

		idx_t vertexCount = static_cast<idx_t>(meshlets.size());
		idx_t partitionCount = static_cast<idx_t>(std::max(vertexCount / static_cast<uint32_t>(GROUP_SIZE), 2u));
		idx_t numConstraints = 1;
		idx_t options[METIS_NOPTIONS];
		METIS_SetDefaultOptions(options);

		options[METIS_OPTION_OBJTYPE] = METIS_OBJTYPE_CUT;
		options[METIS_OPTION_CCORDER] = 1;

		idx_t edgesCut = 0;

		std::vector<idx_t> partitions(vertexCount, 0);
		std::vector<idx_t> edgeAdjacency;
		std::vector<idx_t> edgeWeights;
		
		std::vector<idx_t> xadjacency;
		xadjacency.reserve(vertexCount + 1);

		for (size_t meshletIndex = 0; meshletIndex < meshlets.size(); meshletIndex++)
		{
			size_t startIndexInEdgeAdjacency = edgeAdjacency.size();

			for (const auto& edge : meshlets2Edges[meshletIndex])
			{
				if (!edges2Meshlets.contains(edge))
				{
					continue;
				}

				const auto& connections = edges2Meshlets.at(edge);
				for (const auto& conntectedMeshlet : connections)
				{
					if (conntectedMeshlet != meshletIndex)
					{
						auto it = std::find(edgeAdjacency.begin() + startIndexInEdgeAdjacency, edgeAdjacency.end(), conntectedMeshlet);
						if (it == edgeAdjacency.end())
						{
							edgeAdjacency.emplace_back(static_cast<uint32_t>(conntectedMeshlet));
							edgeWeights.emplace_back(1);
						}
						else
						{
							std::ptrdiff_t dist = std::distance(edgeAdjacency.begin(), it);
							edgeWeights[dist]++;
						}
					}
				}
			}

			xadjacency.push_back(static_cast<uint32_t>(startIndexInEdgeAdjacency));
		}

		xadjacency.push_back(static_cast<uint32_t>(edgeAdjacency.size()));

		int r = METIS_PartGraphKway(&vertexCount, &numConstraints, xadjacency.data(), edgeAdjacency.data(), nullptr, nullptr, edgeWeights.data(), &partitionCount, nullptr, nullptr, options, &edgesCut, partitions.data());
		VT_CORE_ASSERT(r == METIS_OK, "METIS should never fail!");

		struct Range
		{
			uint32_t begin;
			uint32_t end;
		};

		std::vector<uint32_t> elementCount(partitionCount, 0);
		std::vector<Range> ranges(partitionCount);

		for (size_t i = 0; i < partitions.size(); i++)
		{
			elementCount[partitions[i]]++;
		}

		uint32_t begin = 0;
		for (idx_t partIndex = 0; partIndex < partitionCount; partIndex++)
		{
			ranges[partIndex] = { begin, begin + elementCount[partIndex] };
			begin += elementCount[partIndex];
			elementCount[partIndex] = 0;
		}

		std::vector<uint32_t> indexes(meshlets.size());
		for (uint32_t i = 0; i < static_cast<uint32_t>(meshlets.size()); i++)
		{
			uint32_t partitionIndex = partitions[i];
			uint32_t offset = ranges[partitionIndex].begin;
			uint32_t count = elementCount[partitionIndex]++;

			indexes[offset + count] = i;
		}

		std::vector<MeshletGroup> groups;
		groups.reserve(partitionCount);
		for (const auto& range : ranges)
		{
			auto& group = groups.emplace_back();
			for (uint32_t rangeIndex = range.begin; rangeIndex < range.end; rangeIndex++)
			{
				group.meshletIndices.emplace_back(indexes[rangeIndex]);
			}
		}

		return groups;
	}

	std::vector<MeshProcessor::GroupedMeshletResult> MeshProcessor::MergeMeshletGroups(std::span<const MeshletGroup> groups, std::span<const Meshlet> generatedMeshlets, std::span<const uint32_t> indices)
	{
		std::vector<GroupedMeshletResult> result;

		for (const auto& group : groups)
		{
			std::vector<uint32_t>& groupIndices = result.emplace_back().groupedMeshletIndices;

			for (const auto& meshletIndex : group.meshletIndices)
			{
				const auto& meshlet = generatedMeshlets[meshletIndex];

				size_t start = groupIndices.size();
				groupIndices.resize(start + static_cast<size_t>(meshlet.triangleCount * 3));
				for (size_t i = 0; i < static_cast<size_t>(meshlet.triangleCount * 3); i++)
				{
					groupIndices[start + i] = indices[meshlet.triangleOffset + i];
				}
			}
		}

		return result;
	}

	std::vector<MeshProcessor::SimplifiedGroupResult> MeshProcessor::SimplifyGroups(std::span<const GroupedMeshletResult> groups, std::span<const Vertex> vertices)
	{
		std::vector<SimplifiedGroupResult> result;
		for (const auto& group : groups)
		{
			auto& simplifiedGroup = result.emplace_back();

			float targetError = 1e-1f;
			const float threshold = 0.5f;
			const size_t targetIndexCount = static_cast<size_t>(group.groupedMeshletIndices.size() * threshold);
			const uint32_t options = meshopt_SimplifyLockBorder;

			simplifiedGroup.simplifiedIndices.resize(group.groupedMeshletIndices.size());
			float error = 0.f;

			std::vector<uint32_t> tempIndices(group.groupedMeshletIndices.size());

			meshopt_generateShadowIndexBuffer(tempIndices.data(), group.groupedMeshletIndices.data(), group.groupedMeshletIndices.size(), vertices.data(), vertices.size(), sizeof(Vertex), sizeof(Vertex));

			size_t simplifiedIndexCount = meshopt_simplify(simplifiedGroup.simplifiedIndices.data(), tempIndices.data(), tempIndices.size(), &vertices[0].position.x, vertices.size(), sizeof(Vertex), targetIndexCount, targetError, options, &error);
			simplifiedGroup.simplifiedIndices.resize(simplifiedIndexCount);
			simplifiedGroup.simplifiedIndices.shrink_to_fit();
			simplifiedGroup.simplificationError = error * meshopt_simplifyScale(&vertices[0].position.x, vertices.size(), sizeof(Vertex));
		}

		return result;
	}

	MeshletGenerationResult MeshProcessor::SplitGroups(std::span<const SimplifiedGroupResult> groups, const std::span<const Vertex> vertices)
	{
		MeshletGenerationResult result;

		for (uint32_t groupIndex = 0; const auto& group : groups)
		{
			auto meshletData = GenerateMeshlets(vertices, group.simplifiedIndices);

			const size_t groupIndexOffset = result.meshletIndices.size();
			const size_t groupMeshletOffset = result.meshlets.size();

			result.meshletIndices.insert(result.meshletIndices.end(), meshletData.meshletIndices.begin(), meshletData.meshletIndices.end());
			result.meshletIndexToGroupID.resize(result.meshletIndexToGroupID.size() + meshletData.meshlets.size());

			for (uint32_t meshletIndex = 0; auto& meshlet : meshletData.meshlets)
			{
				meshlet.triangleOffset += static_cast<uint32_t>(groupIndexOffset);
				meshlet.clusterError = group.simplificationError;

				result.meshletIndexToGroupID[groupMeshletOffset + meshletIndex] = groupIndex;
				meshletIndex++;
			}

			result.meshlets.insert(result.meshlets.end(), meshletData.meshlets.begin(), meshletData.meshlets.end());
			groupIndex++;
		}

		return result;
	}

	void MeshProcessor::AddLODLevel(const MeshletGenerationResult& meshletGenerationResult, ProcessedMeshResult& result)
	{
		const size_t lodIndexOffset = result.meshletIndices.size();
		const size_t lodMeshletOffset = result.meshlets.size();

		auto& lodLevel = result.lods.emplace_back();
		lodLevel.meshletCount = static_cast<uint32_t>(meshletGenerationResult.meshlets.size());
		lodLevel.meshletOffset = static_cast<uint32_t>(lodMeshletOffset);

		for (const auto& meshlet : meshletGenerationResult.meshlets)
		{
			result.meshlets.emplace_back(meshlet);
			result.meshlets.back().triangleOffset += static_cast<uint32_t>(lodIndexOffset);
		}

		result.meshletIndices.insert(result.meshletIndices.end(), meshletGenerationResult.meshletIndices.begin(), meshletGenerationResult.meshletIndices.end());
	}

	ProcessedMeshResult MeshProcessor::ProcessMesh(const std::vector<Vertex>& vertices, const std::vector<uint32_t>& indices, const MaterialTable& materialTable, const std::vector<SubMesh>& subMeshes)
	{
		std::vector<Vertex> resultVertices;
		std::vector<uint32_t> resultIndices;
		std::vector<Meshlet> resultMeshlets;
		std::vector<SubMesh> resultSubMeshes = subMeshes;

		ProcessedMeshResult result;

		for (auto& subMesh : resultSubMeshes)
		{
			const uint32_t* indexStartPtr = &indices.at(subMesh.indexStartOffset);
			const Vertex* vertexStartPtr = &vertices.at(subMesh.vertexStartOffset);

			std::vector<uint32_t> subMeshIndices(subMesh.indexCount);
			std::vector<Vertex> subMeshVertices(subMesh.vertexCount);

			{
				std::vector<uint32_t> remapTable(subMesh.indexCount);
				size_t newVertexCount = meshopt_generateVertexRemap(remapTable.data(), indexStartPtr, static_cast<size_t>(subMesh.indexCount), vertexStartPtr, static_cast<size_t>(subMesh.vertexCount), sizeof(Vertex));
				subMeshVertices.resize(newVertexCount);

				const size_t vertexCount = static_cast<size_t>(newVertexCount);
				const size_t indexCount = static_cast<size_t>(subMesh.indexCount);

				meshopt_remapIndexBuffer(subMeshIndices.data(), indices.data(), indexCount, remapTable.data());
				meshopt_remapVertexBuffer(subMeshVertices.data(), vertices.data(), vertexCount, sizeof(Vertex), remapTable.data());

				//meshopt_optimizeVertexCache(subMeshIndices.data(), subMeshIndices.data(), indexCount, vertexCount);
				//meshopt_optimizeOverdraw(subMeshIndices.data(), subMeshIndices.data(), indexCount, &subMeshVertices[0].position.x, vertexCount, sizeof(Vertex), 1.05f);
				//meshopt_optimizeVertexFetch(subMeshVertices.data(), subMeshIndices.data(), indexCount, subMeshVertices.data(), vertexCount, sizeof(Vertex));
			}

			{
				std::vector<glm::vec3> vertexPositions;
				vertexPositions.reserve(subMeshVertices.size());
				for (const auto& vertex : subMeshVertices)
				{
					vertexPositions.emplace_back(vertex.position);
				}

				result.vertexPositionsBuffer = GlobalResource<RHI::StorageBuffer>::Create(RHI::StorageBuffer::Create(static_cast<uint32_t>(vertexPositions.size()), sizeof(glm::vec3), "Test Positions"));
				result.vertexPositionsBuffer->GetResource()->SetData(vertexPositions.data(), vertexPositions.size() * sizeof(glm::vec3));
			}

			auto lod0MeshletData = GenerateMeshlets(subMeshVertices, subMeshIndices);
			AddLODLevel(lod0MeshletData, result);

			for (uint32_t i = 0; const auto& meshlet : lod0MeshletData.meshlets)
			{
				result.lods.back().lodMeshletNodeIDs.emplace_back() = result.lodGraph.AddNode({ meshlet.clusterError, i });
				i++;
			}

			size_t meshletCount = lod0MeshletData.meshlets.size();
			MeshletGenerationResult previousResult = std::move(lod0MeshletData);

			while (meshletCount > 1)
			{
				auto groups = GroupMeshlets(previousResult);
				auto mergedGroups = MergeMeshletGroups(groups, previousResult.meshlets, previousResult.meshletIndices);
				auto simplifiedGroups = SimplifyGroups(mergedGroups, subMeshVertices);

				bool failed = false;

				for (size_t groupIndex = 0; groupIndex < simplifiedGroups.size(); groupIndex++)
				{
					if (mergedGroups.at(groupIndex).groupedMeshletIndices.size() == simplifiedGroups.at(groupIndex).simplifiedIndices.size())
					{
						failed = true;
					}
				}

				if (failed)
				{
					break;
				}

				auto meshletData = SplitGroups(simplifiedGroups, vertices);

				AddLODLevel(meshletData, result);

				for (uint32_t i = 0; const auto& meshlet : meshletData.meshlets)
				{
					const auto nodeId = result.lodGraph.AddNode({ meshlet.clusterError, i });
					result.lods.back().lodMeshletNodeIDs.emplace_back(nodeId);

					auto& previousLOD = result.lods.at(result.lods.size() - 2);

					const uint32_t groupIndex = meshletData.meshletIndexToGroupID.at(i);
					for (const auto& parentMeshletIndex : groups.at(groupIndex).meshletIndices)
					{
						result.lodGraph.LinkNodes(nodeId, previousLOD.lodMeshletNodeIDs.at(parentMeshletIndex));
					}

					i++;

				}

				meshletCount = meshletData.meshlets.size();
				previousResult = std::move(meshletData);
			}

			subMesh.meshletStartOffset = static_cast<uint32_t>(resultMeshlets.size());
			subMesh.meshletVertexStartOffset = static_cast<uint32_t>(resultVertices.size());
			subMesh.meshletCount = static_cast<uint32_t>(result.meshlets.size());
			subMesh.meshletIndexStartOffset = static_cast<uint32_t>(resultIndices.size());
		}

		return result;
	}

	MeshletGenerationResult MeshProcessor::GenerateMeshlets(std::span<const Vertex> vertices, std::span<const uint32_t> indices)
	{
		constexpr size_t MAX_VERTEX_COUNT = 64;
		constexpr size_t MAX_TRIANGLE_COUNT = 128;
		constexpr float CONE_WEIGHT = 0.f;

		const uint32_t indexCount = static_cast<uint32_t>(indices.size());
		const uint32_t vertexCount = static_cast<uint32_t>(vertices.size());

		MeshletGenerationResult result{};

		const size_t maxMeshletCount = meshopt_buildMeshletsBound(indexCount, MAX_VERTEX_COUNT, MAX_TRIANGLE_COUNT);

		std::vector<meshopt_Meshlet> meshoptMeshlets(maxMeshletCount);
		std::vector<uint32_t> meshletVertexIndices(maxMeshletCount * MAX_VERTEX_COUNT);
		std::vector<uint8_t> meshletTriangles(maxMeshletCount * MAX_TRIANGLE_COUNT * 3);

		const size_t meshletCount = meshopt_buildMeshlets(meshoptMeshlets.data(), meshletVertexIndices.data(), meshletTriangles.data(), indices.data(), indexCount,
			&vertices[0].position.x, vertexCount, sizeof(Vertex), MAX_VERTEX_COUNT, MAX_TRIANGLE_COUNT, CONE_WEIGHT);

		meshoptMeshlets.resize(meshletCount);

		const auto& lastMeshlet = meshoptMeshlets.back();
		result.meshletIndices.resize(lastMeshlet.triangle_offset + (lastMeshlet.triangle_count * 3 + 3) & ~3);
		result.meshlets.resize(meshletCount);

		uint32_t meshletTriangleOffset = 0;
		for (size_t i = 0; i < meshletCount; i++)
		{
			auto& meshoptMeshlet = meshoptMeshlets.at(i);
			auto& meshlet = result.meshlets.at(i);

			meshlet.vertexOffset = meshoptMeshlet.vertex_offset;
			meshlet.vertexCount = meshoptMeshlet.vertex_count;
			meshlet.triangleOffset = meshletTriangleOffset;
			meshlet.triangleCount = static_cast<uint32_t>(meshoptMeshlet.triangle_count);

			for (uint32_t triangleIndex = 0; triangleIndex < meshlet.triangleCount; triangleIndex++)
			{
				const uint32_t meshoptTriangleOffset = meshoptMeshlet.triangle_offset + triangleIndex * 3;
				const uint32_t triangleOffset = meshlet.triangleOffset + triangleIndex * 3;

				for (uint32_t index = 0; index < 3; index++)
				{
					result.meshletIndices[triangleOffset + index] = meshletVertexIndices[meshletTriangles[meshoptTriangleOffset + index] + meshoptMeshlet.vertex_offset];
				}
			}

			meshletTriangleOffset += meshlet.triangleCount * 3;
		}

		/*uint32_t meshletTriangleOffset = 0;
		for (const auto& meshlet : meshoptMeshlets)
		{
			size_t dataOffset = result.meshletIndices.size();

			for (uint32_t i = 0; i < meshlet.vertex_count; ++i)
			{
				result.meshletIndices.push_back(meshletVertexIndices[meshlet.vertex_offset + i]);
			}

			const uint32_t* indexGroups = reinterpret_cast<const uint32_t*>(meshletTriangles.data() + meshlet.triangle_offset);
			const uint32_t indexGroupCount = (meshlet.triangle_count * 3 + 3) / 4;

			for (uint32_t i = 0; i < indexGroupCount; i++)
			{
				result.meshletIndices.push_back(indexGroups[i]);
			}

			meshopt_Bounds bounds = meshopt_computeMeshletBounds(&meshletVertexIndices[meshlet.vertex_offset], &meshletTriangles[meshlet.triangle_offset], static_cast<size_t>(meshlet.triangle_count), &vertices[0].position.x, static_cast<size_t>(vertexCount), sizeof(Vertex));

			auto& newMeshlet = result.meshlets.emplace_back();
			newMeshlet.dataOffset = static_cast<uint32_t>(dataOffset);
			newMeshlet.triangleCount = static_cast<uint8_t>(meshlet.triangle_count);
			newMeshlet.vertexCount = static_cast<uint8_t>(meshlet.vertex_count);
			newMeshlet.center = { bounds.center[0], bounds.center[1], bounds.center[2] };
			newMeshlet.radius = bounds.radius;
			newMeshlet.coneAxis[0] = bounds.cone_axis_s8[0];
			newMeshlet.coneAxis[1] = bounds.cone_axis_s8[1];
			newMeshlet.coneAxis[2] = bounds.cone_axis_s8[2];
			newMeshlet.coneCutoff = bounds.cone_cutoff_s8;

			result.meshletTriangleOffsets.emplace_back(meshletTriangleOffset);

			for (uint32_t i = meshlet.triangle_offset; i < meshlet.triangle_offset + meshlet.triangle_count * 3; i++)
			{
				result.nonOffsetedIndices.push_back(meshletVertexIndices[meshlet.vertex_offset + meshletTriangles[i]]);
			}

			meshletTriangleOffset += meshlet.triangle_count * 3;
		}*/

		return result;
	}

	MeshletGenerationResult MeshProcessor::GenerateMeshlets2(std::span<const Vertex> vertices, std::span<const uint32_t> indices)
	{
		constexpr size_t MAX_TRIANGLE_COUNT = 64;

		std::vector<idx_t> xadj;
		xadj.resize(vertices.size() + 1);

		std::vector<idx_t> adjcny;

		std::vector<std::set<uint32_t>> adjecencies;
		adjecencies.resize(vertices.size());

		for (size_t i = 0; i < indices.size(); i += 3)
		{
			uint32_t v0 = indices[i + 0];
			uint32_t v1 = indices[i + 1];
			uint32_t v2 = indices[i + 2];

			adjecencies[v0].insert(v1);
			adjecencies[v0].insert(v2);

			adjecencies[v1].insert(v0);
			adjecencies[v1].insert(v2);

			adjecencies[v2].insert(v0);
			adjecencies[v2].insert(v1);
		}

		memset(xadj.data(), 0, sizeof(idx_t) * vertices.size() + 1);

		for (size_t i = 0; i < adjecencies.size(); i++)
		{
			if (i > 0)
			{
				xadj[i] = xadj[i - 1] + static_cast<uint32_t>(adjecencies.at(i - 1).size());
			}

			for (const auto& adj : adjecencies.at(i))
			{
				adjcny.push_back(adj);
			}
		}

		xadj[vertices.size()] = static_cast<uint32_t>(adjcny.size());

		idx_t numParts = static_cast<idx_t>(Math::DivideRoundUp(indices.size(), MAX_TRIANGLE_COUNT));
		idx_t numConstraints = 1;
		idx_t edgesCut = 0;

		idx_t numVertices = static_cast<idx_t>(vertices.size());

		idx_t options[METIS_NOPTIONS];
		METIS_SetDefaultOptions(options);

		std::vector<idx_t> part(numVertices, 0);
		int r = METIS_PartGraphKway(&numVertices, &numConstraints, xadj.data(), adjcny.data(), nullptr, nullptr, nullptr, &numParts, nullptr, nullptr, nullptr, &edgesCut, part.data());

		MeshletGenerationResult result{};

		struct Range
		{
			uint32_t begin;
			uint32_t end;
		};

		if (r == METIS_OK)
		{
			std::vector<uint32_t> elementCount(numParts, 0);
			std::vector<Range> ranges(numParts);

			for (size_t i = 0; i < part.size(); i++)
			{
				elementCount[part[i]]++;
			}

			uint32_t begin = 0;
			for (int32_t partIndex = 0; partIndex < numParts; partIndex++)
			{
				ranges[partIndex] = { begin, begin + elementCount[partIndex] };
				begin += elementCount[partIndex];
				//elementCount[partIndex] = 0;
			}

			result.meshletIndices.reserve(indices.size());

			std::vector<std::vector<uint32_t>> perMeshletIndices;
			perMeshletIndices.resize(numParts);

			for (uint32_t i = 0; i < static_cast<uint32_t>(vertices.size()); ++i)
			{
				const uint32_t v0 = i;
				const uint32_t partitionIndex = part[v0];

				perMeshletIndices[partitionIndex].push_back(v0);
			}

			for (uint32_t i = 0; const auto& values : perMeshletIndices)
			{
				auto& newMeshlet = result.meshlets.emplace_back();
				newMeshlet.vertexCount = static_cast<uint8_t>(elementCount[i]);
				//newMeshlet.dataOffset = static_cast<uint32_t>(result.meshletIndices.size());
				//
				//result.meshletIndices.insert(result.meshletIndices.end(), values.begin(), values.end());

				std::unordered_map<uint32_t, uint32_t> vertexMap;
				uint32_t newIndex = 0;

				for (uint32_t index : values)
				{
					vertexMap[index] = newIndex++;
				}

				for (const uint32_t index : indices)
				{
					if (vertexMap.contains(index))
					{
						result.meshletIndices.push_back(vertexMap[index]);
					}
				}

				newMeshlet.triangleCount = static_cast<uint8_t>(result.meshletIndices.size() / 3);

				i++;
			}
		}
		else
		{
			VT_CORE_ERROR("Dum dum");
		}

		return result;
	}
}
