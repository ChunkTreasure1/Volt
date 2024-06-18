#pragma once
#include "Volt/Asset/Asset.h"

namespace Volt
{
	class MotionWeaveDatabase : public Asset
	{
	public:
		static AssetType GetStaticType() { return AssetType::MotionWeave; }
		AssetType GetType() override { return GetStaticType(); };
		uint32_t GetVersion() const override { return 1; }

		std::vector<AssetHandle> GetAnimationHandles();
		AssetHandle GetSkeletonHandle();
	private:
		friend class MotionWeaveDatabaseSerializer;

		std::vector<AssetHandle> m_AnimationHandles;
		AssetHandle m_Skeleton;

		
	};
}
