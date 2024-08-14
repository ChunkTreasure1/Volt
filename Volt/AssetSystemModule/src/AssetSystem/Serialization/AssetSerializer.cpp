#include "aspch.h"
#include "AssetSerializer.h"

namespace Volt
{
	size_t AssetSerializer::WriteMetadata(const AssetMetadata& metadata, const uint32_t version, BinaryStreamWriter& streamWriter)
	{
		SerializedAssetMetadata serializedMetadata{};
		serializedMetadata.handle = metadata.handle;
		serializedMetadata.type = metadata.type;
		serializedMetadata.version = version;

		streamWriter.Write(SerializedAssetMetadata::AssetMagic);
		return streamWriter.Write(serializedMetadata);
	}

	SerializedAssetMetadata AssetSerializer::ReadMetadata(BinaryStreamReader& streamReader)
	{
		uint32_t magic;
		streamReader.Read(magic);
		VT_ASSERT(magic == SerializedAssetMetadata::AssetMagic);

		SerializedAssetMetadata result{};
		streamReader.Read(result);
		return result;
	}

}
