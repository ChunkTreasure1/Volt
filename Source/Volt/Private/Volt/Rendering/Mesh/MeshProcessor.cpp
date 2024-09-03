#include "vtpch.h"
#include "Volt/Rendering/Mesh/MeshProcessor.h"

#include "Volt/Math/Math.h"
#include "Volt/Utility/Algorithms.h"

#include <meshoptimizer/meshoptimizer.h>

#include <metis.h>
#include <unordered_set>

namespace Volt
{
	ProcessedMeshResult MeshProcessor::ProcessMesh(const Vector<Vertex>& vertices, const Vector<uint32_t>& indices, const MaterialTable& materialTable, const Vector<SubMesh>& subMeshes)
	{
		Vector<Vertex> resultVertices;
		Vector<uint32_t> resultIndices;
		Vector<Meshlet> resultMeshlets;
		Vector<SubMesh> resultSubMeshes = subMeshes;

		ProcessedMeshResult result;

		for (auto& subMesh : resultSubMeshes)
		{
			Vector<uint32_t> tempIndices;
			tempIndices.resize(subMesh.indexCount);

			const uint32_t* indexStartPtr = &indices.at(subMesh.indexStartOffset);
			const Vertex* vertexStartPtr = &vertices.at(subMesh.vertexStartOffset);

			meshopt_optimizeOverdraw(tempIndices.data(), indexStartPtr, subMesh.indexCount, &vertexStartPtr[0].position.x, subMesh.vertexCount, sizeof(Vertex), 1.05f);
			
			auto lod0MeshletData = GenerateMeshlets2(std::span<const Vertex>(vertexStartPtr, subMesh.vertexCount), std::span<const uint32_t>(indexStartPtr, subMesh.indexCount));

			result.meshlets = lod0MeshletData.meshlets;
			result.meshletIndices = lod0MeshletData.meshletIndices;

			subMesh.meshletStartOffset = static_cast<uint32_t>(resultMeshlets.size());
			subMesh.meshletIndexStartOffset = static_cast<uint32_t>(resultIndices.size());
			subMesh.meshletVertexStartOffset = static_cast<uint32_t>(resultVertices.size());
			subMesh.meshletCount = static_cast<uint32_t>(lod0MeshletData.meshlets.size());

			{
				// Find meshlets who have adjacent triangles
				// Find adjacent triangle edge count

				struct Count
				{
					uint32_t count = 0;
				};

				Vector<std::unordered_set<Edge>> meshletBorderEdges;
				meshletBorderEdges.resize(lod0MeshletData.meshlets.size());

				//for (uint32_t meshletIndex = 0; const auto& meshlet : lod0MeshletData.meshlets)
				//{
				//	std::unordered_map<Edge, Count> edgeUsages;

				//	// A triangle is formed from i+0, i+1, i+2
				//	//for (uint32_t i = meshlet.triangleOffset; i < meshlet.triangleOffset + meshlet.triangleCount * 3; i += 3)
				//	//{
				//	//	for (uint32_t j = 0; j < 3; j++)
				//	//	{
				//	//		uint32_t v1 = lod0MeshletData.nonOffsetedIndices.at(i + j);
				//	//		uint32_t v2 = lod0MeshletData.nonOffsetedIndices.at(i + (j + 1) % 3);

				//	//		if (v1 > v2)
				//	//		{
				//	//			std::swap(v1, v2);
				//	//		}

				//	//		edgeUsages[{ v1, v2 }].count++;
				//	//	}
				//	//}

				//	std::unordered_set<Edge>& borderEdges = meshletBorderEdges[meshletIndex];
				//	for (const auto& [edge, count] : edgeUsages)
				//	{
				//		if (count.count == 1)
				//		{
				//			borderEdges.insert(edge);
				//		}
				//	}

				//	meshletIndex++;
				//}

				std::unordered_map<size_t, std::unordered_map<size_t, Count>> meshletAdjecencyCount;

				for (size_t i = 0; i < 1; i++)
				{
					const auto& outerEdges = meshletBorderEdges.at(i);

					for (const auto& outerEdge : outerEdges)
					{
						for (size_t j = i + 1; j < meshletBorderEdges.size(); j++)
						{
							const auto& innerEdges = meshletBorderEdges.at(j);
							if (innerEdges.contains(outerEdge))
							{
								meshletAdjecencyCount[i][j].count++;
							}
						}
					}
				}
			}
		}

		return result;
	}

	MeshletGenerationResult MeshProcessor::GenerateMeshlets(const Vertex* vertices, const uint32_t* indices, uint32_t indexCount, uint32_t vertexCount)
	{
		constexpr size_t MAX_VERTEX_COUNT = 64;
		constexpr size_t MAX_TRIANGLE_COUNT = 64;
		constexpr float CONE_WEIGHT = 0.f;

		MeshletGenerationResult result{};

		const size_t maxMeshletCount = meshopt_buildMeshletsBound(indexCount, MAX_VERTEX_COUNT, MAX_TRIANGLE_COUNT);

		Vector<meshopt_Meshlet> tempMeshlets(maxMeshletCount);
		Vector<uint32_t> tempVertices(maxMeshletCount * MAX_VERTEX_COUNT);
		Vector<uint8_t> tempTriangles(maxMeshletCount * MAX_TRIANGLE_COUNT	* 3);

		const size_t meshletCount = meshopt_buildMeshlets(tempMeshlets.data(), tempVertices.data(), tempTriangles.data(), indices, indexCount,
			&vertices[0].position.x, vertexCount, sizeof(Vertex), MAX_VERTEX_COUNT, MAX_TRIANGLE_COUNT, CONE_WEIGHT);

		tempMeshlets.resize(meshletCount);

		for (const auto& meshlet : tempMeshlets)
		{
			size_t dataOffset = result.meshletIndices.size();

			for (uint32_t i = 0; i < meshlet.vertex_count; ++i)
			{
				result.meshletIndices.push_back(tempVertices[meshlet.vertex_offset + i]);
			}

			const uint32_t* indexGroups = reinterpret_cast<const uint32_t*>(tempTriangles.data() + meshlet.triangle_offset);
			const uint32_t indexGroupCount = (meshlet.triangle_count * 3 + 3) / 4;

			for (uint32_t i = 0; i < indexGroupCount; i++)
			{
				result.meshletIndices.push_back(indexGroups[i]);
			}

			meshopt_Bounds bounds = meshopt_computeMeshletBounds(&tempVertices[meshlet.vertex_offset], &tempTriangles[meshlet.triangle_offset], static_cast<size_t>(meshlet.triangle_count), &vertices[0].position.x, static_cast<size_t>(vertexCount), sizeof(Vertex));
		
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

			for (uint32_t i = meshlet.triangle_offset; i < meshlet.triangle_offset + meshlet.triangle_count * 3; i++)
			{
				result.nonOffsetedIndices.push_back(tempVertices[meshlet.vertex_offset + tempTriangles[i]]);
			}
		}

		return result;
	}

	MeshletGenerationResult MeshProcessor::GenerateMeshlets2(std::span<const Vertex> vertices, std::span<const uint32_t> indices)
	{
		constexpr size_t MAX_TRIANGLE_COUNT = 64;

		Vector<idx_t> xadj;
		xadj.resize(vertices.size() + 1);

		Vector<idx_t> adjcny;

		Vector<std::set<uint32_t>> adjecencies;
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

		Vector<idx_t> part(numVertices, 0);
		int r = METIS_PartGraphKway(&numVertices, &numConstraints, xadj.data(), adjcny.data(), nullptr, nullptr, nullptr, &numParts, nullptr, nullptr, nullptr, &edgesCut, part.data());

		MeshletGenerationResult result{};

		struct Range
		{
			uint32_t begin;
			uint32_t end;
		};

		if (r == METIS_OK)
		{
			Vector<uint32_t> elementCount(numParts, 0);
			Vector<Range> ranges(numParts);

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

			Vector<Vector<uint32_t>> perMeshletIndices;
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
				newMeshlet.dataOffset = static_cast<uint32_t>(result.meshletIndices.size());
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
			VT_LOG(Error, "Dum dum");
		}

		return result;
	}
}
