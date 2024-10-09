#pragma once

#include "Volt/Asset/Mesh/SubMesh.h"

#include "Volt/Asset/Rendering/MaterialTable.h"
#include "Volt/Asset/AssetTypes.h"

#include "Volt/Rendering/Vertex.h"
#include "Volt/Rendering/BoundingStructures.h"
#include "Volt/Rendering/Mesh/MeshCommon.h"

#include "Volt/Rendering/GPUScene.h"

#include "Volt/SDF/SDFGenerator.h"

#include <RenderCore/Resources/BindlessResource.h>

#include <RHIModule/Buffers/StorageBuffer.h>

#include <CoreUtilities/Containers/Map.h>

#include <AssetSystem/Asset.h>
#include <AssetSystem/AssetType.h>

namespace Volt
{
	namespace RHI
	{
		class VertexBuffer;
		class IndexBuffer;
	}

	class Material;

	struct VertexContainer
	{
		Vector<glm::vec3> positions;
		Vector<VertexMaterialData> materialData;
		
		Vector<VertexAnimationInfo> animationInfo;
		Vector<VertexAnimationData> animationData;

		Vector<uint16_t> boneInfluences;
		Vector<float> boneWeights;

		VT_INLINE size_t Size() const
		{
			return positions.size();
		}

		VT_INLINE void Resize(size_t size)
		{
			positions.resize(size);
			materialData.resize(size);
			animationInfo.resize(size);
			animationData.resize(size);
			boneInfluences.resize(size);
			boneWeights.resize(size);
		}

		VT_INLINE void Append(const VertexContainer& other, const size_t count = 0)
		{
			positions.insert(positions.end(), other.positions.begin(), count > 0 ? other.positions.begin() + count : other.positions.end());
			materialData.insert(materialData.end(), other.materialData.begin(), count > 0 ? other.materialData.begin() + count : other.materialData.end());
			animationInfo.insert(animationInfo.end(), other.animationInfo.begin(), count > 0 ? other.animationInfo.begin() + count : other.animationInfo.end());
			animationData.insert(animationData.end(), other.animationData.begin(), count > 0 ? other.animationData.begin() + count : other.animationData.end());
			boneInfluences.insert(boneInfluences.end(), other.boneInfluences.begin(), other.boneInfluences.end());
			boneWeights.insert(boneWeights.end(), other.boneWeights.begin(), other.boneWeights.end());
		}
	};

	class Mesh : public Asset
	{
	public:
		Mesh() = default;
		Mesh(Vector<Vertex> aVertices, Vector<uint32_t> aIndices, Ref<Material> aMaterial);
		Mesh(Vector<Vertex> aVertices, Vector<uint32_t> aIndices, const MaterialTable& materialTable, const Vector<SubMesh>& subMeshes);
		~Mesh() override;

		void Construct();

		inline const Vector<SubMesh>& GetSubMeshes() const { return m_subMeshes; }
		inline Vector<SubMesh>& GetSubMeshesMutable() { return m_subMeshes; }

		inline const MaterialTable& GetMaterialTable() const { return m_materialTable; }
		void SetMaterial(Ref<Material> material, uint32_t index);

		inline const size_t GetVertexCount() const { return m_vertexContainer.Size(); }
		inline const size_t GetIndexCount() const { return m_indices.size(); }

		inline const Vector<uint32_t>& GetIndices() const { return m_indices; }
		inline const Vector<Meshlet>& GetMeshlets() const { return m_meshlets; }

		inline const BoundingSphere& GetBoundingSphere() const { return m_boundingSphere; }
		inline const BoundingBox& GetBoundingBox() const { return m_boundingBox; }
		inline const Vector<GPUMesh>& GetGPUMeshes() const { return m_gpuMeshes; }
		inline const Vector<GPUMeshSDF>& GetGPUMeshSDFs() const { return m_gpuMeshSDFs; }

		VT_NODISCARD VT_INLINE const Vector<SDFBrick>& GetBrickGrid(const uint32_t index) const { return m_brickGrids.at(index); }
		VT_NODISCARD VT_INLINE const BindlessResourceRef<RHI::StorageBuffer>& GetBrickBuffer(const uint32_t index) const { return m_brickBuffers.at(index); }

		inline const BoundingSphere& GetSubMeshBoundingSphere(const uint32_t index) const { return m_subMeshBoundingSpheres.at(index);  }
		inline const BoundingBox& GetSubMeshBoundingBox(const uint32_t index) const { return m_subMeshBoundingBoxes.at(index);  }

		inline BindlessResourceRef<RHI::StorageBuffer> GetVertexPositionsBuffer() const { return m_vertexPositionsBuffer; }
		inline BindlessResourceRef<RHI::StorageBuffer> GetVertexMaterialBuffer() const { return m_vertexMaterialBuffer; }
		inline BindlessResourceRef<RHI::StorageBuffer> GetVertexAnimationInfoBuffer() const { return m_vertexAnimationDataBuffer; }
		inline BindlessResourceRef<RHI::StorageBuffer> GetIndexBuffer() const { return m_indexBuffer; }

		inline BindlessResourceRef<RHI::StorageBuffer> GetMeshletDataBuffer() const { return m_meshletDataBuffer; }
		inline BindlessResourceRef<RHI::StorageBuffer> GetMeshletBuffer() const { return m_meshletsBuffer; }

		VT_NODISCARD VT_INLINE const VertexContainer& GetVertexContainer() const { return m_vertexContainer; }

		static AssetType GetStaticType() { return AssetTypes::Mesh; }
		AssetType GetType() override { return GetStaticType(); }
		uint32_t GetVersion() const override { return 2; }

	private:
		friend class FbxImporter;
		friend class MeshCompiler;
		friend class MeshSerializer;
		friend class MeshExporterUtilities;
		friend class VTMeshImporter;
		friend class GLTFImporter;
		friend class FbxSourceImporter;
		friend class GLTFSourceImporter;

		VertexMaterialData GetMaterialDataFromVertex(const Vertex& vertex);

		void InitializeWithVertices(const Vector<Vertex>& vertices);

		Vector<SubMesh> m_subMeshes;
		Vector<Meshlet> m_meshlets;

		VertexContainer m_vertexContainer{};
		Vector<uint32_t> m_indices;
		Vector<uint32_t> m_meshletData;

		MaterialTable m_materialTable;

		BindlessResourceRef<RHI::StorageBuffer> m_indexBuffer;
		BindlessResourceRef<RHI::StorageBuffer> m_vertexPositionsBuffer;
		BindlessResourceRef<RHI::StorageBuffer> m_vertexMaterialBuffer;
		BindlessResourceRef<RHI::StorageBuffer> m_vertexAnimationInfoBuffer;
		BindlessResourceRef<RHI::StorageBuffer> m_vertexBoneInfluencesBuffer;
		BindlessResourceRef<RHI::StorageBuffer> m_vertexBoneWeightsBuffer;

		BindlessResourceRef<RHI::StorageBuffer> m_meshletsBuffer;
		BindlessResourceRef<RHI::StorageBuffer> m_meshletDataBuffer;

		BindlessResourceRef<RHI::StorageBuffer> m_vertexAnimationDataBuffer;

		BoundingSphere m_boundingSphere;
		BoundingBox m_boundingBox;

		vt::map<uint32_t, BindlessResourceRef<RHI::Image>> m_sdfTextures;
		vt::map<uint32_t, Vector<SDFBrick>> m_brickGrids;
		vt::map<uint32_t, BindlessResourceRef<RHI::StorageBuffer>> m_brickBuffers;

		Vector<GPUMesh> m_gpuMeshes;
		Vector<GPUMeshSDF> m_gpuMeshSDFs;

		glm::vec3 m_averageScale{ 1.f };
		vt::map<uint32_t, BoundingSphere> m_subMeshBoundingSpheres;
		vt::map<uint32_t, BoundingBox> m_subMeshBoundingBoxes;
	};
}
