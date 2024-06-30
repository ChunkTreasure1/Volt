#pragma once
#include "Volt/Asset/Asset.h"

class AssetWeaveDatabasePanel;
namespace Volt
{
	class MotionWeaveDatabase : public Asset
	{
	public:
		MotionWeaveDatabase();
		MotionWeaveDatabase(AssetHandle skeletonHandle);

		static AssetType GetStaticType() { return AssetType::MotionWeave; }
		AssetType GetType() override { return GetStaticType(); };
		uint32_t GetVersion() const override { return 1; }

		const std::vector<AssetHandle>& GetAnimationHandles();

		void AddAnimation(AssetHandle animationHandle);
		void RemoveAnimation(AssetHandle animationHandle);
		AssetHandle GetSkeletonHandle();
	private:
		friend class MotionWeaveDatabaseSerializer;
		std::vector<AssetHandle> m_AnimationHandles;
		AssetHandle m_skeleton;

		
	};
}
