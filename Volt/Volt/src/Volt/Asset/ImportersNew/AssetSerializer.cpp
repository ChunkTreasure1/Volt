#include "vtpch.h"
#include "AssetSerializer.h"

namespace Volt
{
	void AssetSerializer::WriteMetadata(const AssetMetadata& metadata, const uint32_t version, BinaryStreamWriter& streamWriter)
	{
		SerializedAssetMetadataHeader metadataHeader{};
		metadataHeader.assetMetadataSize = sizeof(SerializedAssetMetadata);

		SerializedAssetMetadata serializedMetadata{};
		serializedMetadata.handle = metadata.handle;
		serializedMetadata.type = metadata.type;
		serializedMetadata.version = version;

		streamWriter.Write(metadataHeader);
		streamWriter.Write(serializedMetadata);
	}

	SerializedAssetMetadata AssetSerializer::ReadMetadata(BinaryStreamReader& streamReader)
	{
		SerializedAssetMetadataHeader header{};
		SerializedAssetMetadata result{};

		streamReader.Read(header);
		VT_CORE_ASSERT(sizeof(SerializedAssetMetadata) == header.assetMetadataSize, "Size mismatch!");

		streamReader.Read(result);
		return result;
	}

}
