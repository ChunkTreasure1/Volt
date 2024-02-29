#include "vtpch.h"
#include "AssetSerializer.h"

namespace Volt
{
	void AssetSerializer::WriteMetadata(const AssetMetadata& metadata, const uint32_t version, BinaryStreamWriter& streamWriter)
	{
		SerializedAssetMetadataHeader metadataHeader{};

		SerializedAssetMetadata serializedMetadata{};
		serializedMetadata.handle = metadata.handle;
		serializedMetadata.type = metadata.type;
		serializedMetadata.version = version;

		streamWriter.Write(metadataHeader);
		streamWriter.Write(serializedMetadata);
	}
}
