#include "gkpch.h"
#include "SequenceNodes.h"

#include "GraphKey/Nodes/Animation/BaseAnimationNodes.h"

#include <Volt/Asset/AssetManager.h>
#include <Volt/Asset/Animation/Skeleton.h>
#include <Volt/Asset/Animation/AnimationGraphAsset.h>
#include "Volt/Animation/AnimationManager.h"

#include <Volt/Asset/AssetManager.h>

#include <Volt/Scene/SceneManager.h>

#include <Volt/Components/Components.h>

#include <Volt/Scripting/Mono/MonoScriptEngine.h>
#include <Volt/Scripting/Mono/MonoScriptInstance.h>

#include <Volt/Asset/Animation/Skeleton.h>

namespace GraphKey
{
	SequencePlayerNode::SequencePlayerNode()
	{
		inputs =
		{
			AttributeConfigDefault("", AttributeDirection::Input, Volt::AssetHandle(0)),
			AttributeConfigDefault("Loop", AttributeDirection::Input, false),
			AttributeConfigDefault("Speed", AttributeDirection::Input, 1.f),
			AttributeConfigDefault("Apply Root Motion", AttributeDirection::Input, false)
		};

		outputs =
		{
			AttributeConfigAnimationPose<AnimationOutputData>("Output", AttributeDirection::Output, GK_BIND_FUNCTION(SequencePlayerNode::TrySampleAnimation))
		};
	}

	Ref<Volt::Animation> SequencePlayerNode::GetAnimation()
	{
		const auto animHandle = GetInput<Volt::AssetHandle>(0);
		return Volt::AssetManager::GetAsset<Volt::Animation>(animHandle);
	}

	const bool SequencePlayerNode::IsLooping()
	{
		return GetInput<bool>(1);
	}

	float SequencePlayerNode::GetCurrentAnimationTime()
	{
		const Volt::AnimationGraphAsset* const animGraph = reinterpret_cast<Volt::AnimationGraphAsset*>(myGraph);
		const float speed = GetInput<float>(2);
		const bool looping = GetInput<bool>(1);
		return GetAnimation()->GetNormalizedCurrentTimeFromStartTime(animGraph->GetStartTime(), speed, looping) * (GetAnimation()->GetDuration() / speed);
	}

	float SequencePlayerNode::GetCurrentAnimationTimeNormalized()
	{
		const Volt::AnimationGraphAsset* const animGraph = reinterpret_cast<Volt::AnimationGraphAsset*>(myGraph);
		const float speed = GetInput<float>(2);
		const bool looping = GetInput<bool>(1);
		
		return GetAnimation()->GetNormalizedCurrentTimeFromStartTime(animGraph->GetStartTime(), speed, looping);
	}

	void SequencePlayerNode::TrySampleAnimation()
	{
		const auto animHandle = GetInput<Volt::AssetHandle>(0);

		Volt::AnimationGraphAsset* animGraph = reinterpret_cast<Volt::AnimationGraphAsset*>(myGraph);
		const auto skeleton = Volt::AssetManager::GetAsset<Volt::Skeleton>(animGraph->GetSkeletonHandle());

		if (!skeleton || !skeleton->IsValid())
		{
			return;
		}

		if (animHandle == Volt::Asset::Null())
		{
			return;
		}

		Ref<Volt::Animation> anim = Volt::AssetManager::GetAsset<Volt::Animation>(animHandle);
		if (!anim || !anim->IsValid())
		{
			return;
		}

		const bool shouldLoop = GetInput<bool>(1);
		const float speed = GetInput<float>(2);
		const bool applyRootMotion = GetInput<bool>(3);
		//const float localTime = Volt::AnimationManager::globalClock - animGraph->GetStartTime();

		const uint32_t currentFrame = anim->GetFrameFromStartTime(animGraph->GetStartTime(), speed);
		/* ANIMATION EVENT STUFF
		const int32_t animationIndex = character->GetAnimationIndexFromHandle(animHandle);
		if (myGraph->GetEntity() != Wire::NullID && Volt::SceneManager::GetActiveScene().lock() && animationIndex != -1 && character->HasAnimationEvents((uint32_t)animationIndex))
		{
			const auto& animEvents = character->GetAnimationEvents((uint32_t)animationIndex);
			for (const auto& event : animEvents)
			{
				if (event.frame == currentFrame && (int32_t)currentFrame != myLastFrame)
				{
					Volt::Entity entity{ myGraph->GetEntity(), Volt::SceneManager::GetActiveScene().lock().get() };
					if (!entity)
					{
						continue;
					}

					if (entity.HasComponent<Volt::MonoScriptComponent>())
					{
						auto& scriptComp = entity.GetComponent<Volt::MonoScriptComponent>();
						for (const auto& script : scriptComp.scriptIds)
						{
							Ref<Volt::MonoScriptInstance> scriptInstance = Volt::MonoScriptEngine::GetInstanceFromId(script);
							if (scriptInstance)
							{
								scriptInstance->InvokeOnAnimationEvent(event.name, event.frame);
							}
						}
					}
				}
			}
		}*/

		myLastFrame = (int32_t)currentFrame;

		AnimationOutputData output{};
		output.pose = anim->SampleTRS(animGraph->GetStartTime(), skeleton, shouldLoop, speed);

		if (applyRootMotion)
		{
			if (currentFrame == 0)
			{
				myLastRootTransform = output.pose.at(0);
			}

			output.rootTRS.position = output.pose.at(0).position - myLastRootTransform.position;
			output.rootTRS.scale = output.pose.at(0).scale - myLastRootTransform.scale;
			output.rootTRS.rotation = output.pose.at(0).rotation * glm::inverse(myLastRootTransform.rotation);

			myLastRootTransform = output.rootTRS;

			output.pose.at(0).position = 0.f;
			output.pose.at(0).rotation = glm::quat{ 1.f, 0.f, 0.f, 0.f };
			output.pose.at(0).scale = 1.f;
		}

		SetOutputData(0, output);
	}
}
