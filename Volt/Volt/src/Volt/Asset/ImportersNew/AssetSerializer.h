#pragma once

#include "Volt/Asset/Asset.h"

#include "Volt/Asset/Serialization/AssetSerializationCommon.h"

#include "Volt/Utility/FileIO/BinaryStreamWriter.h"

namespace Volt
{
	class AssetSerializer
	{
	public:
		virtual ~AssetSerializer() = default;

		virtual void Serialize(const AssetMetadata& metadata, const Ref<Asset>& asset) const = 0;
		virtual bool Deserialize(const AssetMetadata& metadata, Ref<Asset> destinationAsset) const = 0;

		static void WriteMetadata(const AssetMetadata& metadata, const uint32_t version, BinaryStreamWriter& streamWriter);
	};
}
