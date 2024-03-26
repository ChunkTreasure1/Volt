#pragma once

#include "Volt/Asset/ImportersNew/AssetSerializer.h"

namespace Volt
{
	class PrefabSerializer : public	AssetSerializer
	{
	public:
		PrefabSerializer();
		~PrefabSerializer() override;

		void Serialize(const AssetMetadata& metadata, const Ref<Asset>& asset) const override;
		bool Deserialize(const AssetMetadata& metadata, Ref<Asset> destinationAsset) const override;

	private:
		inline static PrefabSerializer* s_instance = nullptr;
	};
}
