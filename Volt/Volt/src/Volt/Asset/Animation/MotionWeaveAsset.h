#pragma once
#include "Volt/Asset/Asset.h"

namespace Volt
{
	struct MotionWeaveAssetEntry
	{
		Volt::AssetHandle animation;
	};

	class MotionWeaveAsset : public Asset
	{
	public:
		MotionWeaveAsset() = default;
		MotionWeaveAsset(Volt::AssetHandle targetSkeletonHandle);
		static AssetType GetStaticType() { return AssetType::MotionWeave; }
		AssetType GetType() override { return GetStaticType(); };

		std::vector<MotionWeaveAssetEntry> GetMotionWeaveAssetEntries();
		
	private:
		friend class MotionWeaveImporter;
		Volt::AssetHandle m_TargetSkeletonHandle = Volt::Asset::Null();

		std::vector<MotionWeaveAssetEntry> m_MotionWeaveAssetEntries;

	};
}
