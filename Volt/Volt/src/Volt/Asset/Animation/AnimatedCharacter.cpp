#include "vtpch.h"
#include "AnimatedCharacter.h"

#include "Volt/Animation/AnimationManager.h"
#include "Volt/Asset/Animation/Animation.h"

namespace Volt
{
	const Vector<glm::mat4> AnimatedCharacter::SampleAnimation(uint32_t index, float aStartTime, bool looping) const
	{
		if (myAnimations.find(index) == myAnimations.end())
		{
			return {};
		}

		return myAnimations.at(index)->SampleStartTime(aStartTime, mySkeleton, looping);
	}

	const Vector<glm::mat4> AnimatedCharacter::SampleAnimation(uint32_t index, uint32_t frameIndex) const
	{
		if (myAnimations.find(index) == myAnimations.end())
		{
			return {};
		}

		return myAnimations.at(index)->Sample(frameIndex, mySkeleton);
	}

	const float AnimatedCharacter::GetAnimationDuration(uint32_t index) const
	{
		if (myAnimations.find(index) == myAnimations.end())
		{
			return 0.f;
		}

		if (!myAnimations.at(index))
		{
			return 0.f;
		}

		return myAnimations.at(index)->GetDuration();
	}

	const AnimatedCharacter::JointAttachment AnimatedCharacter::GetJointAttachmentFromName(const std::string& jntName) const
	{
		for (const auto& jnt : myJointAttachments)
		{
			if (jnt.name == jntName)
			{
				return jnt;
			}
		}

		return {};
	}

	const AnimatedCharacter::JointAttachment AnimatedCharacter::GetJointAttachmentFromID(const UUID64& id) const
	{
		for (const auto& jnt : myJointAttachments)
		{
			if (jnt.id == id)
			{
				return jnt;
			}
		}

		return {};
	}

	const bool AnimatedCharacter::HasJointAttachment(const std::string& attachmentName) const
	{
		for (const auto& jnt : myJointAttachments)
		{
			if (jnt.name == attachmentName)
			{
				return true;
			}
		}
		return false;
	}

	void AnimatedCharacter::RemoveAnimation(uint32_t index)
	{
		if (!myAnimations.contains(index))
		{
			return;
		}

		if (myAnimationEvents.contains(index))
		{
			myAnimationEvents.erase(index);
		}

		myAnimations.erase(index);
	}

	void AnimatedCharacter::RemoveAnimationEvent(const std::string& eventName, uint32_t frame, uint32_t animationIndex)
	{
		if (!myAnimations.contains(animationIndex))
		{
			VT_LOG(LogVerbosity::Error, "Trying to remove animation event from invalid animation index!");
			return;
		}

		myAnimationEvents[animationIndex].erase(std::remove_if(myAnimationEvents[animationIndex].begin(), myAnimationEvents[animationIndex].end(), [&eventName, &frame](const auto& lhs)
		{
			return lhs.name == eventName && lhs.frame == frame;
		}));
	}

	void AnimatedCharacter::AddAnimationEvent(const std::string& eventName, uint32_t frame, uint32_t animationIndex)
	{
		if (!myAnimations.contains(animationIndex))
		{
			VT_LOG(LogVerbosity::Error, "Trying to add animation event to invalid animation index!");
			return;
		}

		myAnimationEvents[animationIndex].emplace_back(frame, eventName);
	}

	const int32_t AnimatedCharacter::GetAnimationIndexFromHandle(Volt::AssetHandle animationHandle)
	{
		for (const auto& [index, animation] : myAnimations)
		{
			if (!animation)
			{
				continue;
			}

			if (animation->handle == animationHandle)
			{
				return index;
			}
		}

		return -1;
	}
}
