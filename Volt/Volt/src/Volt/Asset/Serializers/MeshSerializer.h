#pragma once

#include "Volt/Asset/Serializers/AssetSerializer.h"

namespace Volt
{
	class MeshSerializer : public AssetSerializer
	{
	public:
		void Serialize(const AssetMetadata& metadata, const Ref<Asset>& asset) const override;
		bool Deserialize(const AssetMetadata& metadata, Ref<Asset> destinationAsset) const override;
	};
}
