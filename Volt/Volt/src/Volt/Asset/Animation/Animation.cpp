#include "vtpch.h"
#include "Animation.h"

#include "Volt/Asset/Animation/Skeleton.h"
#include "Volt/Animation/AnimationManager.h"

namespace Volt
{
	const std::vector<glm::mat4> Animation::Sample(float aStartTime, Ref<Skeleton> aSkeleton, bool looping)
	{
		VT_PROFILE_FUNCTION();

		const float localTime = AnimationManager::globalClock - aStartTime;
		const float normalizedTime = localTime / myDuration;

		const int32_t frameCount = (int32_t)myFrames.size();
		int32_t currentFrameIndex = frameCount - std::abs((int32_t)(std::floor(normalizedTime * (float)frameCount)) % (2 * frameCount) - frameCount);

		currentFrameIndex = std::clamp(currentFrameIndex, 0, frameCount - 1);

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

		std::vector<glm::mat4> result;
		result.resize(aSkeleton->GetJointCount(), glm::mat4(1.f));

		if (result.empty())
		{
			return {};
		}

		const float animDelta = 1.f / (float)myFramesPerSecond;
		const float frameTime = localTime / animDelta;
		const float deltaTime = frameTime - (float)currentFrameIndex;

		const Pose& currentFrame = myFrames.at(currentFrameIndex);
		const Pose& nextFrame = myFrames.at(nextFrameIndex);

		const auto& joints = aSkeleton->GetJoints();
		const auto& invBindPoses = aSkeleton->GetInverseBindPose();

		for (size_t i = 0; i < joints.size(); i++)
		{
			const auto& joint = joints[i];

			glm::mat4 parentTransform = { 1.f };

			if (joint.parentIndex >= 0)
			{
				parentTransform = result[joint.parentIndex];
			}

			const auto& currentLocalTransform = currentFrame.localTRS[i];
			const auto& nextLocalTransform = nextFrame.localTRS[i];

			glm::mat4 resultTransform;

			// Blend
			{
				const glm::vec3 position = glm::mix(currentLocalTransform.position, nextLocalTransform.position, deltaTime);
				const glm::quat rotation = glm::slerp(glm::normalize(currentLocalTransform.rotation), glm::normalize(nextLocalTransform.rotation), deltaTime);
				const glm::vec3 scale = glm::mix(currentLocalTransform.scale, nextLocalTransform.scale, deltaTime);

				resultTransform = glm::translate(glm::mat4{ 1.f }, position)* glm::mat4_cast(rotation)* glm::scale(glm::mat4{ 1.f }, scale);
			}

			glm::mat4 resultT = parentTransform * resultTransform;
			result[i] = resultT;
		}

		for (size_t i = 0; i < result.size(); i++)
		{
			result[i] = result[i] * invBindPoses[i];
		}

		return result;
	}

	const std::vector<glm::mat4> Animation::Sample(uint32_t frameIndex, Ref<Skeleton> aSkeleton)
	{
		VT_PROFILE_FUNCTION();

		std::vector<glm::mat4> result;
		result.resize(aSkeleton->GetJointCount(), glm::mat4(1.f));

		if (result.empty())
		{
			return {};
		}

		const Pose& currentFrame = myFrames.at(frameIndex);

		const auto& joints = aSkeleton->GetJoints();
		const auto& invBindPoses = aSkeleton->GetInverseBindPose();

		for (size_t i = 0; i < joints.size(); i++)
		{
			const auto& joint = joints[i];

			glm::mat4 parentTransform = { 1.f };

			if (joint.parentIndex >= 0)
			{
				parentTransform = result[joint.parentIndex];
			}

			const auto& currentLocalTransform = currentFrame.localTRS[i];

			glm::mat4 resultTransform;

			const glm::vec3 position = currentLocalTransform.position;
			const glm::quat rotation = glm::normalize(currentLocalTransform.rotation);
			const glm::vec3 scale = currentLocalTransform.scale;

			resultTransform = glm::translate(glm::mat4{ 1.f }, position)* glm::mat4_cast(rotation)* glm::scale(glm::mat4{ 1.f }, scale);

			glm::mat4 resultT = parentTransform * resultTransform;
			result[i] = resultT;
		}

		for (size_t i = 0; i < result.size(); i++)
		{
			result[i] = result[i] * invBindPoses[i];
		}

		return result;
	}

	const std::vector<Animation::TRS> Animation::SampleTRS(float aStartTime, Ref<Skeleton> aSkeleton, bool looping, float speed) const
	{
		VT_PROFILE_FUNCTION();

		const float finalDuration = myDuration / speed;

		const float localTime = AnimationManager::globalClock - aStartTime;
		const float normalizedTime = localTime / finalDuration;

		const int32_t frameCount = (int32_t)myFrames.size();
		int32_t currentFrameIndex = (int32_t)(std::floor(normalizedTime * (float)frameCount)) % frameCount;

		if (normalizedTime > 1.f && !looping)
		{
			currentFrameIndex = frameCount - 1;
		}

		currentFrameIndex = std::clamp(currentFrameIndex, 0, frameCount - 1);

		const float blendValue = (fmodf(normalizedTime, 1.f) * ((float)frameCount)) - (float)currentFrameIndex;		

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

		const Pose& currentFrame = myFrames.at(currentFrameIndex);
		const Pose& nextFrame = myFrames.at(nextFrameIndex);

		const auto& joints = aSkeleton->GetJoints();
		
		if (currentFrame.localTRS.size() < joints.size())
		{
			return result;
		}

		for (size_t i = 0; i < joints.size(); i++)
		{
			const auto& currentLocalTransform = currentFrame.localTRS[i];
			const auto& nextLocalTransform = nextFrame.localTRS[i];

			// Blend
			result[i].position = glm::mix(currentLocalTransform.position, nextLocalTransform.position, blendValue);
			result[i].rotation = glm::slerp(glm::normalize(currentLocalTransform.rotation), glm::normalize(nextLocalTransform.rotation), blendValue);
			result[i].scale = glm::mix(currentLocalTransform.scale, nextLocalTransform.scale, blendValue);
		}

		return result;
	}

	const bool Animation::IsAtEnd(float startTime, float speed)
	{
		const float localTime = AnimationManager::globalClock - startTime;
		const float normalizedTime = localTime / (myDuration / speed);

		const int32_t frameCount = (int32_t)myFrames.size();
		const int32_t currentFrameIndex = (int32_t)(std::floor(normalizedTime * (float)frameCount)) % frameCount;

		int32_t nextFrameIndex = currentFrameIndex + 1;

		if (nextFrameIndex >= frameCount)
		{
			return true;
		}

		return false;
	}

	const bool Animation::HasPassedTime(float startTime, float speed, float time)
	{
		const float localTime = AnimationManager::globalClock - startTime;
		const float normalizedTime = localTime / (myDuration / speed);

		const float normalizedWantedTime = time / (myDuration / speed);

		const int32_t frameCount = (int32_t)myFrames.size();
		const int32_t currentFrameIndex = (int32_t)(std::floor(normalizedTime * (float)frameCount)) % frameCount;

		const int32_t wantedFrame = (int32_t)(std::floor(normalizedWantedTime * (float)frameCount)) % frameCount;

		int32_t nextFrameIndex = currentFrameIndex + 1;

		if (nextFrameIndex >= wantedFrame)
		{
			return true;
		}

		return false;
	}

	const uint32_t Animation::GetFrameFromStartTime(float startTime, float speed)
	{
		const float localTime = AnimationManager::globalClock - startTime;
		const float normalizedTime = localTime / (myDuration / speed);

		const int32_t frameCount = (int32_t)myFrames.size();
		const int32_t currentFrameIndex = (int32_t)(std::floor(normalizedTime * (float)frameCount)) % frameCount;
		return currentFrameIndex;
	}

	const float Animation::GetNormalizedCurrentTimeFromStartTime(float startTime, float speed, bool looping)
	{
		const float localTime = AnimationManager::globalClock - startTime;
		const float normalizedTime = localTime / (myDuration / speed);
		
		if (looping)
		{
			return fmodf(normalizedTime, 1.f);
		}
		else
		{
			return std::clamp(normalizedTime, 0.f, 1.f);
		}
		//return 0.0f;
	}

	const Animation::PoseData Animation::GetFrameDataFromAnimation(Animation& animation, const float aNormalizedTime)
	{
		PoseData animData{};

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
}

