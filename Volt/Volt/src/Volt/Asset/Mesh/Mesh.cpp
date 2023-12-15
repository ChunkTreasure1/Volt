#include "vtpch.h"
#include "Mesh.h"

#include "Volt/Rendering/Renderer.h"
#include "Volt/RenderingNew/Resources/GlobalResourceManager.h"

#include "Volt/Math/Math.h"

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

	inline static BoundingSphere GetBoundingSphereFromVertices(const std::vector<Vertex>& vertices)
	{
		glm::vec3 minVertex(std::numeric_limits<float>::max());
		glm::vec3 maxVertex(-std::numeric_limits<float>::max());

		for (const auto& vertex : vertices)
		{
			minVertex = glm::min(minVertex, vertex.position);
			maxVertex = glm::max(maxVertex, vertex.position);
		}

		const glm::vec3 center = (minVertex + maxVertex) * 0.5f;

		float radius = 0.0f;
		for (const auto& vertex : vertices)
		{
			const float distanceSquared = glm::length2(vertex.position - center);
			if (distanceSquared > radius)
			{
				radius = distanceSquared;
			}
		}

		radius = std::sqrt(radius);

		return { center, radius };
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

		auto extractedVertices = ExtractSubMeshVertices();
		auto extractedIndices = ExtractSubMeshIndices();

		// Meshlets
		{
			//for (size_t i = 0; i < extractedVertices.size(); i++)
			//{
			//	auto& currentVertices = extractedVertices.at(i);
			//	auto& currentIndices = extractedIndices.at(i);

			//	meshopt_optimizeOverdraw(currentIndices.data(), currentIndices.data(), currentIndices.size(), &currentVertices[0].position.x, currentVertices.size(), sizeof(Vertex), 1.05f);
			//}

			constexpr size_t MAX_VERTICES = 64;
			constexpr size_t MAX_TRIANGLES = 124;
			constexpr float CONE_WEIGHT = 0.f;

			std::vector<Vertex> meshletVertices;

			for (size_t i = 0; i < extractedVertices.size(); i++)
			{
				auto& currentVertices = extractedVertices.at(i);
				auto& currentIndices = extractedIndices.at(i);

				const size_t maxMeshlets = meshopt_buildMeshletsBound(currentIndices.size(), MAX_VERTICES, MAX_TRIANGLES);

				std::vector<meshopt_Meshlet> meshlets{ maxMeshlets };
				std::vector<uint32_t> meshletVertexRemapping(maxMeshlets * MAX_VERTICES);
				std::vector<uint8_t> meshletIndices(maxMeshlets * MAX_TRIANGLES * 3);

				const size_t meshletCount = meshopt_buildMeshlets(meshlets.data(), meshletVertexRemapping.data(), meshletIndices.data(), currentIndices.data(), currentIndices.size(),
					&currentVertices[0].position.x, currentVertices.size(), sizeof(Vertex), MAX_VERTICES, MAX_TRIANGLES, CONE_WEIGHT);

				const meshopt_Meshlet& last = meshlets.at(meshletCount - 1);
				meshletVertexRemapping.resize(last.vertex_offset + last.vertex_count);
				meshletIndices.resize(last.triangle_offset + ((last.triangle_count * 3 + 3) & ~3));
				meshlets.resize(meshletCount);

				meshletVertices.resize(meshletVertices.size() + meshletVertexRemapping.size());

				for (uint32_t index = 0; index < static_cast<uint32_t>(meshletVertexRemapping.size()); index++)
				{
					meshletVertices[index] = currentVertices[meshletVertexRemapping.at(index)];
				}

				const size_t meshletIndexOffset = m_meshletIndices.size();

				for (const auto& meshlet : meshlets)
				{
					for (uint32_t index = 0; index < meshlet.triangle_count * 3; index++)
					{
						const uint32_t triIndex = meshlet.triangle_offset + index;
						m_meshletIndices.emplace_back(meshletIndices.at(triIndex));
					}
				}

				const size_t meshletOffset = m_meshlets.size();
				m_meshlets.reserve(meshletOffset + meshletCount);

				auto& subMesh = m_subMeshes.at(i);
				subMesh.meshletStartOffset = static_cast<uint32_t>(meshletOffset);
				subMesh.meshletCount = static_cast<uint32_t>(meshlets.size());
				subMesh.meshletIndexStartOffset = static_cast<uint32_t>(meshletIndexOffset);

				uint32_t meshletTriOffset = 0;
				for (const auto& meshlet : meshlets)
				{
					auto& newMeshlet = m_meshlets.emplace_back();
					newMeshlet.vertexOffset = meshlet.vertex_offset;
					newMeshlet.vertexCount = meshlet.vertex_count;
					newMeshlet.triangleOffset = meshletTriOffset;
					newMeshlet.triangleCount = meshlet.triangle_count;

					meshletTriOffset += meshlet.triangle_count * 3;
				}
			}

			m_vertices.clear();
			m_indices.clear();

			for (size_t i = 0; i < extractedVertices.size(); i++)
			{
				auto& currentIndices = extractedIndices.at(i);
				auto& currentSubMesh = m_subMeshes.at(i);

				auto& gpuMesh = m_gpuMeshes.emplace_back();
				gpuMesh.vertexStartOffset = currentSubMesh.vertexStartOffset;
				gpuMesh.meshletStartOffset = currentSubMesh.meshletStartOffset;
				gpuMesh.meshletCount = currentSubMesh.meshletCount;
				gpuMesh.meshletIndexStartOffset = currentSubMesh.meshletIndexStartOffset;
				m_indices.insert(m_indices.end(), currentIndices.begin(), currentIndices.end());
			}

			m_vertices = meshletVertices;
		}

		//// Optimize mesh
		//{
		//	for (size_t i = 0; i < extractedVertices.size(); i++)
		//	{
		//		auto& currentVertices = extractedVertices.at(i);
		//		auto& currentIndices = extractedIndices.at(i);

		//		meshopt_optimizeVertexCache(currentIndices.data(), currentIndices.data(), currentIndices.size(), currentVertices.size());
		//		meshopt_optimizeVertexFetch(currentVertices.data(), currentIndices.data(), currentIndices.size(), currentVertices.data(), currentVertices.size(), sizeof(Vertex));
		//	}
		//}

		//// Create LODs
		//{
		//	m_vertices.clear();
		//	m_indices.clear();

		//	// For every sub mesh
		//	for (size_t i = 0; i < extractedVertices.size(); i++)
		//	{
		//		auto& currentVertices = extractedVertices.at(i);
		//		auto& currentIndices = extractedIndices.at(i);
		//		auto& currentSubMesh = m_subMeshes.at(i);

		//		auto& gpuMesh = m_gpuMeshes.emplace_back();
		//		gpuMesh.vertexStartOffset = currentSubMesh.vertexStartOffset;

		//		std::vector<uint32_t> lodIndices = currentIndices;
		//		std::vector<uint32_t> resultIndices;

		//		currentSubMesh.indexStartOffset = uint32_t(m_indices.size());

		//		while (gpuMesh.lodCount < GPUMesh::MAX_LOD_COUNT)
		//		{
		//			GPUMeshLOD& lod = gpuMesh.lods[gpuMesh.lodCount++];

		//			lod.indexOffset = uint32_t(m_indices.size() + resultIndices.size());
		//			lod.indexCount = uint32_t(lodIndices.size());

		//			resultIndices.insert(resultIndices.end(), lodIndices.begin(), lodIndices.end());

		//			if (gpuMesh.lodCount < GPUMesh::MAX_LOD_COUNT)
		//			{
		//				size_t nextIndicesTarget = size_t(double(lodIndices.size()) * 0.75);
		//				size_t nextIndices = meshopt_simplify(lodIndices.data(), lodIndices.data(), lodIndices.size(), &currentVertices[0].position.x, currentVertices.size(), sizeof(Vertex), nextIndicesTarget, 1e-2f);

		//				// Error bound reached
		//				if (nextIndices == lodIndices.size())
		//				{
		//					break;
		//				}

		//				lodIndices.resize(nextIndices);
		//				meshopt_optimizeVertexCache(lodIndices.data(), lodIndices.data(), lodIndices.size(), currentVertices.size());
		//			}
		//		}

		//		m_vertices.insert(m_vertices.end(), currentVertices.begin(), currentVertices.end());
		//		m_indices.insert(m_indices.end(), resultIndices.begin(), resultIndices.end());
		//	}
		//}

		const auto encodedVertices = GetEncodedVertices();
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

		// Set all buffers in the gpu meshes
		for (auto& gpuMesh : m_gpuMeshes)
		{
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
		result.reserve(m_vertices.size());

		for (const auto& vertex : m_vertices)
		{
			result.emplace_back(vertex.position);
		}

		return result;
	}

	const std::vector<Mesh::VertexMaterialData> Mesh::GetVertexMaterialData()
	{
		std::vector<VertexMaterialData> result{};
		result.reserve(m_vertices.size());

		for (const auto& vertex : m_vertices)
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
		result.reserve(m_vertices.size());

		for (const auto& vertex : m_vertices)
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
