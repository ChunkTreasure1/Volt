#pragma once

#include "Volt/Asset/Asset.h"
#include "Volt/Asset/Mesh/SubMesh.h"

#include "Volt/Rendering/Vertex.h"
#include "Volt/Rendering/BoundingStructures.h"

#include "Volt/Rendering/RendererStructs.h"

namespace Volt
{
	namespace RHI
	{
		class VertexBuffer;
		class IndexBuffer;
	}

	class Material;

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

		inline const std::vector<Vertex>& GetVertices() { return m_vertices; }
		inline const std::vector<uint32_t>& GetIndices() { return m_indices; }

		inline const BoundingSphere& GetBoundingSphere() const { return m_boundingSphere; }
		inline const BoundingBox& GetBoundingBox() const { return m_boundingBox; }
		inline const std::vector<GPUMesh>& GetGPUMeshes() const { return m_gpuMeshes; }

		inline const std::map<uint32_t, BoundingSphere> GetSubMeshBoundingSpheres() const { return m_subMeshBoundingSpheres; }

		inline Ref<RHI::VertexBuffer> GetVertexBuffer() const { return m_vertexBuffer; }
		inline Ref<RHI::IndexBuffer> GetIndexBuffer() const { return m_indexBuffer; }

		static AssetType GetStaticType() { return AssetType::Mesh; }
		AssetType GetType() override { return GetStaticType(); }

	private:
		std::vector<std::vector<Vertex>> ExtractSubMeshVertices();
		std::vector<std::vector<uint32_t>> ExtractSubMeshIndices();

		friend class FbxImporter;
		friend class MeshCompiler;
		friend class MeshExporterUtilities;
		friend class VTMeshImporter;
		friend class GLTFImporter;

		std::vector<SubMesh> m_subMeshes;

		std::vector<Vertex> m_vertices;
		std::vector<uint32_t> m_indices;

		Ref<Material> m_material;
		Ref<RHI::VertexBuffer> m_vertexBuffer;
		Ref<RHI::IndexBuffer> m_indexBuffer;

		BoundingSphere m_boundingSphere;
		BoundingBox m_boundingBox;

		std::vector<GPUMesh> m_gpuMeshes;

		glm::vec3 m_averageScale{ 1.f };
		std::map<uint32_t, BoundingSphere> m_subMeshBoundingSpheres;
	};
}
