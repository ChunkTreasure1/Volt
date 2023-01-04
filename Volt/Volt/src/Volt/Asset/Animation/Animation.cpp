#include "vtpch.h"
#include "Animation.h"

#include "Volt/Asset/Animation/Skeleton.h"
#include "Volt/Animation/AnimationManager.h"
#include "Volt/Core/Profiling.h"

#include "Volt/Math/MatrixUtilities.h"

namespace Volt
{
	const std::vector<gem::mat4> Animation::Sample(float aStartTime, Ref<Skeleton> aSkeleton, bool looping)
	{
		VT_PROFILE_FUNCTION();

		const float localTime = AnimationManager::globalClock - aStartTime;
		const float normalizedTime = localTime / myDuration;

		const int32_t frameCount = (int32_t)myFrames.size();
		const int32_t currentFrameIndex = frameCount - std::abs((int32_t)(std::floor(normalizedTime * (float)frameCount)) % (2 * frameCount) - frameCount);

		int32_t nextFrameIndex = currentFrameIndex + 1;

		if (nextFrameIndex >= frameCount)
		{
			if (looping)
			{
				nextFrameIndex = 0;
			}
			else
			{
				nextFrameIndex = currentFrameIndex;
			}
		}

		std::vector<gem::mat4> result;
		result.resize(aSkeleton->GetJointCount(), gem::mat4(1.f));

		const float animDelta = 1.f / (float)myFramesPerSecond;
		const float frameTime = localTime / animDelta;
		const float deltaTime = frameTime - (float)currentFrameIndex;

		VT_CORE_TRACE("Frame index {0}", currentFrameIndex);


		const Frame& currentFrame = myFrames.at(currentFrameIndex);
		const Frame& nextFrame = myFrames.at(nextFrameIndex);

		const auto& joints = aSkeleton->GetJoints();
		const auto& invBindPoses = aSkeleton->GetInverseBindPoses();

		for (size_t i = 0; i < joints.size(); i++)
		{
			const auto& joint = joints[i];

			gem::mat4 parentTransform = { 1.f };

			if (joint.parentIndex >= 0)
			{
				parentTransform = result[joint.parentIndex];
			}

			const auto& currentLocalTransform = currentFrame.localTRS[i];
			const auto& nextLocalTransform = nextFrame.localTRS[i];

			gem::mat4 resultTransform;

			// Blend
			{
				const gem::vec3 position = gem::lerp(currentLocalTransform.position, nextLocalTransform.position, 0.5f);
				const gem::quat rotation = gem::slerp(gem::normalize(currentLocalTransform.rotation), gem::normalize(nextLocalTransform.rotation), 0.5f);
				const gem::vec3 scale = gem::lerp(currentLocalTransform.scale, nextLocalTransform.scale, 0.5f);

				resultTransform = gem::translate(gem::mat4{ 1.f }, position) * gem::mat4_cast(rotation) * gem::scale(gem::mat4{ 1.f }, scale);
			}

			gem::mat4 resultT = parentTransform * resultTransform;
			result[i] = resultT;
		}

		for (size_t i = 0; i < result.size(); i++)
		{
			result[i] = result[i] * invBindPoses[i];
		}

		return result;
	}

	const std::vector<Animation::TRS> Animation::SampleTRS(float aStartTime, Ref<Skeleton> aSkeleton, bool looping) const
	{
		VT_PROFILE_FUNCTION();

		const float localTime = AnimationManager::globalClock - aStartTime;
		const float normalizedTime = localTime / myDuration;

		const int32_t frameCount = (int32_t)myFrames.size();
		const int32_t currentFrameIndex = frameCount - std::abs((int32_t)(std::floor(normalizedTime * (float)frameCount)) % (2 * frameCount) - frameCount);

		int32_t nextFrameIndex = currentFrameIndex + 1;

		if (nextFrameIndex >= frameCount)
		{
			if (looping)
			{
				nextFrameIndex = 0;
			}
			else
			{
				nextFrameIndex = currentFrameIndex;
			}
		}

		std::vector<TRS> result;
		result.resize(aSkeleton->GetJointCount(), TRS{});

		const float animDelta = 1.f / (float)myFramesPerSecond;
		const float frameTime = localTime / animDelta;
		const float deltaTime = frameTime - (float)currentFrameIndex;

		const Frame& currentFrame = myFrames.at(currentFrameIndex);
		const Frame& nextFrame = myFrames.at(nextFrameIndex);

		const auto& joints = aSkeleton->GetJoints();
		const auto& invBindPoses = aSkeleton->GetInverseBindPoses();

		for (size_t i = 0; i < joints.size(); i++)
		{
			const auto& joint = joints[i];

			const auto& currentLocalTransform = currentFrame.localTRS[i];
			const auto& nextLocalTransform = nextFrame.localTRS[i];

			// Blend and set
			const gem::vec3 position = gem::lerp(currentLocalTransform.position, nextLocalTransform.position, 0.5f);
			const gem::quat rotation = gem::slerp(gem::normalize(currentLocalTransform.rotation), gem::normalize(nextLocalTransform.rotation), 0.5f);
			const gem::vec3 scale = gem::lerp(currentLocalTransform.scale, nextLocalTransform.scale, 0.5f);
		
			result[i].position = position;
			result[i].rotation = rotation;
			result[i].scale = scale;
		}

		return result;
	}

	const std::vector<gem::mat4> Animation::SampleCrossfaded(float lerpT, float aNormalizedTime, float aOtherNormalizedTime, Ref<Skeleton> aSkeleton, Ref<Animation> otherAnimation)
	{
		const FrameAnimData thisFrameData = GetFrameDataFromAnimation(*this, aNormalizedTime);
		const FrameAnimData otherFrameData = GetFrameDataFromAnimation(*otherAnimation, aOtherNormalizedTime);

		const Frame& thisCurrentFrame = myFrames.at(thisFrameData.currentFrameIndex);
		const Frame& thisNextFrame = myFrames.at(thisFrameData.nextFrameIndex);

		const Frame& otherCurrentFrame = otherAnimation->myFrames.at(otherFrameData.currentFrameIndex);
		const Frame& otherNextFrame = otherAnimation->myFrames.at(otherFrameData.nextFrameIndex);

		const auto& joints = aSkeleton->GetJoints();
		const auto& invBindPoses = aSkeleton->GetInverseBindPoses();

		std::vector<gem::mat4> result;
		result.resize(aSkeleton->GetJointCount(), gem::mat4(1.f));

		for (size_t i = 0; i < joints.size(); i++)
		{
			const auto& joint = joints[i];
			gem::mat4 parentTransform = { 1.f };

			if (joint.parentIndex >= 0)
			{
				parentTransform = result.at(joint.parentIndex);
			}

			const gem::mat4 thisBlendResult = BlendFrames(thisCurrentFrame, thisNextFrame, parentTransform, i, thisFrameData.deltaTime);
			const gem::mat4 otherBlendResult = BlendFrames(otherCurrentFrame, otherNextFrame, parentTransform, i, otherFrameData.deltaTime);

			const gem::mat4 resultTransform = Math::Lerp(thisBlendResult, otherBlendResult, lerpT);
			result[i] = resultTransform;
		}

		for (size_t i = 0; i < result.size(); i++)
		{
			result[i] = result[i] * invBindPoses[i];
		}

		return result;
	}

	const Animation::FrameAnimData Animation::GetFrameDataFromAnimation(Animation& animation, const float aNormalizedTime)
	{
		FrameAnimData animData{};

		const size_t frameCount = animation.myFrames.size();

		animData.currentFrameIndex = (size_t)std::floorf((float)frameCount * aNormalizedTime);
		animData.nextFrameIndex = animData.currentFrameIndex + 1;

		if (animData.nextFrameIndex >= frameCount)
		{
			animData.nextFrameIndex = 1;
		}

		if (animData.currentFrameIndex == frameCount)
		{
			animData.currentFrameIndex = frameCount - 1;
		}

		const float localTime = animation.myDuration * aNormalizedTime;
		const float animDelta = 1.f / (float)animation.myFramesPerSecond;
		const float frameTime = localTime / animDelta;

		animData.deltaTime = frameTime - (float)animData.currentFrameIndex;

		return animData;
	}

	const gem::mat4 Animation::BlendFrames(const Frame& currentFrame, const Frame& nextFrame, const gem::mat4& parentTransform, const size_t jointIndex, const float blendFactor)
	{
		//const gem::mat4 currentLocalTransform = currentFrame.localTransforms.at(jointIndex);
		//const gem::mat4 nextLocalTransform = nextFrame.localTransforms.at(jointIndex);

		//const gem::mat4 currentGlobalTransform = parentTransform * gem::transpose(currentLocalTransform);
		//const gem::mat4 nextGlobalTransform = parentTransform * gem::transpose(nextLocalTransform);

		//return Math::Lerp(currentGlobalTransform, nextGlobalTransform, blendFactor);

		return gem::mat4{ 1.f };
	}
}

