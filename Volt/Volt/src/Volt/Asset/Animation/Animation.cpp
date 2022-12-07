#include "vtpch.h"
#include "Animation.h"

#include "Volt/Asset/Animation/Skeleton.h"
#include "Volt/Animation/AnimationManager.h"
#include "Volt/Core/Profiling.h"

#include "Volt/Utility/Math.h"

namespace Volt
{
	const std::vector<gem::mat4> Animation::Sample(float aStartTime, Ref<Skeleton> aSkeleton, bool looping)
	{
		VT_PROFILE_FUNCTION();

		float localTime = 0.f;

		if (!looping)
		{
			localTime = std::clamp(AnimationManager::globalClock - aStartTime, 0.f, myDuration);
		}
		else
		{
			localTime = fmodf(AnimationManager::globalClock - aStartTime, myDuration);
		}

		std::vector<gem::mat4> result;
		result.resize(aSkeleton->GetJointCount(), gem::mat4(1.f));

		const float normalizedTime = localTime / myDuration;

		const size_t frameCount = myFrames.size();
		size_t currentFrameIndex = (size_t)std::floorf((float)frameCount * normalizedTime);
		size_t nextFrameIndex = currentFrameIndex + 1;

		if (currentFrameIndex >= frameCount)
		{
			currentFrameIndex = frameCount - 1;
		}

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

			gem::mat4 parentTransform = { 1.f };

			if (joint.parentIndex >= 0)
			{
				parentTransform = result[joint.parentIndex];
			}

			const gem::mat4 currentLocalTransform = currentFrame.localTransforms[i];
			const gem::mat4 nextLocalTransform = nextFrame.localTransforms[i];

			const gem::mat4 currentGlobalTransform = parentTransform * gem::transpose(currentLocalTransform);
			const gem::mat4 nextGlobalTransform = parentTransform * gem::transpose(nextLocalTransform);

			gem::mat4 resultTransform;

			// Blend
			{
				resultTransform = Math::Lerp(currentGlobalTransform, nextGlobalTransform, deltaTime);
			}

			gem::mat4 resultT = resultTransform;
			result[i] = resultT;
		}

		for (size_t i = 0; i < result.size(); i++)
		{
			result[i] = result[i] * invBindPoses[i];
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
		const gem::mat4 currentLocalTransform = currentFrame.localTransforms.at(jointIndex);
		const gem::mat4 nextLocalTransform = nextFrame.localTransforms.at(jointIndex);

		const gem::mat4 currentGlobalTransform = parentTransform * gem::transpose(currentLocalTransform);
		const gem::mat4 nextGlobalTransform = parentTransform * gem::transpose(nextLocalTransform);

		return Math::Lerp(currentGlobalTransform, nextGlobalTransform, blendFactor);
	}
}

