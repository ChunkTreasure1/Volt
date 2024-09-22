#pragma once

#include "Volt-Assets/Assets/Font.h"
#include "Volt-Assets/Config.h"

#include <AssetSystem/Serialization/AssetSerializer.h>
#include <AssetSystem/AssetSerializerRegistry.h>

namespace Volt
{
	class VTASSETS_API FontSerializer : public AssetSerializer
	{
	public:
		void Serialize(const AssetMetadata& metadata, const Ref<Asset>& asset) const override;
		bool Deserialize(const AssetMetadata& metadata, Ref<Asset> destinationAsset) const override;
	};

	VT_REGISTER_ASSET_SERIALIZER(AssetTypes::Font, FontSerializer);
}
