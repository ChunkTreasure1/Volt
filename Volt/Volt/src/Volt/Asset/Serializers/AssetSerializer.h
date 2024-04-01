#pragma once

#include "Volt/Asset/Asset.h"

#include "Volt/Asset/Serialization/AssetSerializationCommon.h"

#include "Volt/Utility/FileIO/BinaryStreamWriter.h"
#include "Volt/Utility/FileIO/BinaryStreamReader.h"

namespace Volt
{
	class AssetSerializer
	{
	public:
		virtual ~AssetSerializer() = default;

		virtual void Serialize(const AssetMetadata& metadata, const Ref<Asset>& asset) const = 0;
		virtual bool Deserialize(const AssetMetadata& metadata, Ref<Asset> destinationAsset) const = 0;

		[[nodiscard]] static size_t WriteMetadata(const AssetMetadata& metadata, const uint32_t version, BinaryStreamWriter& streamWriter);
		static SerializedAssetMetadata ReadMetadata(BinaryStreamReader& streamReader);
	};
}
