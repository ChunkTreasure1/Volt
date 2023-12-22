#include "vtpch.h"
#include "Mesh.h"

#include "Volt/Rendering/Renderer.h"
#include "Volt/RenderingNew/Resources/GlobalResourceManager.h"

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
		static glm::vec2 OctNormalWrap(glm::vec2 v)
		{
			glm::vec2 wrap;
			wrap.x = (1.0f - glm::abs(v.y)) * (v.x >= 0.0f ? 1.0f : -1.0f);
			wrap.y = (1.0f - glm::abs(v.x)) * (v.y >= 0.0f ? 1.0f : -1.0f);
			return wrap;
		}

		static glm::vec2 OctNormalEncode(glm::vec3 n)
		{
			n /= (glm::abs(n.x) + glm::abs(n.y) + glm::abs(n.z));

			glm::vec2 wrapped = OctNormalWrap(n);

			glm::vec2 result;
			result.x = n.z >= 0.0f ? n.x : wrapped.x;
			result.y = n.z >= 0.0f ? n.y : wrapped.y;

			result.x = result.x * 0.5f + 0.5f;
			result.y = result.y * 0.5f + 0.5f;

			return result;
		}

		// From https://www.jeremyong.com/graphics/2023/01/09/tangent-spaces-and-diamond-encoding/
		static float DiamondEncode(const glm::vec2& p)
		{
			// Project to the unit diamond, then to the x-axis.
			float x = p.x / (glm::abs(p.x) + glm::abs(p.y));

			// Contract the x coordinate by a factor of 4 to represent all 4 quadrants in
			// the unit range and remap
			float pySign = 0.f;
			if (p.y < 0.f)
			{
				pySign = -1.f;
			}
			else if (p.y > 0.f)
			{
				pySign = 1.f;
			}

			return -pySign * 0.25f * x + 0.5f + pySign * 0.25f;
		}

		// Given a normal and tangent vector, encode the tangent as a single float that can be
		// subsequently quantized.
		float EncodeTangent(const glm::vec3& normal, const glm::vec3& tangent)
		{
			// First, find a canonical direction in the tangent plane
			glm::vec3 t1;
			if (abs(normal.y) > abs(normal.z))
			{
				// Pick a canonical direction orthogonal to n with z = 0
				t1 = glm::vec3(normal.y, -normal.x, 0.f);
			}
			else
			{
				// Pick a canonical direction orthogonal to n with y = 0
				t1 = glm::vec3(normal.z, 0.f, -normal.x);
			}
			t1 = normalize(t1);

			// Construct t2 such that t1 and t2 span the plane
			glm::vec3 t2 = cross(t1, normal);

			// Decompose the tangent into two coordinates in the canonical basis
			glm::vec2 packed_tangent = glm::vec2(dot(tangent, t1), dot(tangent, t2));

			// Apply our diamond encoding to our two coordinates
			return DiamondEncode(packed_tangent);
		}
	}

	inline static BoundingSphere GetBoundingSphereFromVertices(const Vertex* vertexPtr, const size_t size)
	{
		glm::vec3 minVertex(std::numeric_limits<float>::max());
		glm::vec3 maxVertex(std::numeric_limits<float>::min());

		for (size_t i = 0; i < size; i++)
		{
			const auto& vertex = vertexPtr[i];

			minVertex = glm::min(minVertex, vertex.position);
			maxVertex = glm::max(maxVertex, vertex.position);
		}

		glm::vec3 extents = (maxVertex - minVertex) * 0.5f;
		glm::vec3 origin = extents + minVertex;

		float radius = 0.0f;
		for (size_t i = 0; i < size; i++)
		{
			const auto& vertex = vertexPtr[i];

			glm::vec3 offset = vertex.position - origin;
			float distance = offset.x * offset.x + offset.y * offset.y + offset.z * offset.z;

			radius = std::max(radius, distance);
		}

		radius = std::sqrt(radius);

		return { origin, radius };
	}

	inline static BoundingSphere GetBoundingSphereFromVertices(const std::vector<Vertex>& vertices)
	{
		return GetBoundingSphereFromVertices(vertices.data(), vertices.size());
	}

	Mesh::Mesh(std::vector<Vertex> aVertices, std::vector<uint32_t> aIndices, Ref<Material> aMaterial)
	{
		m_vertices = aVertices;
		m_indices = aIndices;

		m_material = aMaterial;

		SubMesh subMesh;
		subMesh.indexCount = (uint32_t)aIndices.size();
		subMesh.vertexCount = (uint32_t)aVertices.size();
		subMesh.vertexStartOffset = 0;
		subMesh.indexStartOffset = 0;
		subMesh.materialIndex = 0;

		subMesh.GenerateHash();

		m_subMeshes.push_back(subMesh);

		Construct();
	}

	Mesh::Mesh(std::vector<Vertex> aVertices, std::vector<uint32_t> aIndices, Ref<Material> aMaterial, const std::vector<SubMesh>& subMeshes)
	{
		m_vertices = aVertices;
		m_indices = aIndices;

		m_material = aMaterial;
		m_subMeshes = subMeshes;

		Construct();
	}

	Mesh::~Mesh()
	{
		m_vertexPositionsBuffer = nullptr;
		m_vertexMaterialBuffer = nullptr;
		m_vertexAnimationBuffer = nullptr;
		m_indexBuffer = nullptr;
	}

	void Mesh::Construct()
	{
		VT_CORE_ASSERT(!m_indices.empty() && !m_vertices.empty(), "Indices and vertices must not be empty!");

		constexpr size_t MAX_VERTEX_COUNT = 64;
		constexpr size_t MAX_TRIANGLE_COUNT = 64;
		constexpr float CONE_WEIGHT = 0.4f;

		const uint32_t subMeshCount = static_cast<uint32_t>(m_subMeshes.size());
		const uint32_t threadCount = Algo::GetThreadCountFromIterationCount(subMeshCount);
		
		std::vector<std::vector<Vertex>> perThreadVertices(threadCount);
		std::vector<std::vector<uint32_t>> perThreadIndices(threadCount);
		std::vector<std::vector<Meshlet>> perThreadMeshlets(threadCount);

		auto fu = Algo::ForEachParallelLockable([&](uint32_t threadIdx, uint32_t elementIdx) 
		{
			auto& subMesh = m_subMeshes.at(elementIdx);

			std::vector<uint32_t> tempIndices;
			tempIndices.resize(subMesh.indexCount);

			const uint32_t* indexStartPtr = &m_indices.at(subMesh.indexStartOffset);
			const Vertex* vertexStartPtr = &m_vertices.at(subMesh.vertexStartOffset);

			meshopt_optimizeOverdraw(tempIndices.data(), indexStartPtr, subMesh.indexCount, &vertexStartPtr[0].position.x, subMesh.vertexCount, sizeof(Vertex), 1.05f);
			const size_t maxMeshletCount = meshopt_buildMeshletsBound(subMesh.indexCount, MAX_VERTEX_COUNT, MAX_TRIANGLE_COUNT);

			std::vector<meshopt_Meshlet> tempMeshlets(maxMeshletCount);
			std::vector<uint32_t> meshletVertexRemap(maxMeshletCount * MAX_VERTEX_COUNT);
			std::vector<uint8_t> meshletIndices(maxMeshletCount * MAX_TRIANGLE_COUNT * 3);

			const size_t meshletCount = meshopt_buildMeshlets(tempMeshlets.data(), meshletVertexRemap.data(), meshletIndices.data(), tempIndices.data(), subMesh.indexCount,
				&vertexStartPtr[0].position.x, subMesh.vertexCount, sizeof(Vertex), MAX_VERTEX_COUNT, MAX_TRIANGLE_COUNT, CONE_WEIGHT);

			tempMeshlets.resize(meshletCount);

			const auto& lastMeshlet = tempMeshlets.back();
			meshletVertexRemap.resize(lastMeshlet.vertex_offset + lastMeshlet.vertex_count);
			meshletIndices.resize(lastMeshlet.triangle_offset + ((lastMeshlet.triangle_count * 3 + 3) & ~3));

			auto& currentVertices = perThreadVertices.at(threadIdx);
			auto& currentIndices = perThreadIndices.at(threadIdx);
			auto& currentMeshlets = perThreadMeshlets.at(threadIdx);

			const uint32_t subMeshVertexStartOffset = static_cast<uint32_t>(currentVertices.size());
			currentVertices.resize(currentVertices.size() + meshletVertexRemap.size());

			for (uint32_t i = 0; i < static_cast<uint32_t>(meshletVertexRemap.size()); i++)
			{
				currentVertices[subMeshVertexStartOffset + i] = vertexStartPtr[meshletVertexRemap.at(i)];
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

				BoundingSphere boundingSphere = GetBoundingSphereFromVertices(&currentVertices[meshlet.vertex_offset], meshlet.vertex_count);
				newMeshlet.boundingSphereCenter = boundingSphere.center;
				newMeshlet.boundingSphereRadius = boundingSphere.radius;

				triangleOffset += newMeshlet.triangleCount * 3;

				glm::vec3 triNormals[MAX_TRIANGLE_COUNT];

				for (uint32_t i = 0; i < newMeshlet.triangleCount; i++)
				{
					const auto& v0 = currentVertices.at(currentIndices.at(newMeshlet.triangleOffset + i + 0) + newMeshlet.vertexOffset);
					const auto& v1 = currentVertices.at(currentIndices.at(newMeshlet.triangleOffset + i + 1) + newMeshlet.vertexOffset);
					const auto& v2 = currentVertices.at(currentIndices.at(newMeshlet.triangleOffset + i + 2) + newMeshlet.vertexOffset);

					glm::vec3 p10 = v1.position - v0.position;
					glm::vec3 p20 = v2.position - v0.position;
				
					glm::vec3 normal = glm::cross(p10, p20);

					float area = glm::length(normal);
					float invArea = area == 0.f ? 0.f : 1.f / area;

					triNormals[i] = normal * invArea;
				}

				glm::vec3 avgNormal = 0.f;
				for (uint32_t i = 0; i < newMeshlet.triangleCount; i++)
				{
					avgNormal += triNormals[i];
				}

				float avgLength = glm::length(avgNormal);

				if (avgLength == 0.f)
				{
					avgNormal = { 1.f, 0.f, 0.f };
				}
				else
				{
					avgNormal /= avgLength;
				}

				float minDp = 1.f;

				for (uint32_t i = 0; i < newMeshlet.triangleCount; i++)
				{
					float dp = glm::dot(triNormals[i], avgNormal);

					minDp = glm::min(minDp, dp);
				}

				newMeshlet.cone = { avgNormal, minDp };
			}

		}, subMeshCount);

		for (const auto& f : fu)
		{
			f.wait();
		}

		const auto meshletPrefixSums = Algo::ElementCountPrefixSum(perThreadMeshlets);
		const auto vertexPrefixSums = Algo::ElementCountPrefixSum(perThreadVertices);
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

		std::vector<Vertex> finalVertices;
		std::vector<uint32_t> finalIndices;

		for (size_t i = 0; i < perThreadVertices.size(); i++)
		{
			m_meshlets.insert(m_meshlets.end(), perThreadMeshlets.at(i).begin(), perThreadMeshlets.at(i).end());
			finalVertices.insert(finalVertices.end(), perThreadVertices.at(i).begin(), perThreadVertices.at(i).end());
			finalIndices.insert(finalIndices.end(), perThreadIndices.at(i).begin(), perThreadIndices.at(i).end());
		}

		/*for (uint32_t si = 0; auto& subMesh : m_subMeshes)
		{
			std::vector<uint32_t> tempIndices;
			tempIndices.resize(subMesh.indexCount);

			const uint32_t* indexStartPtr = &m_indices.at(subMesh.indexStartOffset);
			const Vertex* vertexStartPtr = &m_vertices.at(subMesh.vertexStartOffset);

			meshopt_optimizeOverdraw(tempIndices.data(), indexStartPtr, subMesh.indexCount, &vertexStartPtr[0].position.x, subMesh.vertexCount, sizeof(Vertex), 1.05f);

			const size_t maxMeshletCount = meshopt_buildMeshletsBound(subMesh.indexCount, MAX_VERTEX_COUNT, MAX_TRIANGLE_COUNT);

			std::vector<meshopt_Meshlet> tempMeshlets(maxMeshletCount);
			std::vector<uint32_t> meshletVertexRemap(maxMeshletCount * MAX_VERTEX_COUNT);
			std::vector<uint8_t> meshletIndices(maxMeshletCount * MAX_TRIANGLE_COUNT * 3);
			
			const size_t meshletCount = meshopt_buildMeshlets(tempMeshlets.data(), meshletVertexRemap.data(), meshletIndices.data(), tempIndices.data(), subMesh.indexCount,
				&vertexStartPtr[0].position.x, subMesh.vertexCount, sizeof(Vertex), MAX_VERTEX_COUNT, MAX_TRIANGLE_COUNT, CONE_WEIGHT);

			tempMeshlets.resize(meshletCount);

			const auto& lastMeshlet = tempMeshlets.back();
			meshletVertexRemap.resize(lastMeshlet.vertex_offset + lastMeshlet.vertex_count);
			meshletIndices.resize(lastMeshlet.triangle_offset + ((lastMeshlet.triangle_count * 3 + 3) & ~3));

			const uint32_t subMeshVertexStartOffset = static_cast<uint32_t>(finalVertices.size());
			finalVertices.resize(finalVertices.size() + meshletVertexRemap.size());

			for (uint32_t i = 0; i < static_cast<uint32_t>(meshletVertexRemap.size()); i++)
			{
				finalVertices[subMeshVertexStartOffset + i] = vertexStartPtr[meshletVertexRemap.at(i)];
			}

			const uint32_t subMeshIndexStartOffset = static_cast<uint32_t>(finalIndices.size());

			uint32_t meshletsTotalIndexCount = 0;
			for (const auto& meshlet : tempMeshlets)
			{
				meshletsTotalIndexCount += meshlet.triangle_count * 3;
			}

			finalIndices.reserve(finalIndices.size() + meshletsTotalIndexCount);

			for (const auto& meshlet : tempMeshlets)
			{
				for (uint32_t i = 0; i < meshlet.triangle_count * 3; i++)
				{
					const uint32_t index = meshlet.triangle_offset + i;
					finalIndices.emplace_back(meshletIndices.at(index));
				}
			}

			const uint32_t subMeshMeshletStartOffset = static_cast<uint32_t>(m_meshlets.size());
			m_meshlets.reserve(m_meshlets.size() + meshletCount);

			subMesh.meshletStartOffset = subMeshMeshletStartOffset;
			subMesh.meshletCount = static_cast<uint32_t>(meshletCount);
			subMesh.meshletIndexStartOffset = subMeshIndexStartOffset;
			subMesh.meshletVertexStartOffset = subMeshVertexStartOffset;

			uint32_t triangleOffset = 0;
			for (const auto& meshlet : tempMeshlets)
			{
				auto& newMeshlet = m_meshlets.emplace_back();
				newMeshlet.vertexOffset = meshlet.vertex_offset;
				newMeshlet.vertexCount = meshlet.vertex_count;
				newMeshlet.triangleOffset = triangleOffset;
				newMeshlet.triangleCount = meshlet.triangle_count;

				BoundingSphere boundingSphere = GetBoundingSphereFromVertices(&finalVertices[meshlet.vertex_offset], meshlet.vertex_count);
				newMeshlet.boundingSphereCenter = boundingSphere.center;
				newMeshlet.boundingSphereRadius = boundingSphere.radius;

				triangleOffset += newMeshlet.triangleCount * 3;
			}

			auto& gpuMesh = m_gpuMeshes.emplace_back();
			gpuMesh.vertexStartOffset = subMesh.meshletVertexStartOffset;
			gpuMesh.meshletStartOffset = subMesh.meshletStartOffset;
			gpuMesh.meshletCount = subMesh.meshletCount;
			gpuMesh.meshletIndexStartOffset = subMesh.meshletIndexStartOffset;

			VT_CORE_TRACE("Mesh #{}", si);
			si++;
		}*/

		m_meshletIndices = finalIndices;
		m_meshletVertices = finalVertices;

		const std::string meshName = assetName;

		// Vertex positions
		{
			const auto vertexPositions = GetVertexPositions();
			m_vertexPositionsBuffer = GlobalResource<RHI::StorageBuffer>::Create(RHI::StorageBuffer::Create(static_cast<uint32_t>(vertexPositions.size()), sizeof(glm::vec3), "Vertex Positions - " + meshName));
			m_vertexPositionsBuffer->GetResource()->SetData(vertexPositions.data(), vertexPositions.size() * sizeof(glm::vec3));
		}

		// Vertex material data
		{
			const auto vertexMaterialData = GetVertexMaterialData();
			m_vertexMaterialBuffer = GlobalResource<RHI::StorageBuffer>::Create(RHI::StorageBuffer::Create(static_cast<uint32_t>(vertexMaterialData.size()), sizeof(VertexMaterialData), "Vertex Material Data - " + meshName));
			m_vertexMaterialBuffer->GetResource()->SetData(vertexMaterialData.data(), vertexMaterialData.size() * sizeof(VertexMaterialData));
		}

		// Vertex animation data
		{
			const auto vertexAnimationData = GetVertexAnimationData();
			m_vertexAnimationBuffer = GlobalResource<RHI::StorageBuffer>::Create(RHI::StorageBuffer::Create(static_cast<uint32_t>(vertexAnimationData.size()), sizeof(VertexAnimationData), "Vertex Animation Data - " + meshName));
			m_vertexAnimationBuffer->GetResource()->SetData(vertexAnimationData.data(), vertexAnimationData.size() * sizeof(VertexAnimationData));
		}

		// Indices
		{
			m_indexBuffer = GlobalResource<RHI::StorageBuffer>::Create(RHI::StorageBuffer::Create(static_cast<uint32_t>(m_indices.size()), sizeof(uint32_t), "Index Buffer - " + meshName));
			m_indexBuffer->GetResource()->SetData(m_indices.data(), m_indices.size() * sizeof(uint32_t));
		}

		// Meshlet Triangles
		{
			m_meshletIndexBuffer = GlobalResource<RHI::StorageBuffer>::Create(RHI::StorageBuffer::Create(static_cast<uint32_t>(m_meshletIndices.size()), sizeof(uint32_t), "Meshlet Indices Buffer"));
			m_meshletIndexBuffer->GetResource()->SetData(m_meshletIndices.data(), m_meshletIndices.size() * sizeof(uint32_t));
		}

		// Meshlets
		{
			m_meshletsBuffer = GlobalResource<RHI::StorageBuffer>::Create(RHI::StorageBuffer::Create(static_cast<uint32_t>(m_meshlets.size()), sizeof(Meshlet), "Meshlet Buffer"));
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
			gpuMesh.vertexAnimationBuffer = m_vertexAnimationBuffer->GetResourceHandle();
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

		for (const auto& vertex : m_vertices)
		{
			const auto scaledPos = vertex.position * m_averageScale;

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
		m_boundingSphere = GetBoundingSphereFromVertices(m_vertices);

		for (uint32_t i = 0; auto & subMesh : m_subMeshes)
		{
			std::vector<Vertex> subMeshVertices;
			subMeshVertices.insert(subMeshVertices.end(), std::next(m_vertices.begin(), subMesh.vertexStartOffset), std::next(m_vertices.begin(), subMesh.vertexStartOffset + subMesh.vertexCount));

			m_subMeshBoundingSpheres[i] = GetBoundingSphereFromVertices(subMeshVertices);
			i++;
		}
	}

	const std::vector<EncodedVertex> Mesh::GetEncodedVertices() const
	{
		std::vector<EncodedVertex> result{};
		result.reserve(m_vertices.size());

		for (const auto& vertex : m_vertices)
		{
			auto& encodedVertex = result.emplace_back();

			encodedVertex.position = vertex.position;

			// Encode normal
			{
				const auto octNormal = Utility::OctNormalEncode(vertex.normal);

				encodedVertex.normal.x = uint8_t(octNormal.x * 255);
				encodedVertex.normal.y = uint8_t(octNormal.y * 255);
			}

			// Encode tangent
			{
				encodedVertex.tangent = Utility::EncodeTangent(vertex.normal, vertex.tangent);
			}

			// Encode color
			{
				// #TODO_Ivar: Implement
			}

			// Tex coords
			{
				encodedVertex.texCoords[0] = static_cast<half_float::half>(vertex.texCoords.x);
				encodedVertex.texCoords[1] = static_cast<half_float::half>(vertex.texCoords.y);
			}

			// Influences
			{
				encodedVertex.influences.x = static_cast<uint16_t>(vertex.influences.x);
				encodedVertex.influences.y = static_cast<uint16_t>(vertex.influences.y);
				encodedVertex.influences.z = static_cast<uint16_t>(vertex.influences.z);
				encodedVertex.influences.w = static_cast<uint16_t>(vertex.influences.w);
			}

			// Weights
			{
				encodedVertex.weights[0] = static_cast<half_float::half>(vertex.weights.x);
				encodedVertex.weights[1] = static_cast<half_float::half>(vertex.weights.y);
				encodedVertex.weights[2] = static_cast<half_float::half>(vertex.weights.z);
				encodedVertex.weights[3] = static_cast<half_float::half>(vertex.weights.w);
			}
		}

		return result;
	}
	std::vector<std::vector<Vertex>> Mesh::ExtractSubMeshVertices()
	{
		std::vector<std::vector<Vertex>> result{};

		for (const auto& subMesh : m_subMeshes)
		{
			auto& vertices = result.emplace_back();

			for (uint32_t i = 0; i < subMesh.vertexCount; i++)
			{
				vertices.emplace_back(m_vertices.at(i + subMesh.vertexStartOffset));
			}
		}

		return result;
	}
	std::vector<std::vector<uint32_t>> Mesh::ExtractSubMeshIndices()
	{
		std::vector<std::vector<uint32_t>> result{};

		for (const auto& subMesh : m_subMeshes)
		{
			auto& indices = result.emplace_back();

			for (uint32_t i = 0; i < subMesh.indexCount; i++)
			{
				indices.emplace_back(m_indices.at(i + subMesh.indexStartOffset));
			}
		}

		return result;
	}

	const std::vector<glm::vec3> Mesh::GetVertexPositions()
	{
		std::vector<glm::vec3> result{};
		result.reserve(m_meshletVertices.size());

		for (const auto& vertex : m_meshletVertices)
		{
			result.emplace_back(vertex.position);
		}

		return result;
	}

	const std::vector<Mesh::VertexMaterialData> Mesh::GetVertexMaterialData()
	{
		std::vector<VertexMaterialData> result{};
		result.reserve(m_meshletVertices.size());

		for (const auto& vertex : m_meshletVertices)
		{
			auto& data = result.emplace_back();
			
			const auto octNormal = Utility::OctNormalEncode(vertex.normal);

			data.normal.x = uint8_t(octNormal.x * 255);
			data.normal.y = uint8_t(octNormal.y * 255);
			data.tangent = Utility::EncodeTangent(vertex.normal, vertex.tangent);
			data.texCoords.x = static_cast<half_float::half>(vertex.texCoords.x);
			data.texCoords.y = static_cast<half_float::half>(vertex.texCoords.y);
		}

		return result;
	}

	const std::vector<Mesh::VertexAnimationData> Mesh::GetVertexAnimationData()
	{
		std::vector<VertexAnimationData> result{};
		result.reserve(m_meshletVertices.size());

		for (const auto& vertex : m_meshletVertices)
		{
			auto& data = result.emplace_back();
			
			// Influences
			{
				data.influences.x = static_cast<uint16_t>(vertex.influences.x);
				data.influences.y = static_cast<uint16_t>(vertex.influences.y);
				data.influences.z = static_cast<uint16_t>(vertex.influences.z);
				data.influences.w = static_cast<uint16_t>(vertex.influences.w);
			}

			// Weights
			{
				data.weights[0] = static_cast<half_float::half>(vertex.weights.x);
				data.weights[1] = static_cast<half_float::half>(vertex.weights.y);
				data.weights[2] = static_cast<half_float::half>(vertex.weights.z);
				data.weights[3] = static_cast<half_float::half>(vertex.weights.w);
			}
		}

		return result;
	}
}
