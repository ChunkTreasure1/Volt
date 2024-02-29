#pragma once

#include "Volt/Asset/Asset.h"
#include "Volt/Asset/Mesh/SubMesh.h"

#include "Volt/Rendering/Vertex.h"
#include "Volt/Rendering/BoundingStructures.h"

#include "Volt/Rendering/RendererStructs.h"

namespace Volt
{
	class Material;
	class VertexBuffer;
	class IndexBuffer;

	class Mesh : public Asset
	{
	public:
		Mesh() = default;
		Mesh(std::vector<Vertex> aVertices, std::vector<uint32_t> aIndices, Ref<Material> aMaterial);
		Mesh(std::vector<Vertex> aVertices, std::vector<uint32_t> aIndices, Ref<Material> aMaterial, const std::vector<SubMesh>& subMeshes);
		~Mesh() override;

		void Construct();
		const std::vector<EncodedVertex> GetEncodedVertices() const;

		inline const std::vector<SubMesh>& GetSubMeshes() const { return mySubMeshes; }
		inline std::vector<SubMesh>& GetSubMeshesMutable() { return mySubMeshes; }
		inline const Ref<Material>& GetMaterial() const { return myMaterial; }
		inline void SetMaterial(Ref<Material> material) { myMaterial = material; }

		inline const size_t GetVertexCount() const { return myVertices.size(); }
		inline const size_t GetIndexCount() const { return myIndices.size(); }

		inline const std::vector<Vertex>& GetVertices() { return myVertices; }
		inline const std::vector<uint32_t>& GetIndices() { return myIndices; }

		inline const BoundingSphere& GetBoundingSphere() const { return myBoundingSphere; }
		inline const BoundingBox& GetBoundingBox() const { return myBoundingBox; }
		inline const std::vector<GPUMesh>& GetGPUMeshes() const { return myGPUMeshes; }

		inline const uint32_t GetVertexStartOffset() const { return myVertexStartOffset; }
		inline const uint32_t GetIndexStartOffset() const { return myIndexStartOffset; }

		inline const std::map<uint32_t, BoundingSphere> GetSubMeshBoundingSpheres() const { return mySubMeshBoundingSpheres; }

		inline Ref<VertexBuffer> GetVertexBuffer() const { return myVertexBuffer; }
		inline Ref<IndexBuffer> GetIndexBuffer() const { return myIndexBuffer; }

		static AssetType GetStaticType() { return AssetType::Mesh; }
		AssetType GetType() override { return GetStaticType(); }

	private:
		std::vector<std::vector<Vertex>> ExtractSubMeshVertices();
		std::vector<std::vector<uint32_t>> ExtractSubMeshIndices();

		friend class FbxImporter;
		friend class MeshCompiler;
		friend class MeshImporter;
		friend class MeshExporterUtilities;
		friend class VTMeshImporter;
		friend class GLTFImporter;

		std::vector<SubMesh> mySubMeshes;

		std::vector<Vertex> myVertices;
		std::vector<uint32_t> myIndices;

		Ref<Material> myMaterial;
		Ref<VertexBuffer> myVertexBuffer;
		Ref<IndexBuffer> myIndexBuffer;

		BoundingSphere myBoundingSphere;
		BoundingBox myBoundingBox;

		std::vector<GPUMesh> myGPUMeshes;

		uint32_t myIndexStartOffset = 0;
		uint32_t myVertexStartOffset = 0;

		glm::vec3 myAverageScale{ 1.f };
		std::map<uint32_t, BoundingSphere> mySubMeshBoundingSpheres;
	};
}
