#include "vtpch.h"
#include "MeshImporter.h"

#include "Volt/Asset/Mesh/Mesh.h"
#include "Volt/Asset/Mesh/Material.h"
#include "Volt/Asset/Serialization/AssetSerializationCommon.h"

#include "Volt/Utility/FileIO/BinaryStreamWriter.h"

namespace Volt
{
	constexpr uint32_t CURRENT_ASSET_VERSION = 1;

	struct MeshSerializationData
	{
		AssetHandle materialHandle;

		uint32_t vertexCount;
		std::vector<Vertex> vertices;

		uint32_t indexCount;
		std::vector<uint32_t> indices;

		glm::vec3 boundingSphereCenter;
		float boundingSphereRadius;

		std::vector<SubMesh> subMeshes;

		static void Serialize(BinaryStreamWriter& streamWriter, const MeshSerializationData& data)
		{
			streamWriter.Write(data.materialHandle);
			streamWriter.Write(data.vertexCount);
			streamWriter.Write(data.vertices);
			streamWriter.Write(data.indexCount);
			streamWriter.Write(data.indices);
			streamWriter.Write(data.boundingSphereCenter);
			streamWriter.Write(data.boundingSphereRadius);
			streamWriter.Write(data.subMeshes);
		}
	};

	void MeshImporter::Serialize(const AssetMetadata& metadata, const Ref<Asset>& asset) const
	{
		Ref<Mesh> mesh = std::reinterpret_pointer_cast<Mesh>(asset);

		BinaryStreamWriter streamWriter{};
		AssetSerializer::WriteMetadata(metadata, CURRENT_ASSET_VERSION, streamWriter);

		MeshSerializationData serializationData;

		streamWriter.Write(static_cast<uint32_t>(mesh->mySubMeshes.size()));
		streamWriter.Write(mesh->myMaterial->handle);
		streamWriter.Write(serializationData);
	}

	bool MeshImporter::Deserialize(const AssetMetadata& metadata, Ref<Asset> destinationAsset) const
	{
		return false;
	}
}
