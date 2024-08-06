#include "vtpch.h"
#include "Animation.h"

#include "Volt/Asset/Animation/Skeleton.h"
#include "Volt/Animation/AnimationManager.h"

namespace Volt
{
	const Vector<glm::mat4> Animation::SampleStartTime(float aStartTime, Ref<Skeleton> aSkeleton, bool looping)
	{
		VT_PROFILE_FUNCTION();

		const float localTime = AnimationManager::globalClock - aStartTime;
		const float normalizedTime = localTime / m_duration;

		const int32_t frameCount = (int32_t)m_frames.size();
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

		Vector<glm::mat4> result;
		result.resize(aSkeleton->GetJointCount(), glm::mat4(1.f));

		if (result.empty())
		{
			return {};
		}

		const float animDelta = 1.f / (float)m_framesPerSecond;
		const float frameTime = localTime / animDelta;
		const float deltaTime = frameTime - (float)currentFrameIndex;

		const Pose& currentFrame = m_frames.at(currentFrameIndex);
		const Pose& nextFrame = m_frames.at(nextFrameIndex);

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

				resultTransform = glm::translate(glm::mat4{ 1.f }, position) * glm::mat4_cast(rotation) * glm::scale(glm::mat4{ 1.f }, scale);
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

	const Vector<glm::mat4> Animation::Sample(float samplePercent, Ref<Skeleton> skeleton, bool looping)
	{
		VT_PROFILE_FUNCTION();

		if (!skeleton)
		{
			VT_LOG(LogVerbosity::Error, "Tried to sample animation with no skeleton");
			return {};
		}
		if (samplePercent < 0.f)
		{
			VT_LOG(LogVerbosity::Error, "Sample percent has to be a positive value");
			return {};
		}
		if (samplePercent > 1.f)
		{
			VT_LOG(LogVerbosity::Error, "Sample percent has to be a value between 0 and 1");
			return {};
		}

		const uint32_t frameCount = static_cast<uint32_t>(m_frames.size());

		if (frameCount == 0)
		{
			VT_LOG(LogVerbosity::Error, "Tried to sample animation with no frames");
			return {};
		}

		const uint32_t frameIndexBeforeTime = static_cast<uint32_t>(std::floor(samplePercent * static_cast<float>(frameCount)));
		//if we are looping, modulus the frame index to the frame count, otherwise clamp it
		const uint32_t frameIndexAfterTime = looping ? ((frameIndexBeforeTime + 1) % (frameCount)) : std::clamp(frameIndexBeforeTime + 1, 0u, frameCount - 1);
		const float percentageBetweenFrames = (samplePercent * frameCount) - frameIndexBeforeTime;

		Vector<glm::mat4> result;
		result.resize(skeleton->GetJointCount(), glm::mat4(1.f));

		if (result.empty())
		{
			VT_LOG(LogVerbosity::Error, "Tried to sample using a skeleton with no joints. AssetHandle: {0}", skeleton->handle);
			return {};
		}

		//B
		const Pose& currentFrame = m_frames.at(frameIndexBeforeTime);
		const Pose& nextFrame = m_frames.at(frameIndexAfterTime);

		const Pose blendedPose = GetBlendedPose(currentFrame, nextFrame, percentageBetweenFrames);
		//B

		const Vector<Skeleton::Joint>& joints = skeleton->GetJoints();
		const Vector<glm::mat4>& invBindPose = skeleton->GetInverseBindPose();

		for (size_t i = 0; i < joints.size(); i++)
		{
			const Skeleton::Joint& joint = joints[i];

			glm::mat4 parentTransform = { 1.f };

			if (joint.parentIndex >= 0)
			{
				parentTransform = result[joint.parentIndex];
			}

			const Animation::TRS& localTransform = blendedPose.localTRS[i];

			glm::mat4 resultTransform = glm::translate(glm::mat4{ 1.f }, localTransform.position) * glm::mat4_cast(localTransform.rotation) * glm::scale(glm::mat4{ 1.f }, localTransform.scale);

			glm::mat4 resultT = parentTransform * resultTransform;
			result[i] = resultT;
		}

		for (size_t i = 0; i < result.size(); i++)
		{
			result[i] = result[i] * invBindPose[i];
		}
		
		return result;
	}

	const Vector<glm::mat4> Animation::Sample(uint32_t frameIndex, Ref<Skeleton> aSkeleton)
	{
		VT_PROFILE_FUNCTION();

		Vector<glm::mat4> result;
		result.resize(aSkeleton->GetJointCount(), glm::mat4(1.f));

		if (result.empty())
		{
			return {};
		}

		const Pose& currentFrame = m_frames.at(frameIndex);

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

			resultTransform = glm::translate(glm::mat4{ 1.f }, position) * glm::mat4_cast(rotation) * glm::scale(glm::mat4{ 1.f }, scale);

			glm::mat4 resultT = parentTransform * resultTransform;
			result[i] = resultT;
		}

		for (size_t i = 0; i < result.size(); i++)
		{
			result[i] = result[i] * invBindPoses[i];
		}

		return result;
	}

	Vector<glm::mat4> Animation::LocalPoseToGlobalMatrices(const Pose& localPose, Ref<Skeleton> aSkeleton)
	{
		return Vector<glm::mat4>();
	}

	void Animation::BlendPoseWith(Pose& target, const Pose& poseToBlendWith, float blendFactor)
	{
		VT_PROFILE_FUNCTION();

		if (target.localTRS.size() != poseToBlendWith.localTRS.size())
		{
			VT_LOG(LogVerbosity::Error, "Cannot blend poses with different joint counts");
			return;
		}

		for (size_t i = 0; i < target.localTRS.size(); i++)
		{
			const auto& currentLocalTransform = target.localTRS[i];
			const auto& nextLocalTransform = poseToBlendWith.localTRS[i];

			target.localTRS[i].position = glm::mix(currentLocalTransform.position, nextLocalTransform.position, blendFactor);
			target.localTRS[i].rotation = glm::slerp(currentLocalTransform.rotation, nextLocalTransform.rotation, blendFactor);
			target.localTRS[i].scale = glm::mix(currentLocalTransform.scale, nextLocalTransform.scale, blendFactor);
		}
	}

	Animation::Pose Animation::GetBlendedPose(const Pose& target, const Pose& poseToBlendWith, float blendFactor)
	{
		VT_PROFILE_FUNCTION();

		if (target.localTRS.size() != poseToBlendWith.localTRS.size())
		{
			VT_LOG(LogVerbosity::Error, "Cannot blend poses with different joint counts");
			return target;
		}

		Pose result;
		result.localTRS.resize(target.localTRS.size());
		for (size_t i = 0; i < target.localTRS.size(); i++)
		{
			const auto& currentLocalTransform = target.localTRS[i];
			const auto& nextLocalTransform = poseToBlendWith.localTRS[i];

			result.localTRS[i].position = glm::mix(currentLocalTransform.position, nextLocalTransform.position, blendFactor);
			result.localTRS[i].rotation = glm::slerp(currentLocalTransform.rotation, nextLocalTransform.rotation, blendFactor);
			result.localTRS[i].scale = glm::mix(currentLocalTransform.scale, nextLocalTransform.scale, blendFactor);
		}
		
		return result;
	}

	const Vector<Animation::TRS> Animation::SampleTRS(float aStartTime, Ref<Skeleton> aSkeleton, bool looping, float speed) const
	{
		VT_PROFILE_FUNCTION();

		const float finalDuration = m_duration / speed;

		const float localTime = AnimationManager::globalClock - aStartTime;
		const float normalizedTime = localTime / finalDuration;

		const int32_t frameCount = (int32_t)m_frames.size();
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

		Vector<TRS> result;
		result.resize(aSkeleton->GetJointCount(), TRS{});

		const Pose& currentFrame = m_frames.at(currentFrameIndex);
		const Pose& nextFrame = m_frames.at(nextFrameIndex);

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
		const float normalizedTime = localTime / (m_duration / speed);

		const int32_t frameCount = (int32_t)m_frames.size();
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
		const float normalizedTime = localTime / (m_duration / speed);

		const float normalizedWantedTime = time / (m_duration / speed);

		const int32_t frameCount = (int32_t)m_frames.size();
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
		const float normalizedTime = localTime / (m_duration / speed);

		const int32_t frameCount = (int32_t)m_frames.size();
		const int32_t currentFrameIndex = (int32_t)(std::floor(normalizedTime * (float)frameCount)) % frameCount;
		return currentFrameIndex;
	}

	const float Animation::GetNormalizedCurrentTimeFromStartTime(float startTime, float speed, bool looping)
	{
		const float localTime = AnimationManager::globalClock - startTime;
		const float normalizedTime = localTime / (m_duration / speed);

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

	void Animation::AddEvent(const std::string& eventName, uint32_t frame)
	{
		m_events.emplace_back(frame, eventName);
	}

	void Animation::RemoveEvent(const std::string& eventName, uint32_t frame)
	{
		auto it = std::find_if(m_events.begin(), m_events.end(), [eventName](const auto& event)
		{
			return event.name == eventName;
		});

		if (it != m_events.end())
		{
			m_events.erase(it);
		}
	}

	const Animation::PoseData Animation::GetFrameDataFromAnimation(Animation& animation, const float aNormalizedTime)
	{
		PoseData animData{};

		const size_t frameCount = animation.m_frames.size();

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

		const float localTime = animation.m_duration * aNormalizedTime;
		const float animDelta = 1.f / (float)animation.m_framesPerSecond;
		const float frameTime = localTime / animDelta;

		animData.deltaTime = frameTime - (float)animData.currentFrameIndex;

		return animData;
	}
}

