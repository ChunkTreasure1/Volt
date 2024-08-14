#pragma once

#include "Volt/Asset/Asset.h"

class BinaryStreamWriter;
class BinaryStreamReader;

namespace Volt
{
	struct SerializedAssetMetadata
	{
		inline static constexpr uint32_t AssetMagic = 9999;

		AssetType type;
		uint32_t version;
		AssetHandle handle;

		static void Serialize(BinaryStreamWriter& streamWriter, const SerializedAssetMetadata& data);
		static void Deserialize(BinaryStreamReader& streamReader, SerializedAssetMetadata& outData);
	};
}
