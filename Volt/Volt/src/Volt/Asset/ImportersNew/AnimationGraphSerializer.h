#pragma once

#include "Volt/Asset/ImportersNew/AnimationSerializer.h"

namespace Volt
{
	class AnimationGraphSerializer : public AssetSerializer
	{
	public:
		void Serialize(const AssetMetadata& metadata, const Ref<Asset>& asset) const override;
		bool Deserialize(const AssetMetadata& metadata, Ref<Asset> destinationAsset) const override;
	};
}