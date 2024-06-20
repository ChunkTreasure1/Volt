#include "vtpch.h"
#include "Mesh.h"

#include "Volt/Asset/Rendering/Material.h"
#include "Volt/Rendering/Mesh/MeshCommon.h"

#include "Volt/Math/Math.h"
#include "Volt/Utility/Algorithms.h"

#include <VoltRHI/Buffers/VertexBuffer.h>
#include <VoltRHI/Buffers/IndexBuffer.h>
#include <VoltRHI/Buffers/StorageBuffer.h>

#include <meshoptimizer/meshoptimizer.h>

namespace Volt
{
	namespace Utility
	{
		VT_NODISCARD std::vector<uint32_t> ElementCountPrefixSum(const std::vector<VertexContainer>& elements)
		{
			std::vector<uint32_t> prefixSums(elements.size());

			prefixSums.resize(elements.size());
			prefixSums[0] = 0;

			for (size_t i = 1; i < elements.size(); i++)
			{
				size_t sum = 0;

				for (size_t j = 0; j < i; j++)
				{
					sum += elements.at(j).positions.size();
				}

				prefixSums[i] = static_cast<uint32_t>(sum);
			}

			return prefixSums;
		}
	}

	inline static BoundingSphere GetBoundingSphereFromVertices(const glm::vec3* vertexPtr, const size_t size)
	{
		glm::vec3 minVertex(std::numeric_limits<float>::max());
		glm::vec3 maxVertex(std::numeric_limits<float>::min());

		for (size_t i = 0; i < size; i++)
		{
			const auto& vertex = vertexPtr[i];

			minVertex = glm::min(minVertex, vertex);
			maxVertex = glm::max(maxVertex, vertex);
		}

		glm::vec3 extents = (maxVertex - minVertex) * 0.5f;
		glm::vec3 origin = extents + minVertex;

		float radius = 0.0f;
		for (size_t i = 0; i < size; i++)
		{
			const auto& vertex = vertexPtr[i];

			glm::vec3 offset = vertex - origin;
			float distance = offset.x * offset.x + offset.y * offset.y + offset.z * offset.z;

			radius = std::max(radius, distance);
		}

		radius = std::sqrt(radius);

		return { origin, radius };
	}

	inline static BoundingSphere GetBoundingSphereFromVertices(const std::vector<glm::vec3>& vertices)
	{
		return GetBoundingSphereFromVertices(vertices.data(), vertices.size());
	}

	Mesh::Mesh(std::vector<Vertex> aVertices, std::vector<uint32_t> aIndices, Ref<Material> aMaterial)
	{
		InitializeWithVertices(aVertices);
		m_indices = aIndices;

		m_materialTable.SetMaterial(aMaterial->handle, 0);

		SubMesh& subMesh = m_subMeshes.emplace_back();
		subMesh.indexCount = (uint32_t)aIndices.size();
		subMesh.vertexCount = (uint32_t)aVertices.size();
		subMesh.vertexStartOffset = 0;
		subMesh.indexStartOffset = 0;
		subMesh.materialIndex = 0;

		subMesh.GenerateHash();

		Construct();
	}

	Mesh::Mesh(std::vector<Vertex> aVertices, std::vector<uint32_t> aIndices, const MaterialTable& materialTable, const std::vector<SubMesh>& subMeshes)
	{
		InitializeWithVertices(aVertices);
		m_indices = aIndices;

		m_materialTable = materialTable;
		m_subMeshes = subMeshes;

		Construct();
	}

	Mesh::~Mesh()
	{
		m_vertexPositionsBuffer = nullptr;
		m_vertexMaterialBuffer = nullptr;
		m_vertexAnimationInfoBuffer = nullptr;
		m_indexBuffer = nullptr;
	}

	void Mesh::Construct()
	{
		VT_CORE_ASSERT(!m_indices.empty() && !m_vertexContainer.positions.empty(), "Indices and vertices must not be empty!");

		constexpr size_t MAX_VERTEX_COUNT = 64;
		constexpr size_t MAX_TRIANGLE_COUNT = 64;
		constexpr float CONE_WEIGHT = 0.f;

		const uint32_t subMeshCount = static_cast<uint32_t>(m_subMeshes.size());
		const uint32_t threadCount = Algo::GetThreadCountFromIterationCount(subMeshCount);
		
		std::vector<VertexContainer> perThreadVertices(threadCount);
		std::vector<std::vector<uint32_t>> perThreadIndices(threadCount);
		std::vector<std::vector<Meshlet>> perThreadMeshlets(threadCount);

		auto fu = Algo::ForEachParallelLockable([&](uint32_t threadIdx, uint32_t elementIdx) 
		{
			auto& subMesh = m_subMeshes.at(elementIdx);

			std::vector<uint32_t> tempIndices;
			tempIndices.resize(subMesh.indexCount);

			const uint32_t* indexStartPtr = &m_indices.at(subMesh.indexStartOffset);
			const glm::vec3* vertexPositionStartPtr = &m_vertexContainer.positions.at(subMesh.vertexStartOffset);
			const VertexMaterialData* vertexMaterialDataStartPtr = &m_vertexContainer.materialData.at(subMesh.vertexStartOffset);
			const VertexAnimationInfo* vertexBoneInfluenceStartPtr = &m_vertexContainer.vertexAnimationInfo.at(subMesh.vertexStartOffset);

			meshopt_optimizeOverdraw(tempIndices.data(), indexStartPtr, subMesh.indexCount, &vertexPositionStartPtr[0].x, subMesh.vertexCount, sizeof(glm::vec3), 1.05f);
			const size_t maxMeshletCount = meshopt_buildMeshletsBound(subMesh.indexCount, MAX_VERTEX_COUNT, MAX_TRIANGLE_COUNT);

			std::vector<meshopt_Meshlet> tempMeshlets(maxMeshletCount);
			std::vector<uint32_t> meshletVertexRemap(maxMeshletCount * MAX_VERTEX_COUNT);
			std::vector<uint8_t> meshletIndices(maxMeshletCount * MAX_TRIANGLE_COUNT * 3);

			const size_t meshletCount = meshopt_buildMeshlets(tempMeshlets.data(), meshletVertexRemap.data(), meshletIndices.data(), tempIndices.data(), subMesh.indexCount,
				&vertexPositionStartPtr[0].x, subMesh.vertexCount, sizeof(glm::vec3), MAX_VERTEX_COUNT, MAX_TRIANGLE_COUNT, CONE_WEIGHT);

			tempMeshlets.resize(meshletCount);

			const auto& lastMeshlet = tempMeshlets.back();
			meshletVertexRemap.resize(lastMeshlet.vertex_offset + lastMeshlet.vertex_count);
			meshletIndices.resize(lastMeshlet.triangle_offset + ((lastMeshlet.triangle_count * 3 + 3) & ~3));

			auto& currentVertices = perThreadVertices.at(threadIdx);
			auto& currentIndices = perThreadIndices.at(threadIdx);
			auto& currentMeshlets = perThreadMeshlets.at(threadIdx);

			const uint32_t subMeshVertexStartOffset = static_cast<uint32_t>(currentVertices.Size());
			currentVertices.Resize(currentVertices.Size() + meshletVertexRemap.size());

			for (uint32_t i = 0; i < static_cast<uint32_t>(meshletVertexRemap.size()); i++)
			{
				currentVertices.positions[subMeshVertexStartOffset + i] = vertexPositionStartPtr[meshletVertexRemap.at(i)];
				currentVertices.materialData[subMeshVertexStartOffset + i] = vertexMaterialDataStartPtr[meshletVertexRemap.at(i)];
				currentVertices.vertexAnimationInfo[subMeshVertexStartOffset + i] = vertexBoneInfluenceStartPtr[meshletVertexRemap.at(i)];
			}

			const uint32_t subMeshIndexStartOffset = static_cast<uint32_t>(currentIndices.size());

			uint32_t meshletsTotalIndexCount = 0;
			for (const auto& meshlet : tempMeshlets)
			{
				meshletsTotalIndexCount += meshlet.triangle_count * 3;
			}

			currentIndices.reserve(currentIndices.size() + meshletsTotalIndexCount);

			for (const auto& meshlet : tempMeshlets)
			{
				for (uint32_t i = 0; i < meshlet.triangle_count * 3; i++)
				{
					const uint32_t index = meshlet.triangle_offset + i;
					currentIndices.emplace_back(meshletIndices.at(index));
				}
			}

			const uint32_t subMeshMeshletStartOffset = static_cast<uint32_t>(currentMeshlets.size());
			currentMeshlets.reserve(currentMeshlets.size() + meshletCount);

			subMesh.meshletStartOffset = subMeshMeshletStartOffset;
			subMesh.meshletCount = static_cast<uint32_t>(meshletCount);
			subMesh.meshletIndexStartOffset = subMeshIndexStartOffset;
			subMesh.meshletVertexStartOffset = subMeshVertexStartOffset;

			uint32_t triangleOffset = 0;
			for (const auto& meshlet : tempMeshlets)
			{
				auto& newMeshlet = currentMeshlets.emplace_back();
				newMeshlet.vertexOffset = meshlet.vertex_offset;
				newMeshlet.vertexCount = meshlet.vertex_count;
				newMeshlet.triangleOffset = triangleOffset;
				newMeshlet.triangleCount = meshlet.triangle_count;

				BoundingSphere boundingSphere = GetBoundingSphereFromVertices(&currentVertices.positions[meshlet.vertex_offset], meshlet.vertex_count);
				newMeshlet.boundingSphereCenter = boundingSphere.center;
				newMeshlet.boundingSphereRadius = boundingSphere.radius;

				triangleOffset += newMeshlet.triangleCount * 3;
			}

		}, subMeshCount);

		for (const auto& f : fu)
		{
			f.wait();
		}

		const auto meshletPrefixSums = Algo::ElementCountPrefixSum(perThreadMeshlets);
		const auto vertexPrefixSums = Utility::ElementCountPrefixSum(perThreadVertices);
		const auto indexPrefixSums = Algo::ElementCountPrefixSum(perThreadIndices);

		auto fu2 = Algo::ForEachParallelLockable([&](uint32_t threadIdx, uint32_t elementIdx)
		{
			auto& subMesh = m_subMeshes.at(elementIdx);
			
			subMesh.meshletStartOffset += meshletPrefixSums.at(threadIdx);
			subMesh.meshletVertexStartOffset += vertexPrefixSums.at(threadIdx);
			subMesh.meshletIndexStartOffset += indexPrefixSums.at(threadIdx);

		}, subMeshCount);

		for (const auto& f : fu2)
		{
			f.wait();
		}

		std::vector<uint32_t> finalIndices;
		VertexContainer finalVertexContainer;

		for (size_t i = 0; i < perThreadVertices.size(); i++)
		{
			m_meshlets.insert(m_meshlets.end(), perThreadMeshlets.at(i).begin(), perThreadMeshlets.at(i).end());
			finalVertexContainer.positions.insert(finalVertexContainer.positions.end(), perThreadVertices.at(i).positions.begin(), perThreadVertices.at(i).positions.end());
			finalVertexContainer.materialData.insert(finalVertexContainer.materialData.end(), perThreadVertices.at(i).materialData.begin(), perThreadVertices.at(i).materialData.end());
			finalVertexContainer.vertexAnimationInfo.insert(finalVertexContainer.vertexAnimationInfo.end(), perThreadVertices.at(i).vertexAnimationInfo.begin(), perThreadVertices.at(i).vertexAnimationInfo.end());
			finalIndices.insert(finalIndices.end(), perThreadIndices.at(i).begin(), perThreadIndices.at(i).end());
		}

		m_meshletIndices = finalIndices;
		m_meshletVertexContainer = finalVertexContainer;

		const std::string meshName = assetName;

		// Vertex positions
		{
			const auto& vertexPositions = m_meshletVertexContainer.positions;
			m_vertexPositionsBuffer = BindlessResource<RHI::StorageBuffer>::CreateRef(static_cast<uint32_t>(vertexPositions.size()), sizeof(glm::vec3), "Vertex Positions - " + meshName);
			m_vertexPositionsBuffer->GetResource()->SetData(vertexPositions.data(), vertexPositions.size() * sizeof(glm::vec3));
		}

		// Vertex material data
		{
			const auto& vertexMaterialData = m_meshletVertexContainer.materialData;
			m_vertexMaterialBuffer = BindlessResource<RHI::StorageBuffer>::CreateRef(static_cast<uint32_t>(vertexMaterialData.size()), sizeof(VertexMaterialData), "Vertex Material Data - " + meshName);
			m_vertexMaterialBuffer->GetResource()->SetData(vertexMaterialData.data(), vertexMaterialData.size() * sizeof(VertexMaterialData));
		}

		// Vertex animation info
		{
			const auto& vertexAnimationInfo = m_meshletVertexContainer.vertexAnimationInfo;
			m_vertexAnimationInfoBuffer = BindlessResource<RHI::StorageBuffer>::CreateRef(static_cast<uint32_t>(vertexAnimationInfo.size()), sizeof(VertexAnimationInfo), "Vertex Animation Info - " + meshName);
			m_vertexAnimationInfoBuffer->GetResource()->SetData(vertexAnimationInfo.data(), vertexAnimationInfo.size() * sizeof(VertexAnimationInfo));
		}

		// Vertex bone influences
		{
			const auto& vertexBoneInfluences = m_vertexContainer.vertexBoneInfluences;
			if (!vertexBoneInfluences.empty())
			{
				m_vertexBoneInfluencesBuffer = BindlessResource<RHI::StorageBuffer>::CreateRef(static_cast<uint32_t>(vertexBoneInfluences.size()), sizeof(uint16_t), "Vertex Bone Influences - " + meshName);
				m_vertexBoneInfluencesBuffer->GetResource()->SetData(vertexBoneInfluences.data(), vertexBoneInfluences.size() * sizeof(uint16_t));
			}
		}

		// Vertex bone weights
		{
			const auto& vertexBoneWeights = m_vertexContainer.vertexBoneWeights;
			if (!vertexBoneWeights.empty())
			{
				m_vertexBoneWeightsBuffer = BindlessResource<RHI::StorageBuffer>::CreateRef(static_cast<uint32_t>(vertexBoneWeights.size()), sizeof(float), "Vertex Bone Weights - " + meshName);
				m_vertexBoneWeightsBuffer->GetResource()->SetData(vertexBoneWeights.data(), vertexBoneWeights.size() * sizeof(float));
			}
		}

		// Indices
		{
			m_indexBuffer = BindlessResource<RHI::StorageBuffer>::CreateRef(static_cast<uint32_t>(m_indices.size()), sizeof(uint32_t), "Index Buffer - " + meshName, RHI::BufferUsage::StorageBuffer | RHI::BufferUsage::IndexBuffer);
			m_indexBuffer->GetResource()->SetData(m_indices.data(), m_indices.size() * sizeof(uint32_t));
		}

		// Meshlet Triangles
		{
			m_meshletIndexBuffer = BindlessResource<RHI::StorageBuffer>::CreateRef(static_cast<uint32_t>(m_meshletIndices.size()), sizeof(uint32_t), "Meshlet Indices Buffer");
			m_meshletIndexBuffer->GetResource()->SetData(m_meshletIndices.data(), m_meshletIndices.size() * sizeof(uint32_t));
		}

		// Meshlets
		{
			m_meshletsBuffer = BindlessResource<RHI::StorageBuffer>::CreateRef(static_cast<uint32_t>(m_meshlets.size()), sizeof(Meshlet), "Meshlet Buffer");
			m_meshletsBuffer->GetResource()->SetData(m_meshlets.data(), m_meshlets.size() * sizeof(Meshlet));
		}

		// Create GPU Meshes
		for (const auto& subMesh : m_subMeshes)
		{
			auto& gpuMesh = m_gpuMeshes.emplace_back();
			gpuMesh.vertexStartOffset = subMesh.meshletVertexStartOffset;
			gpuMesh.meshletStartOffset = subMesh.meshletStartOffset;
			gpuMesh.meshletCount = subMesh.meshletCount;
			gpuMesh.meshletIndexStartOffset = subMesh.meshletIndexStartOffset;
			gpuMesh.vertexPositionsBuffer = m_vertexPositionsBuffer->GetResourceHandle();
			gpuMesh.vertexMaterialBuffer = m_vertexMaterialBuffer->GetResourceHandle();
			gpuMesh.vertexAnimationInfoBuffer = m_vertexAnimationInfoBuffer->GetResourceHandle();
			gpuMesh.vertexBoneWeightsBuffer = m_vertexBoneWeightsBuffer ? m_vertexBoneWeightsBuffer->GetResourceHandle() : Resource::Invalid;
			gpuMesh.vertexBoneInfluencesBuffer = m_vertexBoneInfluencesBuffer ? m_vertexBoneInfluencesBuffer->GetResourceHandle() : Resource::Invalid;
			gpuMesh.indexBuffer = m_indexBuffer->GetResourceHandle();
			gpuMesh.meshletsBuffer = m_meshletsBuffer->GetResourceHandle();
			gpuMesh.meshletIndexBuffer = m_meshletIndexBuffer->GetResourceHandle();
		}

		for (auto& subMesh : m_subMeshes)
		{
			glm::vec3 t, r, s;
			Math::Decompose(subMesh.transform, t, r, s);

			m_averageScale += s;
		}

		m_averageScale /= (float)m_subMeshes.size();

		glm::vec3 min = { FLT_MAX, FLT_MAX, FLT_MAX };
		glm::vec3 max = { -FLT_MAX, -FLT_MAX, -FLT_MAX };

		for (const auto& vertex : m_vertexContainer.positions)
		{
			const auto scaledPos = vertex * m_averageScale;

			if (glm::all(glm::lessThan(scaledPos, min)))
			{
				min = scaledPos;
			}

			if (glm::all(glm::greaterThan(scaledPos, max)))
			{
				max = scaledPos * m_averageScale;
			}
		}

		m_boundingBox = BoundingBox{ max, min };
		m_boundingSphere = GetBoundingSphereFromVertices(m_vertexContainer.positions);

		for (uint32_t i = 0; auto & subMesh : m_subMeshes)
		{
			std::vector<glm::vec3> subMeshVertices;
			subMeshVertices.insert(subMeshVertices.end(), std::next(m_vertexContainer.positions.begin(), subMesh.vertexStartOffset), std::next(m_vertexContainer.positions.begin(), subMesh.vertexStartOffset + subMesh.vertexCount));

			m_subMeshBoundingSpheres[i] = GetBoundingSphereFromVertices(subMeshVertices);
			i++;
		}
	}

	void Mesh::SetMaterial(Ref<Material> material, uint32_t index)
	{
		m_materialTable.SetMaterial(material->handle, index);
	}

	VertexMaterialData Mesh::GetMaterialDataFromVertex(const Vertex& vertex)
	{
		VertexMaterialData result;
		const auto octNormal = Utility::OctNormalEncode(vertex.normal);

		result.normal.x = uint8_t(octNormal.x * 255);
		result.normal.y = uint8_t(octNormal.y * 255);
		result.tangent = Utility::EncodeTangent(vertex.normal, vertex.tangent);
		result.texCoords.x = static_cast<half_float::half>(vertex.uv.x);
		result.texCoords.y = static_cast<half_float::half>(vertex.uv.y);

		return result;
	}

	void Mesh::InitializeWithVertices(const std::vector<Vertex>& vertices)
	{
		m_vertexContainer.Resize(vertices.size());

		for (uint32_t i = 0; const auto& vertex : vertices)
		{
			m_vertexContainer.positions[i] = vertex.position;
			m_vertexContainer.materialData[i] = GetMaterialDataFromVertex(vertex);
			i++;
		}
	}
}
