#pragma once
#include <Volt/Asset/Importers/AssetImporter.h>

namespace Volt
{
	class MotionWeaveImporter : public AssetImporter
	{
		bool Load(const AssetMetadata& metadata, Ref<Asset>& asset) const override;
		void Save(const AssetMetadata& metadata, const Ref<Asset>& asset) const override;
	};
}
