#pragma once

#include "Volt/Asset/ImportersNew/AssetSerializer.h"

namespace Volt
{
	class SourceMeshImporter : public AssetSerializer
	{
	public:
		void Serialize(const AssetMetadata& metadata, const Ref<Asset>& asset) const override {} // Type is not serializable
		bool Deserialize(const AssetMetadata& metadata, Ref<Asset> destinationAsset) const override;
	};
}
