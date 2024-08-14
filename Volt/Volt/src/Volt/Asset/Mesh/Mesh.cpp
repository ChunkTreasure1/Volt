#include "vtpch.h"
#include "Mesh.h"

#include "Volt/Asset/Rendering/Material.h"
#include "Volt/Rendering/Mesh/MeshCommon.h"

#include "Volt/Math/Math.h"
#include "Volt/Utility/Algorithms.h"

#include "Volt/SDF/SDFGenerator.h"

#include <RHIModule/Buffers/VertexBuffer.h>
#include <RHIModule/Buffers/IndexBuffer.h>
#include <RHIModule/Buffers/StorageBuffer.h>
#include <RHIModule/Images/Image.h>
#include <RHIModule/Images/ImageView.h>

#include <meshoptimizer/meshoptimizer.h>

namespace Volt
{
	VT_REGISTER_ASSET_FACTORY(AssetTypes::Mesh, Mesh);

	namespace Utility
	{
		VT_NODISCARD Vector<uint32_t> ElementCountPrefixSum(const Vector<VertexContainer>& elements)
		{
			Vector<uint32_t> prefixSums(elements.size());

			prefixSums.resize_uninitialized(elements.size());
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

	inline static BoundingSphere GetBoundingSphereFromVertices(const Vector<glm::vec3>& vertices)
	{
		return GetBoundingSphereFromVertices(vertices.data(), vertices.size());
	}

	Mesh::Mesh(Vector<Vertex> aVertices, Vector<uint32_t> aIndices, Ref<Material> aMaterial)
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

	Mesh::Mesh(Vector<Vertex> aVertices, Vector<uint32_t> aIndices, const MaterialTable& materialTable, const Vector<SubMesh>& subMeshes)
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
	}

	void Mesh::Construct()
	{
		VT_ASSERT_MSG(!m_indices.empty() && !m_vertexContainer.positions.empty(), "Indices and vertices must not be empty!");
		
		constexpr size_t MAX_VERTEX_COUNT = 64;
		constexpr size_t MAX_TRIANGLE_COUNT = 64;
		constexpr float CONE_WEIGHT = 0.f;

		struct PackedTri
		{
			uint32_t i0 : 10;
			uint32_t i1 : 10;
			uint32_t i2 : 10;
		};

		const uint32_t subMeshCount = static_cast<uint32_t>(m_subMeshes.size());
		const uint32_t threadCount = Algo::GetThreadCountFromIterationCount(subMeshCount);

		Vector<Vector<uint32_t>> perThreadMeshletData(threadCount);
		Vector<Vector<Meshlet>> perThreadMeshlets(threadCount);

		auto fu = Algo::ForEachParallelLockable([&](uint32_t threadIdx, uint32_t elementIdx)
		{
			auto& meshletData = perThreadMeshletData.at(threadIdx);
			auto& meshlets = perThreadMeshlets.at(threadIdx);

			auto& subMesh = m_subMeshes.at(elementIdx);

			const uint32_t* indicesPtr = &m_indices.at(subMesh.indexStartOffset);
			const glm::vec3* vertexPositionsPtr = &m_vertexContainer.positions.at(subMesh.vertexStartOffset);

			Vector<uint32_t> tempIndices(subMesh.indexCount);
			meshopt_optimizeVertexCache(tempIndices.data(), indicesPtr, subMesh.indexCount, subMesh.vertexCount);

			Vector<meshopt_Meshlet> meshoptMeshlets(meshopt_buildMeshletsBound(subMesh.indexCount, MAX_VERTEX_COUNT, MAX_TRIANGLE_COUNT));
			Vector<uint32_t> meshletVertices(meshoptMeshlets.size() * MAX_VERTEX_COUNT);
			Vector<uint8_t> meshletTriangles(meshoptMeshlets.size() * MAX_TRIANGLE_COUNT * 3);

			meshoptMeshlets.resize(meshopt_buildMeshlets(meshoptMeshlets.data(), meshletVertices.data(), meshletTriangles.data(), indicesPtr, static_cast<size_t>(subMesh.indexCount), &vertexPositionsPtr[0].x, static_cast<size_t>(subMesh.vertexCount), sizeof(glm::vec3), MAX_VERTEX_COUNT, MAX_TRIANGLE_COUNT, CONE_WEIGHT));

			subMesh.meshletCount = static_cast<uint32_t>(meshoptMeshlets.size());
			subMesh.meshletStartOffset = static_cast<uint32_t>(meshlets.size());

			meshlets.reserve(meshlets.size() + meshoptMeshlets.size());

			for (auto& meshlet : meshoptMeshlets)
			{
				meshopt_optimizeMeshlet(&meshletVertices[meshlet.vertex_offset], &meshletTriangles[meshlet.triangle_offset], meshlet.triangle_count, meshlet.vertex_count);

				size_t dataOffset = meshletData.size();

				meshletData.reserve(meshletData.size() + meshlet.vertex_count);
				for (uint32_t i = 0; i < meshlet.vertex_count; i++)
				{
					meshletData.push_back(meshletVertices[meshlet.vertex_offset + i]);
				}

				meshletData.reserve(meshletData.size() + meshlet.triangle_count);
				for (uint32_t i = 0; i < meshlet.triangle_count * 3; i += 3)
				{
					const uint8_t i0 = meshletTriangles[meshlet.triangle_offset + i + 0];
					const uint8_t i1 = meshletTriangles[meshlet.triangle_offset + i + 1];
					const uint8_t i2 = meshletTriangles[meshlet.triangle_offset + i + 2];

					PackedTri tri{ i0, i1, i2 };
					meshletData.push_back(*reinterpret_cast<uint32_t*>(&tri));
				}

				meshopt_Bounds bounds = meshopt_computeMeshletBounds(&meshletVertices[meshlet.vertex_offset], &meshletTriangles[meshlet.triangle_offset], meshlet.triangle_count, &vertexPositionsPtr[0].x, static_cast<size_t>(subMesh.vertexCount), sizeof(glm::vec3));

				auto& newMeshlet = meshlets.emplace_back();
				newMeshlet.dataOffset = static_cast<uint32_t>(dataOffset);

				newMeshlet.vertexTriCount = { meshlet.vertex_count, meshlet.triangle_count };
				newMeshlet.boundingSphereCenter = glm::vec3{ bounds.center[0], bounds.center[1], bounds.center[2] };
				newMeshlet.boundingSphereRadius = bounds.radius;

				newMeshlet.cone.x = bounds.cone_axis_s8[0];
				newMeshlet.cone.y = bounds.cone_axis_s8[1];
				newMeshlet.cone.z = bounds.cone_axis_s8[2];
				newMeshlet.cone.cutoff = bounds.cone_cutoff_s8;
			}

		}, subMeshCount);

		for (const auto& f : fu)
		{
			f.wait();
		}

		const auto meshletPrefixSums = Algo::ElementCountPrefixSum(perThreadMeshlets);
		const auto indexPrefixSums = Algo::ElementCountPrefixSum(perThreadMeshletData);

		auto fu2 = Algo::ForEachParallelLockable([&](uint32_t threadIdx, uint32_t elementIdx)
		{
			auto& subMesh = m_subMeshes.at(elementIdx);
			subMesh.meshletStartOffset += meshletPrefixSums.at(threadIdx);

		}, subMeshCount);

		auto fu3 = Algo::ForEachParallelLockable([&](uint32_t threadIdx, uint32_t elementIdx)
		{
			for (auto& meshlet : perThreadMeshlets.at(elementIdx))
			{
				meshlet.dataOffset += indexPrefixSums.at(elementIdx);
			}

		}, static_cast<uint32_t>(perThreadMeshlets.size()));

		for (const auto& f : fu2)
		{
			f.wait();
		}

		for (const auto& f : fu3)
		{
			f.wait();
		}

		for (size_t i = 0; i < perThreadMeshlets.size(); i++)
		{
			m_meshlets.append(perThreadMeshlets.at(i));
			m_meshletData.append(perThreadMeshletData.at(i));
		}

		const std::string meshName = !assetName.empty() ? " - " + assetName : "";

		// Index buffer
		{
			const auto& indices = m_indices;
			m_indexBuffer = BindlessResource<RHI::StorageBuffer>::CreateRef(static_cast<uint32_t>(indices.size()), sizeof(uint32_t), "Index Buffer" + meshName, RHI::BufferUsage::StorageBuffer | RHI::BufferUsage::IndexBuffer);
			m_indexBuffer->GetResource()->SetData(indices.data(), indices.size() * sizeof(uint32_t));
		}

		// Vertex positions
		{
			const auto& vertexPositions = m_vertexContainer.positions;
			m_vertexPositionsBuffer = BindlessResource<RHI::StorageBuffer>::CreateRef(static_cast<uint32_t>(vertexPositions.size()), sizeof(glm::vec3), "Vertex Positions" + meshName, RHI::BufferUsage::StorageBuffer | RHI::BufferUsage::VertexBuffer);
			m_vertexPositionsBuffer->GetResource()->SetData(vertexPositions.data(), vertexPositions.size() * sizeof(glm::vec3));
		}

		// Vertex material data
		{
			const auto& vertexMaterialData = m_vertexContainer.materialData;
			m_vertexMaterialBuffer = BindlessResource<RHI::StorageBuffer>::CreateRef(static_cast<uint32_t>(vertexMaterialData.size()), sizeof(VertexMaterialData), "Vertex Material Data" + meshName);
			m_vertexMaterialBuffer->GetResource()->SetData(vertexMaterialData.data(), vertexMaterialData.size() * sizeof(VertexMaterialData));
		}

		// Vertex animation info
		{
			const auto& vertexAnimationInfo = m_vertexContainer.animationInfo;
			m_vertexAnimationInfoBuffer = BindlessResource<RHI::StorageBuffer>::CreateRef(static_cast<uint32_t>(vertexAnimationInfo.size()), sizeof(VertexAnimationInfo), "Vertex Animation Info" + meshName);
			m_vertexAnimationInfoBuffer->GetResource()->SetData(vertexAnimationInfo.data(), vertexAnimationInfo.size() * sizeof(VertexAnimationInfo));
		}

		// Vertex animation data
		{
			const auto& vertexAnimationData = m_vertexContainer.animationData;
			m_vertexAnimationDataBuffer = BindlessResource<RHI::StorageBuffer>::CreateRef(static_cast<uint32_t>(vertexAnimationData.size()), sizeof(VertexAnimationData), "Vertex Animation Data" + meshName);
			m_vertexAnimationDataBuffer->GetResource()->SetData(vertexAnimationData.data(), vertexAnimationData.size() * sizeof(VertexAnimationData));
		}

		// Vertex bone influences
		{
			const auto& vertexBoneInfluences = m_vertexContainer.boneInfluences;
			if (!vertexBoneInfluences.empty())
			{
				m_vertexBoneInfluencesBuffer = BindlessResource<RHI::StorageBuffer>::CreateRef(static_cast<uint32_t>(vertexBoneInfluences.size()), sizeof(uint16_t), "Vertex Bone Influences" + meshName);
				m_vertexBoneInfluencesBuffer->GetResource()->SetData(vertexBoneInfluences.data(), vertexBoneInfluences.size() * sizeof(uint16_t));
			}
		}

		// Vertex bone weights
		{
			const auto& vertexBoneWeights = m_vertexContainer.boneWeights;
			if (!vertexBoneWeights.empty())
			{
				m_vertexBoneWeightsBuffer = BindlessResource<RHI::StorageBuffer>::CreateRef(static_cast<uint32_t>(vertexBoneWeights.size()), sizeof(float), "Vertex Bone Weights" + meshName);
				m_vertexBoneWeightsBuffer->GetResource()->SetData(vertexBoneWeights.data(), vertexBoneWeights.size() * sizeof(float));
			}
		}

		// Meshlet Data
		{
			m_meshletDataBuffer = BindlessResource<RHI::StorageBuffer>::CreateRef(static_cast<uint32_t>(m_meshletData.size()), sizeof(uint32_t), "Meshlet Data Buffer" + meshName);
			m_meshletDataBuffer->GetResource()->SetData(m_meshletData.data(), m_meshletData.size() * sizeof(uint32_t));
		}

		// Meshlets
		{
			m_meshletsBuffer = BindlessResource<RHI::StorageBuffer>::CreateRef(static_cast<uint32_t>(m_meshlets.size()), sizeof(Meshlet), "Meshlet Buffer" + meshName);
			m_meshletsBuffer->GetResource()->SetData(m_meshlets.data(), m_meshlets.size() * sizeof(Meshlet));
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
			Vector<glm::vec3> subMeshVertices;
			subMeshVertices.insert(subMeshVertices.end(), std::next(m_vertexContainer.positions.begin(), subMesh.vertexStartOffset), std::next(m_vertexContainer.positions.begin(), subMesh.vertexStartOffset + subMesh.vertexCount));

			// Find bounding box
			{
				glm::vec3 t, r, s;
				Math::Decompose(subMesh.transform, t, r, s);
			
				glm::vec3 subMin = { FLT_MAX, FLT_MAX, FLT_MAX };
				glm::vec3 subMax = { -FLT_MAX, -FLT_MAX, -FLT_MAX };

				for (const auto& vp : subMeshVertices)
				{
					subMin = glm::min(subMin, vp);
					subMax = glm::max(subMax, vp);
				}

				m_subMeshBoundingBoxes[i] = BoundingBox{ subMax, subMin };
			}

			m_subMeshBoundingSpheres[i] = GetBoundingSphereFromVertices(subMeshVertices);
			i++;
		}

		// Create GPU Meshes
		for (uint32_t i = 0; const auto& subMesh : m_subMeshes)
		{
			auto& gpuMesh = m_gpuMeshes.emplace_back();
			gpuMesh.vertexStartOffset = subMesh.vertexStartOffset;
			gpuMesh.meshletStartOffset = subMesh.meshletStartOffset;
			gpuMesh.meshletCount = subMesh.meshletCount;
			gpuMesh.meshletIndexStartOffset = subMesh.meshletIndexStartOffset;
			gpuMesh.vertexPositionsBuffer = m_vertexPositionsBuffer->GetResourceHandle();
			gpuMesh.vertexMaterialBuffer = m_vertexMaterialBuffer->GetResourceHandle();
			gpuMesh.vertexAnimationInfoBuffer = m_vertexAnimationDataBuffer->GetResourceHandle();
			gpuMesh.vertexBoneWeightsBuffer = m_vertexBoneWeightsBuffer ? m_vertexBoneWeightsBuffer->GetResourceHandle() : Resource::Invalid;
			gpuMesh.vertexBoneInfluencesBuffer = m_vertexBoneInfluencesBuffer ? m_vertexBoneInfluencesBuffer->GetResourceHandle() : Resource::Invalid;
			gpuMesh.meshletsBuffer = m_meshletsBuffer->GetResourceHandle();
			gpuMesh.meshletDataBuffer = m_meshletDataBuffer->GetResourceHandle();
			gpuMesh.center = subMesh.transform * glm::vec4(m_subMeshBoundingSpheres.at(i).center, 1.f);

			glm::vec3 t, r, s;
			Math::Decompose(subMesh.transform, t, r, s);

			gpuMesh.radius = m_subMeshBoundingSpheres.at(i).radius * glm::max(s.x, glm::max(s.y, s.z));

			i++;
		}

		// Create SDF data
		{
			SDFGenerator sdfGenerator{};
			auto res = sdfGenerator.Generate(*this);
		
			m_gpuMeshSDFs.reserve(res.size());

			for (uint32_t i = 0; const auto& sdf : res)
			{
				m_sdfTextures[i] = sdf.sdfTexture;
				m_brickGrids[i] = sdf.brickGrid;
				m_brickBuffers[i] = CreateRef<BindlessResource<RHI::StorageBuffer>>(sdf.sdfBricksBuffer);

				auto& gpuSDF = m_gpuMeshSDFs.emplace_back();
				gpuSDF.min = sdf.min;
				gpuSDF.max = sdf.max;
				gpuSDF.size = sdf.size; 
				gpuSDF.sdfTexture = sdf.sdfTexture->GetResourceHandle();
				gpuSDF.bricksBuffer = m_brickBuffers[i]->GetResourceHandle();
				gpuSDF.brickCount = static_cast<uint32_t>(sdf.brickGrid.size());

				i++;
			}
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

	void Mesh::InitializeWithVertices(const Vector<Vertex>& vertices)
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
