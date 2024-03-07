#pragma once

#include "Volt/Asset/Asset.h"

namespace Volt
{
	class BinaryStreamWriter;
	class BinaryStreamReader;

	struct SerializedAssetMetadata
	{
		AssetHandle handle;
		AssetType type;
		uint32_t version;

		std::vector<AssetHandle> dependencies;

		static void Serialize(BinaryStreamWriter& streamWriter, const SerializedAssetMetadata& data);
		static void Deserialize(BinaryStreamReader& streamReader, SerializedAssetMetadata& outData);
	};
}
