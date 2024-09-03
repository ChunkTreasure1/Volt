#pragma once

#include "Volt/Asset/AssetTypes.h"

#include <AssetSystem/Serialization/AssetSerializer.h>
#include <AssetSystem/AssetSerializerRegistry.h>

namespace Volt
{
	class SourceTextureSerializer : public AssetSerializer
	{
	public:
		void Serialize(const AssetMetadata& metadata, const Ref<Asset>& asset) const override {} // Type is not serializable
		bool Deserialize(const AssetMetadata& metadata, Ref<Asset> destinationAsset) const override;
	};

	VT_REGISTER_ASSET_SERIALIZER(AssetTypes::TextureSource, SourceTextureSerializer);
}
