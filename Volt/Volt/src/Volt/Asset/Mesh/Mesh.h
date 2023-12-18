#pragma once

#include "Volt/Asset/Asset.h"
#include "Volt/Asset/Mesh/SubMesh.h"

#include "Volt/Rendering/Vertex.h"
#include "Volt/Rendering/BoundingStructures.h"

#include "Volt/RenderingNew/Resources/GlobalResource.h"
#include "Volt/RenderingNew/GPUScene.h"

namespace Volt
{
	namespace RHI
	{
		class VertexBuffer;
		class IndexBuffer;

		class StorageBuffer;
	}

	class Material;

	struct Meshlet
	{
		uint32_t vertexOffset;
		uint32_t triangleOffset;
		uint32_t vertexCount;
		uint32_t triangleCount;

		uint32_t objectId;
		uint32_t meshId;
	};

	class Mesh : public Asset
	{
	public:
		Mesh() = default;
		Mesh(std::vector<Vertex> aVertices, std::vector<uint32_t> aIndices, Ref<Material> aMaterial);
		Mesh(std::vector<Vertex> aVertices, std::vector<uint32_t> aIndices, Ref<Material> aMaterial, const std::vector<SubMesh>& subMeshes);
		~Mesh() override;

		void Construct();
		const std::vector<EncodedVertex> GetEncodedVertices() const;

		inline const std::vector<SubMesh>& GetSubMeshes() const { return m_subMeshes; }
		inline std::vector<SubMesh>& GetSubMeshesMutable() { return m_subMeshes; }
		inline const Ref<Material>& GetMaterial() const { return m_material; }
		inline void SetMaterial(Ref<Material> material) { m_material = material; }

		inline const size_t GetVertexCount() const { return m_vertices.size(); }
		inline const size_t GetIndexCount() const { return m_indices.size(); }

		inline const std::vector<Vertex>& GetVertices() const { return m_vertices; }
		inline const std::vector<uint32_t>& GetIndices() const { return m_indices; }
		inline const std::vector<Meshlet>& GetMeshlets() const { return m_meshlets; }

		inline const BoundingSphere& GetBoundingSphere() const { return m_boundingSphere; }
		inline const BoundingBox& GetBoundingBox() const { return m_boundingBox; }
		inline const std::vector<GPUMesh>& GetGPUMeshes() const { return m_gpuMeshes; }

		inline const std::map<uint32_t, BoundingSphere> GetSubMeshBoundingSpheres() const { return m_subMeshBoundingSpheres; }
		
		inline Ref<GlobalResource<RHI::StorageBuffer>> GetVertexPositionsBuffer() const { return m_vertexPositionsBuffer; }
		inline Ref<GlobalResource<RHI::StorageBuffer>> GetVertexMaterialBuffer() const { return m_vertexMaterialBuffer; }
		inline Ref<GlobalResource<RHI::StorageBuffer>> GetVertexAnimationBuffer() const { return m_vertexAnimationBuffer; }
		inline Ref<GlobalResource<RHI::StorageBuffer>> GetIndexStorageBuffer() const { return m_indexBuffer; }

		static AssetType GetStaticType() { return AssetType::Mesh; }
		AssetType GetType() override { return GetStaticType(); }

	private:
		friend class FbxImporter;
		friend class MeshCompiler;
		friend class MeshExporterUtilities;
		friend class VTMeshImporter;
		friend class GLTFImporter;

		struct VertexMaterialData
		{
			glm::vec<4, uint8_t> normal;
			float tangent = 0.f;
			glm::vec<2, half_float::half> texCoords = glm::vec<2, half_float::half>(0.f, 0.f);
		};

		struct VertexAnimationData
		{
			glm::vec<4, uint16_t> influences = 0;
			half_float::half weights[4] = { half_float::half(0.f), half_float::half(0.f), half_float::half(0.f), half_float::half(0.f) };
		};

		std::vector<std::vector<Vertex>> ExtractSubMeshVertices();
		std::vector<std::vector<uint32_t>> ExtractSubMeshIndices();

		const std::vector<glm::vec3> GetVertexPositions();
		const std::vector<VertexMaterialData> GetVertexMaterialData();
		const std::vector<VertexAnimationData> GetVertexAnimationData();

		std::vector<SubMesh> m_subMeshes;

		std::vector<Vertex> m_vertices;
		std::vector<uint32_t> m_indices;

		std::vector<Meshlet> m_meshlets;
		std::vector<uint32_t> m_meshletIndices;

		Ref<Material> m_material;

		Ref<GlobalResource<RHI::StorageBuffer>> m_vertexPositionsBuffer;
		Ref<GlobalResource<RHI::StorageBuffer>> m_vertexMaterialBuffer;
		Ref<GlobalResource<RHI::StorageBuffer>> m_vertexAnimationBuffer;
		Ref<GlobalResource<RHI::StorageBuffer>> m_indexBuffer;

		Ref<GlobalResource<RHI::StorageBuffer>> m_meshletIndexBuffer;
		Ref<GlobalResource<RHI::StorageBuffer>> m_meshletsBuffer;

		BoundingSphere m_boundingSphere;
		BoundingBox m_boundingBox;

		std::vector<GPUMesh> m_gpuMeshes;

		glm::vec3 m_averageScale{ 1.f };
		std::map<uint32_t, BoundingSphere> m_subMeshBoundingSpheres;
	};
}
