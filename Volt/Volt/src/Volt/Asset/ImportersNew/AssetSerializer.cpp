#include "vtpch.h"
#include "AssetSerializer.h"

namespace Volt
{
	size_t AssetSerializer::WriteMetadata(const AssetMetadata& metadata, const uint32_t version, BinaryStreamWriter& streamWriter)
	{
		SerializedAssetMetadata serializedMetadata{};
		serializedMetadata.handle = metadata.handle;
		serializedMetadata.type = metadata.type;
		serializedMetadata.version = version;
		serializedMetadata.dependencies = metadata.dependencies;

		return streamWriter.Write(serializedMetadata);
	}

	SerializedAssetMetadata AssetSerializer::ReadMetadata(BinaryStreamReader& streamReader)
	{
		SerializedAssetMetadata result{};
		streamReader.Read(result);
		return result;
	}

}
