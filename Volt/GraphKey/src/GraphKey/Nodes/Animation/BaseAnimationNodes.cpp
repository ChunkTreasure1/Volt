#include "gkpch.h"
#include "BaseAnimationNodes.h"

#include <Volt/Asset/AssetManager.h>
#include <Volt/Asset/Animation/Skeleton.h>
#include <Volt/Asset/Animation/AnimationGraphAsset.h>
#include <Volt/Asset/Animation/AnimatedCharacter.h>

namespace GraphKey
{
	OutputPoseNode::OutputPoseNode()
	{
		inputs =
		{
			AttributeConfig("Result", AttributeDirection::Input)
		};
	}

	const AnimationOutputData OutputPoseNode::Sample(bool moveToWorldSpace, float startTime)
	{
		VT_PROFILE_FUNCTION();

		AnimationOutputData output{};

		Volt::AnimationGraphAsset* animGraph = reinterpret_cast<Volt::AnimationGraphAsset*>(myGraph);
		const auto character = Volt::AssetManager::GetAsset<Volt::AnimatedCharacter>(animGraph->GetCharacterHandle());

		if (!character || !character->IsValid())
		{
			return output;
		}

		animGraph->SetStartTime(startTime);

		auto input = GetInput<AnimationOutputData>(0);
		if (input.pose.empty())
		{
			return output;
		}

		output.pose.resize(input.pose.size());

		if (moveToWorldSpace)
		{
			const auto& joints = character->GetSkeleton()->GetJoints();
			for (size_t i = 0; i < joints.size(); i++)
			{
				const auto& joint = joints[i];

				Volt::Animation::TRS parentTransform{};
				if (joint.parentIndex >= 0)
				{
					parentTransform = output.pose.at(joint.parentIndex);
				}

				output.pose[i].position = parentTransform.position + parentTransform.rotation * input.pose.at(i).position;
				output.pose[i].rotation = parentTransform.rotation * input.pose.at(i).rotation;
				output.pose[i].scale = parentTransform.scale * input.pose.at(i).scale;
			}
		}
		else
		{
			output.pose = input.pose;
		}

		output.rootTRS = input.rootTRS;

		return output;
	}
}
