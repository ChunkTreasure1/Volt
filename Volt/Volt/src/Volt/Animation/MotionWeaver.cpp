#include "vtpch.h"
#include "MotionWeaver.h"
#include "Volt/Log/Log.h"

#include "Volt/Animation/AnimationManager.h"

#include "Volt/Asset/Animation/Animation.h"
#include "Volt/Asset/Animation/Skeleton.h"
#include "Volt/Asset/AssetManager.h"

namespace Volt
{

	MotionWeaver::MotionWeaver(Ref<MotionWeaveAsset> motionWeaveAsset, Ref<Skeleton> skeleton) 
		: m_MotionWeaveAsset(motionWeaveAsset), m_Skeleton(skeleton)
	{
		assert(m_MotionWeaveAsset);
		assert(m_Skeleton);
	}

	MotionWeaver::~MotionWeaver()
	{
	}

	void MotionWeaver::Update(float deltaTime)
	{
		if (m_MotionWeaveAsset->GetMotionWeaveAssetEntries().empty())
		{
			return;
		}
		
		if (m_Entries.empty())
		{
			MotionWeaveAssetEntry& entry = m_MotionWeaveAsset->GetMotionWeaveAssetEntries()[0];
			
			MotionWeaveEntry newEntry;
			newEntry.animation = AssetManager::GetAsset<Animation>(entry.animation);
			newEntry.speed = 1;
			newEntry.weight = 1;
			newEntry.looping = true;
			newEntry.startTime = 0;
			
			m_Entries.push_back(newEntry);
		}
	}

	const std::vector<glm::mat4> MotionWeaver::Sample()
	{
		if (m_Entries.empty())
		{
			return std::vector<glm::mat4>();
		}
		
		auto targetEntry = m_Entries.front();
		const Animation::Pose sample = targetEntry.animation->SamplePose(targetEntry.startTime, AnimationManager::globalClock, m_Skeleton, targetEntry.looping, targetEntry.speed);
		const auto& invBindPose = m_Skeleton->GetInverseBindPose();

		if(sample.localTRS.size() != invBindPose.size())
		{
			VT_CORE_ERROR("Sampled pose size does not match inverse bind pose size");
			return std::vector<glm::mat4>();
		}
		if (sample.localTRS.empty())
		{
			VT_CORE_ERROR("Sampled pose is empty");
			return std::vector<glm::mat4>();
		}
		
		std::vector<glm::mat4> result{};
		result.resize(sample.localTRS.size());

		glm::vec3 rootMotion = sample.localTRS[0].position - m_PrevRootPosition;
		m_PrevRootPosition = sample.localTRS[0].position;

		for (size_t i = 0; i < sample.localTRS.size(); i++)
		{
			const auto& trs = sample.localTRS.at(i);

			const glm::mat4 transform = glm::translate(glm::mat4{ 1.f }, trs.position)* glm::mat4_cast(trs.rotation)* glm::scale(glm::mat4{ 1.f }, trs.scale);
			result[i] = transform * invBindPose[i];
		}

		return result;
	}

}
