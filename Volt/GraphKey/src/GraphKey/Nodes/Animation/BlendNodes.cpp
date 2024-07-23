#include "gkpch.h"
#include "BlendNodes.h"

#include "GraphKey/Nodes/Animation/BaseAnimationNodes.h"

#include <Volt/Asset/AssetManager.h>
#include <Volt/Asset/Animation/Skeleton.h>
#include <Volt/Asset/Animation/AnimationGraphAsset.h>
#include <Volt/Asset/Animation/Skeleton.h>
#include <Volt/Animation/BlendSpace.h>

namespace GraphKey
{
	CrossfadeNode::CrossfadeNode()
	{
		inputs =
		{
			AttributeConfigAnimationPose<AnimationOutputData>("A", AttributeDirection::Input),
			AttributeConfigAnimationPose<AnimationOutputData>("B", AttributeDirection::Input),
			AttributeConfigDefault("Alpha", AttributeDirection::Input, 0.f)
		};

		outputs =
		{
			AttributeConfigAnimationPose<AnimationOutputData>("Result", AttributeDirection::Output, GK_BIND_FUNCTION(CrossfadeNode::TryApplyCrossfade))
		};
	}

	void CrossfadeNode::TryApplyCrossfade()
	{
		const auto& a = GetInput<AnimationOutputData>(0);
		const auto& b = GetInput<AnimationOutputData>(1);
		const float alpha = std::clamp(GetInput<float>(2), 0.f, 1.f);

		if (a.pose.size() != b.pose.size())
		{
			SetOutputData(0, a);
		}

		Vector<Volt::Animation::TRS> result{ a.pose.size() };
		for (size_t i = 0; i < result.size(); i++)
		{
			const auto& aTRS = a.pose.at(i);
			const auto& bTRS = b.pose.at(i);

			result[i].position = glm::mix(aTRS.position, bTRS.position, alpha);
			result[i].rotation = glm::slerp(aTRS.rotation, bTRS.rotation, alpha);
			result[i].scale = glm::mix(aTRS.scale, bTRS.scale, alpha);
		}

		AnimationOutputData output{};
		output.pose = result;
		output.rootTRS = a.rootTRS;

		SetOutputData(0, output);
	}

	AdditiveNode::AdditiveNode()
	{
		inputs =
		{
			AttributeConfigAnimationPose<AnimationOutputData>("Base Pose", AttributeDirection::Input),
			AttributeConfigAnimationPose<AnimationOutputData>("Additive Pose", AttributeDirection::Input),
			AttributeConfigDefault("Alpha", AttributeDirection::Input, 0.f)
		};

		outputs =
		{
			AttributeConfigAnimationPose<AnimationOutputData>("Result", AttributeDirection::Output, GK_BIND_FUNCTION(AdditiveNode::TryApplyAdditive))
		};
	}

	void AdditiveNode::TryApplyAdditive()
	{
		Volt::AnimationGraphAsset* animGraph = reinterpret_cast<Volt::AnimationGraphAsset*>(myGraph);
		const auto skeleton = Volt::AssetManager::GetAsset<Volt::Skeleton>(animGraph->GetSkeletonHandle());

		if (!skeleton || !skeleton->IsValid())
		{
			return;
		}

		const auto& base = GetInput<AnimationOutputData>(0);
		const auto& additive = GetInput<AnimationOutputData>(1);
		const auto& additiveBase = skeleton->GetRestPose();

		if (base.pose.size() != additive.pose.size())
		{
			SetOutputData(0, base);
			return;
		}

		Vector<Volt::Animation::TRS> result{ additive.pose.size() };
		for (size_t i = 0; i < result.size(); i++)
		{
			const auto& baseTRS = base.pose.at(i);
			const auto& additiveTRS = additive.pose.at(i);
			const auto& additiveBaseTRS = additiveBase.at(i);

			result[i].position = baseTRS.position + (additiveTRS.position - additiveBaseTRS.position);
			result[i].rotation = glm::normalize(baseTRS.rotation * (glm::inverse(additiveBaseTRS.rotation) * additiveTRS.rotation));
			result[i].scale = baseTRS.scale + (additiveTRS.scale - additiveBaseTRS.scale);
		}

		AnimationOutputData output{};
		output.pose = result;
		output.rootTRS = base.rootTRS;

		SetOutputData(0, output);
	}

	BlendSpaceNode::BlendSpaceNode()
	{
		inputs =
		{
			AttributeConfigDefault("", AttributeDirection::Input, Volt::AssetHandle(0)),
			AttributeConfigDefault("Value", AttributeDirection::Input, glm::vec2(0.f)),
		};

		outputs =
		{
			AttributeConfigAnimationPose<AnimationOutputData>("Result", AttributeDirection::Output, GK_BIND_FUNCTION(BlendSpaceNode::Sample))
		};
	}

	void BlendSpaceNode::Sample()
	{
		const auto blendSpaceHandle = GetInput<Volt::AssetHandle>(0);
		const auto blendSpaceValue = GetInput<glm::vec2>(1);

		const auto blendSpace = Volt::AssetManager::GetAsset<Volt::BlendSpace>(blendSpaceHandle);

		if (blendSpaceHandle == Volt::Asset::Null() || !blendSpace || !blendSpace->IsValid())
		{
			return;
		}

		if (blendSpace->GetAnimations().empty())
		{
			return;
		}

		switch (blendSpace->GetDimension())
		{
			case Volt::BlendSpaceDimension::OneD:
			{
				// Find closest min and max animations;
				const auto closestAnims = FindClosestKeys1D(blendSpace, blendSpaceValue.x);

				const auto minAnimSample = TrySampleAnimation(closestAnims.at(0).second);
				const auto maxAnimSample = TrySampleAnimation(closestAnims.at(1).second);

				if (minAnimSample.size() != maxAnimSample.size())
				{
					return;
				}

				// Calculate current T between the closest animations
				const float t = std::clamp((blendSpaceValue.x - closestAnims.at(0).first) / (closestAnims.at(1).first - closestAnims.at(0).first), 0.f, 1.f);

				Vector<Volt::Animation::TRS> result{ minAnimSample.size() };
				for (size_t i = 0; i < result.size(); i++)
				{
					const auto& aTRS = minAnimSample.at(i);
					const auto& bTRS = maxAnimSample.at(i);

					result[i].position = glm::mix(aTRS.position, bTRS.position, t);
					result[i].rotation = glm::slerp(aTRS.rotation, bTRS.rotation, t);
					result[i].scale = glm::mix(aTRS.scale, bTRS.scale, t);
				}

				AnimationOutputData output{};
				output.pose = result;

				SetOutputData(0, output);
				break;
			}

			case Volt::BlendSpaceDimension::TwoD:
			{
				const auto sortedAnims = GetSortedAnimationWeights(blendSpace, blendSpaceValue);
				const size_t jointCount = GetSkeletonJointCount();

				Vector<Volt::Animation::TRS> result{ jointCount };

				for (size_t i = 1; i < sortedAnims.size(); i++)
				{
					const auto& [firstValue, firstAnim] = sortedAnims.at(i - 1);
					const auto& [secondValue, secondAnim] = sortedAnims.at(i);

					const auto firstAnimSample = TrySampleAnimation(firstAnim);
					const auto secondAnimSample = TrySampleAnimation(secondAnim);

					// Skip animations that have a different joint count
					if (firstAnimSample.size() != jointCount || secondAnimSample.size() != jointCount)
					{
						continue;
					}

					for (size_t j = 0; j < result.size(); j++)
					{
						const auto& aTRS = firstAnimSample.at(j);
						const auto& bTRS = secondAnimSample.at(j);

						if (i > 1)
						{
							result[j].position = glm::mix(result[j].position, bTRS.position, secondValue);
							result[j].rotation = glm::slerp(result[j].rotation, bTRS.rotation, secondValue);
							result[j].scale = glm::mix(result[j].scale, bTRS.scale, secondValue);
						}
						else
						{
							result[j].position = glm::mix(aTRS.position, bTRS.position, secondValue);
							result[j].rotation = glm::slerp(aTRS.rotation, bTRS.rotation, secondValue);
							result[j].scale = glm::mix(aTRS.scale, bTRS.scale, secondValue);
						}

					}
				}

				AnimationOutputData output{};
				output.pose = result;

				SetOutputData(0, output);
				break;
			}
		}
	}

	const std::array<std::pair<float, Volt::AssetHandle>, 2> BlendSpaceNode::FindClosestKeys1D(Ref<Volt::BlendSpace> blendSpace, float blendValue)
	{
		float closestMinValue = blendSpace->GetHorizontalValues().x;
		float closestMaxValue = blendSpace->GetHorizontalValues().y;

		Volt::AssetHandle closestMinAnim = Volt::Asset::Null();
		Volt::AssetHandle closestMaxAnim = Volt::Asset::Null();

		for (const auto& [value, anim] : blendSpace->GetAnimations())
		{
			if (value.x >= closestMinValue && value.x <= blendValue)
			{
				closestMinValue = value.x;
				closestMaxAnim = anim;
			}

			if (value.x <= closestMaxValue && value.x >= blendValue)
			{
				closestMaxValue = value.x;
				closestMaxAnim = anim;
			}
		}

		return { std::pair{ closestMinValue, closestMinAnim }, std::pair{ closestMaxValue, closestMaxAnim } };
	}

	const Vector<Volt::Animation::TRS> BlendSpaceNode::TrySampleAnimation(Volt::AssetHandle animationHandle)
	{
		Volt::AnimationGraphAsset* animGraph = reinterpret_cast<Volt::AnimationGraphAsset*>(myGraph);
		const auto skeleton = Volt::AssetManager::GetAsset<Volt::Skeleton>(animGraph->GetSkeletonHandle());

		if (!skeleton || !skeleton->IsValid())
		{
			return {};
		}

		if (animationHandle == Volt::Asset::Null())
		{
			return {};
		}

		Ref<Volt::Animation> anim = Volt::AssetManager::GetAsset<Volt::Animation>(animationHandle);
		if (!anim || !anim->IsValid())
		{
			return {};
		}
		return anim->SampleTRS(animGraph->GetStartTime(), skeleton, true);
	}

	const Vector<std::pair<float, Volt::AssetHandle>> BlendSpaceNode::GetSortedAnimationWeights(Ref<Volt::BlendSpace> blendSpace, const glm::vec2& blendValue)
	{
		if (blendSpace->GetAnimations().size() < 3)
		{
			return {};
		}

		Vector<std::pair<float, Volt::AssetHandle>> weights;
		Vector<std::pair<float, Volt::AssetHandle>> distances;

		for (const auto& [value, anim] : blendSpace->GetAnimations())
		{
			const float distance = glm::distance(value, blendValue);
			distances.emplace_back(distance, anim);
		}

		std::sort(distances.begin(), distances.end(), [](const auto& lhs, const auto& rhs)
		{
			return lhs.first < rhs.first;
		});

		for (uint32_t i = 0; i < 3; i++)
		{
			auto& [weight, anim] = weights.emplace_back(distances.at(i).first, distances.at(i).second);

			if (weight == 0.f)
			{
				weight = 1.f;
				weights.emplace_back(0.f, distances.at(1).second);
				weights.emplace_back(0.f, distances.at(2).second);
				break;
			}

			weight = 1.f / weight;
		}

		// Normalize weights
		float weightSum = 0.f;
		for (const auto& [weight, anim] : weights)
		{
			weightSum += weight;
		}

		for (auto& [weight, anim] : weights)
		{
			weight /= weightSum;
		}

		std::sort(weights.begin(), weights.end(), [](const auto& lhs, const auto& rhs)
		{
			return lhs.first > rhs.first;
		});

		return weights;
	}

	const size_t BlendSpaceNode::GetSkeletonJointCount() const
	{
		Volt::AnimationGraphAsset* animGraph = reinterpret_cast<Volt::AnimationGraphAsset*>(myGraph);
		const auto skeleton = Volt::AssetManager::GetAsset<Volt::Skeleton>(animGraph->GetSkeletonHandle());

		if (!skeleton || !skeleton->IsValid())
		{
			return 0;
		}

		return skeleton->GetJointCount();
	}

	LayeredBlendPerBoneNode::LayeredBlendPerBoneNode()
	{
		inputs =
		{
			AttributeConfigAnimationPose<AnimationOutputData>("Base Pose", AttributeDirection::Input),
			AttributeConfigAnimationPose<AnimationOutputData>("Blend Pose", AttributeDirection::Input),
			AttributeConfigDefault("Alpha", AttributeDirection::Input, 0.f)
		};

		outputs =
		{
			AttributeConfigAnimationPose<AnimationOutputData>("Result", AttributeDirection::Output, GK_BIND_FUNCTION(LayeredBlendPerBoneNode::TryApplyLayeredBlendPerBone))
		};
	}

	void LayeredBlendPerBoneNode::Serialize(YAMLStreamWriter& out)
	{
		out.BeginSequence("BlendIncludeFilters");
		for (const auto& filter : myIncludeFilters)
		{
			out.AddValue(filter.boneName);
		}
		out.EndSequence();

		out.BeginSequence("BlendExcludeFilters");
		for (const auto& filter : myExcludeFilters)
		{
			out.AddValue(filter.boneName);
		}
		out.EndSequence();
	}

	void LayeredBlendPerBoneNode::Deserialize(YAMLStreamReader& node)
	{
		if (node.HasKey("BlendIncludeFilters"))
		{
			node.ForEach("BlendIncludeFilters", [&]()
			{
				myIncludeFilters.emplace_back(node.ReadValue<std::string>());
			});
		}

		if (node.HasKey("BlendExcludeFilters"))
		{
			node.ForEach("BlendExcludeFilters", [&]()
			{
				myExcludeFilters.emplace_back(node.ReadValue<std::string>());
			});
		}
	}

	Ref<Node> LayeredBlendPerBoneNode::CreateCopy(Graph* ownerGraph, Volt::EntityID entityId)
	{
		Ref<Node> copy = Node::CreateCopy(ownerGraph, entityId);
		Ref<LayeredBlendPerBoneNode> blendNode = std::reinterpret_pointer_cast<LayeredBlendPerBoneNode>(copy);
		blendNode->myIncludeFilters = myIncludeFilters;
		blendNode->myExcludeFilters = myExcludeFilters;

		return copy;
	}

	VT_OPTIMIZE_OFF
		void LayeredBlendPerBoneNode::TryApplyLayeredBlendPerBone()
	{
		Volt::AnimationGraphAsset* animGraph = reinterpret_cast<Volt::AnimationGraphAsset*>(myGraph);
		const auto skeleton = Volt::AssetManager::GetAsset<Volt::Skeleton>(animGraph->GetSkeletonHandle());

		if (!skeleton || !skeleton->IsValid())
		{
			return;
		}

		const auto& basePose = GetInput<AnimationOutputData>(0);
		const auto& blendPose = GetInput<AnimationOutputData>(1);
		const float alpha = std::clamp(GetInput<float>(2), 0.f, 1.f);

		if (basePose.pose.size() != blendPose.pose.size())
		{
			SetOutputData(0, basePose);
		}

		Vector<int32_t> includedBones;
		Vector<int32_t> excludedBones;

		for (const auto& filter : myIncludeFilters)
		{
			int32_t jointIndex = skeleton->GetJointIndexFromName(filter.boneName);
			if (jointIndex != -1)
			{
				includedBones.emplace_back(jointIndex);
			}
		}

		for (const auto& filter : myExcludeFilters)
		{
			int32_t jointIndex = skeleton->GetJointIndexFromName(filter.boneName);
			if (jointIndex != -1)
			{
				excludedBones.emplace_back(jointIndex);
			}
		}

		Vector<Volt::Animation::TRS> result{ basePose.pose.size() };
		for (size_t i = 0; i < result.size(); i++)
		{
			bool shouldSkip = true;

			for (const auto& includedBone : includedBones)
			{
				if (skeleton->JointIsDecendantOf((int32_t)i, includedBone))
				{
					shouldSkip = false;
					break;
				}
			}

			for (const auto& excludedBone : excludedBones)
			{
				if (skeleton->JointIsDecendantOf((int32_t)i, excludedBone))
				{
					shouldSkip = true;
					break;
				}
			}

			const auto& aTRS = basePose.pose.at(i);

			if (shouldSkip || (int32_t)i > (int32_t)blendPose.pose.size() - 1)
			{
				result[i].position = aTRS.position;
				result[i].rotation = aTRS.rotation;
				result[i].scale = aTRS.scale;

				continue;
			}

			const auto& bTRS = blendPose.pose.at(i);

			result[i].position = glm::mix(aTRS.position, bTRS.position, alpha);
			result[i].rotation = glm::slerp(aTRS.rotation, bTRS.rotation, alpha);
			result[i].scale = glm::mix(aTRS.scale, bTRS.scale, alpha);
		}

		AnimationOutputData output{};
		output.pose = result;
		SetOutputData(0, output);
	}

	RotateBoneNode::RotateBoneNode()
	{
		inputs =
		{
			AttributeConfigAnimationPose<AnimationOutputData>("Pose", AttributeDirection::Input),
			AttributeConfigDefault("Bone Name", AttributeDirection::Input, std::string("")),
			AttributeConfigDefault("Rotation", AttributeDirection::Input, glm::vec3{ 0.f })
		};

		outputs =
		{
			AttributeConfigAnimationPose<AnimationOutputData>("Result", AttributeDirection::Output, GK_BIND_FUNCTION(RotateBoneNode::RotateBone))
		};
	}

	void RotateBoneNode::RotateBone()
	{
		Volt::AnimationGraphAsset* animGraph = reinterpret_cast<Volt::AnimationGraphAsset*>(myGraph);
		const auto skeleton = Volt::AssetManager::GetAsset<Volt::Skeleton>(animGraph->GetSkeletonHandle());

		if (!skeleton || !skeleton->IsValid())
		{
			return;
		}

		auto basePose = GetInput<AnimationOutputData>(0);
		const auto& boneName = GetInput<std::string>(1);
		const auto& rotation = GetInput<glm::vec3>(2);

		int32_t jointIndex = skeleton->GetJointIndexFromName(boneName);
		if (jointIndex != -1)
		{
			basePose.pose[jointIndex].rotation *= glm::quat{ glm::radians(rotation) };
		}

		SetOutputData(0, basePose);
	}
}
