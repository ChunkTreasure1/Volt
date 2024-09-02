#pragma once

#include "AssetSystem/Asset.h"

class BinaryStreamWriter;
class BinaryStreamReader;

namespace Volt
{
	struct VTAS_API SerializedAssetMetadata
	{
		inline static constexpr uint32_t AssetMagic = 9999;
		inline static constexpr size_t HeaderSize = sizeof(AssetType) + sizeof(uint32_t) + sizeof(AssetHandle) + sizeof(TypeHeader) * 9;

		AssetType type;
		uint32_t version;
		AssetHandle handle;

		static void Serialize(BinaryStreamWriter& streamWriter, const SerializedAssetMetadata& data);
		static void Deserialize(BinaryStreamReader& streamReader, SerializedAssetMetadata& outData);
	};
}
