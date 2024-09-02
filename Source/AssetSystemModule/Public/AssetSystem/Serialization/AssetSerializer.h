#pragma once

#include "AssetSystem/Asset.h"
#include "AssetSystem/Serialization/AssetSerializationCommon.h"

#include <CoreUtilities/FileIO/BinaryStreamWriter.h>
#include <CoreUtilities/FileIO/BinaryStreamReader.h>

namespace Volt
{
	class VTAS_API AssetSerializer
	{
	public:
		virtual ~AssetSerializer() = default;

		virtual void Serialize(const AssetMetadata& metadata, const Ref<Asset>& asset) const = 0;
		virtual bool Deserialize(const AssetMetadata& metadata, Ref<Asset> destinationAsset) const = 0;

		[[nodiscard]] static size_t WriteMetadata(const AssetMetadata& metadata, const uint32_t version, BinaryStreamWriter& streamWriter);
		static SerializedAssetMetadata ReadMetadata(BinaryStreamReader& streamReader);
	};
}
