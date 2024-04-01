#include "vtpch.h"
#include "MeshSerializer.h"

#include "Volt/Asset/AssetManager.h"
#include "Volt/Asset/Mesh/Mesh.h"
#include "Volt/Asset/Mesh/Material.h"
#include "Volt/Asset/Serialization/AssetSerializationCommon.h"

namespace Volt
{
	struct MeshSerializationData
	{
		AssetHandle materialHandle;

		std::vector<Vertex> vertices;
		std::vector<uint32_t> indices;

		glm::vec3 boundingSphereCenter;
		float boundingSphereRadius;

		std::vector<SubMesh> subMeshes;

		static void Serialize(BinaryStreamWriter& streamWriter, const MeshSerializationData& data)
		{
			streamWriter.Write(data.materialHandle);
			streamWriter.WriteRaw(data.vertices);
			streamWriter.WriteRaw(data.indices);
			streamWriter.Write(data.boundingSphereCenter);
			streamWriter.Write(data.boundingSphereRadius);
			streamWriter.Write(data.subMeshes);
		}

		static void Deserialize(BinaryStreamReader& streamReader, MeshSerializationData& outData)
		{
			streamReader.Read(outData.materialHandle);
			streamReader.ReadRaw(outData.vertices);
			streamReader.ReadRaw(outData.indices);
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

		MeshSerializationData serializationData{}; 
		serializationData.materialHandle = mesh->GetMaterial()->handle;
		serializationData.vertices = mesh->GetVertices();
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
			VT_CORE_ERROR("File {0} not found!", metadata.filePath);
			destinationAsset->SetFlag(AssetFlag::Missing, true);
			return false;
		}

		BinaryStreamReader streamReader{ filePath };

		if (!streamReader.IsStreamValid())
		{
			VT_CORE_ERROR("Failed to open file: {0}!", metadata.filePath);
			destinationAsset->SetFlag(AssetFlag::Invalid, true);
			return false;
		}

		SerializedAssetMetadata serializedMetadata = AssetSerializer::ReadMetadata(streamReader);
		VT_CORE_ASSERT(serializedMetadata.version == destinationAsset->GetVersion(), "Incompatible version!");

		MeshSerializationData serializationData{};
		streamReader.Read(serializationData);

		Ref<Mesh> mesh = std::reinterpret_pointer_cast<Mesh>(destinationAsset);

		mesh->myMaterial = AssetManager::QueueAsset<Material>(serializationData.materialHandle);
		mesh->myVertices = serializationData.vertices;
		mesh->myIndices = serializationData.indices;
		mesh->myBoundingSphere.center = serializationData.boundingSphereCenter;
		mesh->myBoundingSphere.radius = serializationData.boundingSphereRadius;
		mesh->mySubMeshes = serializationData.subMeshes;

		for (auto& subMesh : mesh->mySubMeshes)
		{
			subMesh.GenerateHash();
		}

		mesh->Construct();
		return true;
	}
}
