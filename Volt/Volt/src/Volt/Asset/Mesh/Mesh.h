#pragma once

#include "Volt/Asset/Asset.h"
#include "Volt/Asset/Mesh/SubMesh.h"

#include "Volt/Asset/Rendering/MaterialTable.h"

#include "Volt/Rendering/Vertex.h"
#include "Volt/Rendering/BoundingStructures.h"
#include "Volt/Rendering/Mesh/MeshCommon.h"

#include "Volt/Rendering/Resources/BindlessResource.h"
#include "Volt/Rendering/GPUScene.h"

namespace Volt
{
	namespace RHI
	{
		class VertexBuffer;
		class IndexBuffer;

		class StorageBuffer;
	}

	class Material;

	struct VertexContainer
	{
		std::vector<glm::vec3> positions;
		std::vector<VertexMaterialData> materialData;
		
		std::vector<VertexAnimationInfo> vertexAnimationInfo;
		std::vector<uint16_t> vertexBoneInfluences;
		std::vector<uint8_t> vertexBoneWeights;

		VT_INLINE size_t Size() const
		{
			return positions.size();
		}

		VT_INLINE void Resize(size_t size)
		{
			positions.resize(size);
			materialData.resize(size);
			vertexAnimationInfo.resize(size);
		}
	};

	class Mesh : public Asset
	{
	public:
		Mesh() = default;
		Mesh(std::vector<Vertex> aVertices, std::vector<uint32_t> aIndices, Ref<Material> aMaterial);
		Mesh(std::vector<Vertex> aVertices, std::vector<uint32_t> aIndices, const MaterialTable& materialTable, const std::vector<SubMesh>& subMeshes);
		~Mesh() override;

		void Construct();

		inline const std::vector<SubMesh>& GetSubMeshes() const { return m_subMeshes; }
		inline std::vector<SubMesh>& GetSubMeshesMutable() { return m_subMeshes; }

		inline const MaterialTable& GetMaterialTable() const { return m_materialTable; }
		void SetMaterial(Ref<Material> material, uint32_t index);

		inline const size_t GetVertexCount() const { return m_vertexContainer.Size(); }
		inline const size_t GetIndexCount() const { return m_indices.size(); }

		inline const std::vector<uint32_t>& GetIndices() const { return m_indices; }
		inline const std::vector<Meshlet>& GetMeshlets() const { return m_meshlets; }

		inline const BoundingSphere& GetBoundingSphere() const { return m_boundingSphere; }
		inline const BoundingBox& GetBoundingBox() const { return m_boundingBox; }
		inline const std::vector<GPUMesh>& GetGPUMeshes() const { return m_gpuMeshes; }

		inline const BoundingSphere& GetSubMeshBoundingSphere(const uint32_t index) const { return m_subMeshBoundingSpheres.at(index);  }

		inline BindlessResourceRef<RHI::StorageBuffer> GetVertexPositionsBuffer() const { return m_vertexPositionsBuffer; }
		inline BindlessResourceRef<RHI::StorageBuffer> GetVertexMaterialBuffer() const { return m_vertexMaterialBuffer; }
		inline BindlessResourceRef<RHI::StorageBuffer> GetVertexAnimationInfoBuffer() const { return m_vertexAnimationInfoBuffer; }
		inline BindlessResourceRef<RHI::StorageBuffer> GetIndexStorageBuffer() const { return m_indexBuffer; }

		VT_NODISCARD VT_INLINE const VertexContainer& GetVertexContainer() const { return m_vertexContainer; }

		static AssetType GetStaticType() { return AssetType::Mesh; }
		AssetType GetType() override { return GetStaticType(); }
		uint32_t GetVersion() const override { return 2; }

	private:
		friend class FbxImporter;
		friend class MeshCompiler;
		friend class MeshSerializer;
		friend class MeshExporterUtilities;
		friend class VTMeshImporter;
		friend class GLTFImporter;

		VertexMaterialData GetMaterialDataFromVertex(const Vertex& vertex);

		void InitializeWithVertices(const std::vector<Vertex>& vertices);

		std::vector<SubMesh> m_subMeshes;
		VertexContainer m_meshletVertexContainer;

		std::vector<uint32_t> m_indices;

		std::vector<Meshlet> m_meshlets;
		std::vector<uint32_t> m_meshletIndices;

		MaterialTable m_materialTable;

		BindlessResourceRef<RHI::StorageBuffer> m_vertexPositionsBuffer;
		BindlessResourceRef<RHI::StorageBuffer> m_vertexMaterialBuffer;
		BindlessResourceRef<RHI::StorageBuffer> m_vertexAnimationInfoBuffer;
		BindlessResourceRef<RHI::StorageBuffer> m_vertexBoneInfluencesBuffer;
		BindlessResourceRef<RHI::StorageBuffer> m_vertexBoneWeightsBuffer;
		BindlessResourceRef<RHI::StorageBuffer> m_indexBuffer;

		BindlessResourceRef<RHI::StorageBuffer> m_meshletIndexBuffer;
		BindlessResourceRef<RHI::StorageBuffer> m_meshletsBuffer;

		BoundingSphere m_boundingSphere;
		BoundingBox m_boundingBox;

		std::vector<GPUMesh> m_gpuMeshes;

		glm::vec3 m_averageScale{ 1.f };
		std::map<uint32_t, BoundingSphere> m_subMeshBoundingSpheres;

		// Prototype vertex stuff
		VertexContainer m_vertexContainer;

		std::vector<uint16_t> m_vertexBoneInfluences;
		std::vector<uint8_t> m_vertexBoneWeights;
	};
}
