#include "vtpch.h"
#include "MotionWeaveDatabase.h"

#include <AssetSystem/AssetManager.h>

namespace Volt
{
	VT_REGISTER_ASSET_FACTORY(AssetTypes::MotionWeave, MotionWeaveDatabase);

	MotionWeaveDatabase::MotionWeaveDatabase()
	{
		m_skeleton = Asset::Null();
	}

	MotionWeaveDatabase::MotionWeaveDatabase(AssetHandle skeletonHandle)
	{
		const AssetMetadata& metadata = AssetManager::GetMetadataFromHandle(skeletonHandle);
		metadata;
		VT_ASSERT_MSG(metadata.type == AssetTypes::Skeleton, "Asset is not a skeleton!");

		m_skeleton = skeletonHandle;
	}
	const Vector<AssetHandle>& MotionWeaveDatabase::GetAnimationHandles()
	{
		return m_AnimationHandles;
	}
	void MotionWeaveDatabase::AddAnimation(AssetHandle animationHandle)
	{
		const AssetMetadata& metadata = AssetManager::GetMetadataFromHandle(animationHandle);
		metadata;
		VT_ASSERT_MSG(metadata.type == AssetTypes::Animation, "Asset is not an animation!");

		m_AnimationHandles.push_back(animationHandle);
	}
	void MotionWeaveDatabase::RemoveAnimation(AssetHandle animationHandle)
	{
		const AssetMetadata& metadata = AssetManager::GetMetadataFromHandle(animationHandle);
		metadata;
		VT_ASSERT_MSG(metadata.type == AssetTypes::Animation, "Asset is not an animation!");

		auto it = std::find(m_AnimationHandles.begin(), m_AnimationHandles.end(), animationHandle);
		if (it != m_AnimationHandles.end())
		{
			m_AnimationHandles.erase(it);
		}
	}
	AssetHandle MotionWeaveDatabase::GetSkeletonHandle()
	{
		return m_skeleton;
	}
}
