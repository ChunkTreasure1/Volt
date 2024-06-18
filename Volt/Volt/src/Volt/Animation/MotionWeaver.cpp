#include "vtpch.h"
#include "MotionWeaver.h"
#include "Volt/Asset/Animation/MotionWeaveDatabase.h"
#include "Volt/Asset/AssetManager.h"
#include "Volt/Asset/Animation/Animation.h"
#include "Volt/Asset/Animation/Skeleton.h"

namespace Volt
{

	Volt::MotionWeaver::MotionWeaver(Ref<MotionWeaveDatabase> database)
	{
		m_Time = 0;
		SetDatabase(database);
	}

	Ref<MotionWeaver> Volt::MotionWeaver::Create(AssetHandle databaseHandle)
	{
		return CreateRef<MotionWeaver>(AssetManager::GetAsset<MotionWeaveDatabase>(databaseHandle));
	}

	void Volt::MotionWeaver::SetDatabase(Ref<MotionWeaveDatabase> database)
	{
		m_Database = database;

		for (AssetHandle& handle : m_Database->GetAnimationHandles())
		{
			m_Animations.push_back(AssetManager::GetAsset<Animation>(handle));
		}

		m_Skeleton = AssetManager::GetAsset<Skeleton>(m_Database->GetSkeletonHandle());
	}

	void Volt::MotionWeaver::Update(float deltaTime)
	{
		m_Time += deltaTime;
		if (m_Time >= m_Animations.front()->GetDuration())
		{
			m_Time = 0;
		}
	}

	std::vector<glm::mat4x4> Volt::MotionWeaver::Sample()
	{
		const float percent = m_Time / m_Animations.front()->GetDuration();
		std::vector<glm::mat4x4> result = m_Animations.front()->Sample(percent, m_Skeleton, true);

		return result;
	}

}
