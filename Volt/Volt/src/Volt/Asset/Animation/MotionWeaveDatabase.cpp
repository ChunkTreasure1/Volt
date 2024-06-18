#include "vtpch.h"
#include "MotionWeaveDatabase.h"
namespace Volt
{
	std::vector<AssetHandle> MotionWeaveDatabase::GetAnimationHandles()
	{
		return m_AnimationHandles;
	}
	AssetHandle MotionWeaveDatabase::GetSkeletonHandle()
	{
		return m_Skeleton;
	}
}
