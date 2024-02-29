#pragma once

#include "Volt/Asset/Asset.h"

namespace Volt
{
	class BinaryStreamWriter;

	struct SerializedAssetMetadata
	{
		AssetHandle handle;
		AssetType type;
		uint32_t version;

		static void Serialize(BinaryStreamWriter& streamWriter, const SerializedAssetMetadata& data);
	};

	struct SerializedAssetMetadataHeader
	{
		size_t assetMetadataSize;
	};
}
