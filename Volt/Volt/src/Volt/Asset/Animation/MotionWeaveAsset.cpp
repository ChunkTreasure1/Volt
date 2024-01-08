#include "vtpch.h"
#include "MotionWeaveAsset.h"

namespace Volt
{
	MotionWeaveAsset::MotionWeaveAsset(Volt::AssetHandle targetSkeletonHandle)
		:m_TargetSkeletonHandle(targetSkeletonHandle)
	{
	}
	std::vector<MotionWeaveAssetEntry> Volt::MotionWeaveAsset::GetMotionWeaveAssetEntries()
	{
		return m_MotionWeaveAssetEntries;
	}
}
