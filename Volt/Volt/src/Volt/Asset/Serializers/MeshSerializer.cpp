#include "vtpch.h"
#include "MeshSerializer.h"

#include "Volt/Asset/AssetManager.h"
#include "Volt/Asset/Mesh/Mesh.h"
#include "Volt/Asset/Rendering/Material.h"
#include "Volt/Asset/Serialization/AssetSerializationCommon.h"

namespace Volt
{
	struct MeshSerializationData_V1
	{
		Vector<AssetHandle> materials;
		Vector<Vertex> vertices;
		Vector<uint32_t> indices;

		glm::vec3 boundingSphereCenter;
		float boundingSphereRadius;

		Vector<SubMesh> subMeshes;

		static void Serialize(BinaryStreamWriter& streamWriter, const MeshSerializationData_V1& data)
		{
			streamWriter.WriteRaw(data.materials);
			streamWriter.WriteRaw(data.vertices);
			streamWriter.WriteRaw(data.indices);
			streamWriter.Write(data.boundingSphereCenter);
			streamWriter.Write(data.boundingSphereRadius);
			streamWriter.Write(data.subMeshes);
		}

		static void Deserialize(BinaryStreamReader& streamReader, MeshSerializationData_V1& outData)
		{
			streamReader.ReadRaw(outData.materials);
			streamReader.ReadRaw(outData.vertices);
			streamReader.ReadRaw(outData.indices);
			streamReader.Read(outData.boundingSphereCenter);
			streamReader.Read(outData.boundingSphereRadius);
			streamReader.Read(outData.subMeshes);
		}
	};

	struct MeshSerializationData_V2
	{
		Vector<glm::vec3> vertexPositions;
		Vector<VertexMaterialData> vertexMaterialData;
		Vector<VertexAnimationInfo> vertexAnimationInfo;
		Vector<VertexAnimationData> vertexAnimationData;
		Vector<uint16_t> vertexBoneInfluences;
		Vector<float> vertexBoneWeights;
		Vector<uint32_t> indices;

		Vector<AssetHandle> materials;

		glm::vec3 boundingSphereCenter;
		float boundingSphereRadius;

		Vector<SubMesh> subMeshes;

		static void Serialize(BinaryStreamWriter& streamWriter, const MeshSerializationData_V2& data)
		{
			streamWriter.WriteRaw(data.vertexPositions);
			streamWriter.WriteRaw(data.vertexMaterialData);
			streamWriter.WriteRaw(data.vertexAnimationInfo);
			streamWriter.WriteRaw(data.vertexAnimationData);
			streamWriter.WriteRaw(data.vertexBoneInfluences);
			streamWriter.WriteRaw(data.vertexBoneWeights);
			streamWriter.WriteRaw(data.indices);
			streamWriter.WriteRaw(data.materials);
			streamWriter.Write(data.boundingSphereCenter);
			streamWriter.Write(data.boundingSphereRadius);
			streamWriter.Write(data.subMeshes);
		}

		static void Deserialize(BinaryStreamReader& streamReader, MeshSerializationData_V2& outData)
		{
			streamReader.ReadRaw(outData.vertexPositions);
			streamReader.ReadRaw(outData.vertexMaterialData);
			streamReader.ReadRaw(outData.vertexAnimationInfo);
			streamReader.ReadRaw(outData.vertexAnimationData);
			streamReader.ReadRaw(outData.vertexBoneInfluences);
			streamReader.ReadRaw(outData.vertexBoneWeights);
			streamReader.ReadRaw(outData.indices);
			streamReader.ReadRaw(outData.materials);
			streamReader.Read(outData.boundingSphereCenter);
			streamReader.Read(outData.boundingSphereRadius);
			streamReader.Read(outData.subMeshes);
		}
	};

	void MeshSerializer::Serialize(const AssetMetadata& metadata, const Ref<Asset>& asset) const
	{
		Ref<Mesh> mesh = std::reinterpret_pointer_cast<Mesh>(asset);

		BinaryStreamWriter streamWriter{};
		const size_t compressedDataOffset = AssetSerializer::WriteMetadata(metadata, asset->GetVersion(), streamWriter);

		const auto& tempMaterialTable = mesh->m_materialTable;

		MeshSerializationData_V2 serializationData{}; 
		for (const auto& mat : tempMaterialTable)
		{
			serializationData.materials.emplace_back(mat);
		}

		const auto& vertexContainer = mesh->GetVertexContainer();

		serializationData.vertexPositions = vertexContainer.positions;
		serializationData.vertexMaterialData = vertexContainer.materialData;
		serializationData.vertexAnimationInfo = vertexContainer.animationInfo;
		serializationData.vertexAnimationData = vertexContainer.animationData;
		serializationData.vertexBoneInfluences = vertexContainer.boneInfluences;
		serializationData.vertexBoneWeights = vertexContainer.boneWeights;
		serializationData.indices = mesh->GetIndices();
		serializationData.boundingSphereCenter = mesh->GetBoundingSphere().center;
		serializationData.boundingSphereRadius = mesh->GetBoundingSphere().radius;
		serializationData.subMeshes = mesh->GetSubMeshes();

		streamWriter.Write(serializationData);

		const auto filePath = AssetManager::GetFilesystemPath(metadata.filePath);
		streamWriter.WriteToDisk(filePath, true, compressedDataOffset);
	}

	bool MeshSerializer::Deserialize(const AssetMetadata& metadata, Ref<Asset> destinationAsset) const
	{
		const auto filePath = AssetManager::GetFilesystemPath(metadata.filePath);

		if (!std::filesystem::exists(filePath))
		{
			VT_LOG(Error, "File {0} not found!", metadata.filePath);
			destinationAsset->SetFlag(AssetFlag::Missing, true);
			return false;
		}

		BinaryStreamReader streamReader{ filePath };

		if (!streamReader.IsStreamValid())
		{
			VT_LOG(Error, "Failed to open file: {0}!", metadata.filePath);
			destinationAsset->SetFlag(AssetFlag::Invalid, true);
			return false;
		}

		Ref<Mesh> mesh = std::reinterpret_pointer_cast<Mesh>(destinationAsset);

		SerializedAssetMetadata serializedMetadata = AssetSerializer::ReadMetadata(streamReader);
		if (serializedMetadata.version == 1)
		{
			MeshSerializationData_V1 serializationData{};
			streamReader.Read(serializationData);

			for (uint32_t i = 0; const auto & mat : serializationData.materials)
			{
				mesh->m_materialTable.SetMaterial(mat, i);
				AssetManager::AddDependencyToAsset(metadata.handle, mat);
				i++;
			}

			mesh->InitializeWithVertices(serializationData.vertices);
			mesh->m_indices = serializationData.indices;
			mesh->m_boundingSphere.center = serializationData.boundingSphereCenter;
			mesh->m_boundingSphere.radius = serializationData.boundingSphereRadius;
			mesh->m_subMeshes = serializationData.subMeshes;
		}
		else
		{
			MeshSerializationData_V2 serializationData{};
			streamReader.Read(serializationData);

			for (uint32_t i = 0; const auto & mat : serializationData.materials)
			{
				mesh->m_materialTable.SetMaterial(mat, i);
				AssetManager::AddDependencyToAsset(metadata.handle, mat);
				i++;
			}

			mesh->m_vertexContainer.positions = serializationData.vertexPositions;
			mesh->m_vertexContainer.materialData = serializationData.vertexMaterialData;
			mesh->m_vertexContainer.animationInfo = serializationData.vertexAnimationInfo;
			mesh->m_vertexContainer.animationData = serializationData.vertexAnimationData;
			mesh->m_vertexContainer.boneInfluences = serializationData.vertexBoneInfluences;
			mesh->m_vertexContainer.boneWeights = serializationData.vertexBoneWeights;
			mesh->m_indices = serializationData.indices;
			mesh->m_boundingSphere.center = serializationData.boundingSphereCenter;
			mesh->m_boundingSphere.radius = serializationData.boundingSphereRadius;
			mesh->m_subMeshes = serializationData.subMeshes;
		}

		for (auto& subMesh : mesh->m_subMeshes)
		{
			subMesh.GenerateHash();
		}

		mesh->Construct();
		return true;
	}
}
