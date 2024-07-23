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
		auto asset = AssetManager::GetAsset<MotionWeaveDatabase>(databaseHandle);
		if (!asset)
		{
			return nullptr;
		}

		return CreateRef<MotionWeaver>(asset);
	}

	void Volt::MotionWeaver::SetDatabase(Ref<MotionWeaveDatabase> database)
	{
		m_Database = database;

		for (const AssetHandle& handle : m_Database->GetAnimationHandles())
		{
			m_Animations.push_back(AssetManager::GetAsset<Animation>(handle));
		}

		m_Skeleton = AssetManager::GetAsset<Skeleton>(m_Database->GetSkeletonHandle());
	}

	void Volt::MotionWeaver::Update(float deltaTime)
	{
		VT_PROFILE_FUNCTION();

		if (m_Animations.empty())
		{
			return;
		}

		m_Time += deltaTime;
		if (m_Time >= m_Animations.front()->GetDuration())
		{
			m_Time = 0;
		}
	}

	Vector<glm::mat4x4> Volt::MotionWeaver::Sample()
	{
		VT_PROFILE_FUNCTION();

		if (m_Animations.empty())
		{
			return {};
		}

		const float percent = m_Time / m_Animations.front()->GetDuration();
		Vector<glm::mat4x4> result = m_Animations.front()->Sample(percent, m_Skeleton, true);

		return result;
	}

}
