#pragma once

#include "AssetImporter.h"

namespace Volt
{
	class ParticlePresetImporter : public AssetImporter
	{
	public:
		bool Load(const AssetMetadata& metadata, Ref<Asset>& asset) const override;
		void Save(const AssetMetadata& metadata, const Ref<Asset>& asset) const override;
	};
}
