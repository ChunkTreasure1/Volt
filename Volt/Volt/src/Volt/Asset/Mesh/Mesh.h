#pragma once

#include "Volt/Asset/Asset.h"
#include "Volt/Asset/Mesh/SubMesh.h"

#include "Volt/Rendering/Vertex.h"
#include "Volt/Rendering/BoundingStructures.h"

namespace Volt
{
	class VertexBuffer;
	class IndexBuffer;
	class Material;

	class Mesh : public Asset
	{
	public:
		Mesh() = default;
		Mesh(std::vector<Vertex> aVertices, std::vector<uint32_t> aIndices, Ref<Material>& aMaterial);
		~Mesh() override = default;

		void Construct();

		inline const std::vector<SubMesh>& GetSubMeshes() const { return mySubMeshes; }
		inline const Ref<Material>& GetMaterial() const { return myMaterial; }
		inline void SetMaterial(Ref<Material> material) { myMaterial = material; }

		inline const size_t GetVertexCount() const { return myVertices.size(); }
		inline const size_t GetIndexCount() const { return myIndices.size(); }

		inline const Ref<VertexBuffer>& GetVertexBuffer() const { return myVertexBuffer; }
		inline const Ref<IndexBuffer>& GetIndexBuffer() const { return myIndexBuffer; }

		static AssetType GetStaticType() { return AssetType::Mesh; }
		AssetType GetType() override { return GetStaticType(); }

		const std::vector<Vertex>& GetVertices() { return myVertices; }
		const std::vector<uint32_t>& GetIndices() { return myIndices; }

		const BoundingSphere& GetBoundingSphere() const { return myBoundingSphere; }
		const BoundingBox& GetBoundingBox() const { return myBoundingBox; }

	private:
		friend class FbxImporter;
		friend class MeshCompiler;
		friend class VTMeshImporter;

		std::vector<SubMesh> mySubMeshes;

		std::vector<Vertex> myVertices;
		std::vector<uint32_t> myIndices;

		Ref<Material> myMaterial;
		Ref<VertexBuffer> myVertexBuffer;
		Ref<IndexBuffer> myIndexBuffer;

		BoundingSphere myBoundingSphere;
		BoundingBox myBoundingBox;
	};
}