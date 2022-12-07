#include "vtpch.h"
#include "AnimatedCharacter.h"

#include "Volt/Animation/AnimationManager.h"
#include "Volt/Asset/Animation/Animation.h"

namespace Volt
{
	const std::vector<gem::mat4> AnimatedCharacter::SampleAnimation(uint32_t index, float aStartTime, bool looping) const
	{
		if (myAnimations.find(index) == myAnimations.end())
		{
			VT_CORE_WARN("Trying to access animation at invalid index {0}!", index);
			return {};
		}

		return myAnimations.at(index)->Sample(aStartTime, mySkeleton, looping);
	}

	const std::vector<gem::mat4> AnimatedCharacter::SampleCrossfadingAnimation(uint32_t crossfadeFrom, uint32_t crossfadeTo, float fromStartTime, float toStartTime, float speed)
	{
		if (myAnimations.find(crossfadeFrom) == myAnimations.end() || myAnimations.find(crossfadeTo) == myAnimations.end())
		{
			VT_CORE_WARN("Trying to crossfade between invalid animations {0} and {1}!", crossfadeFrom, crossfadeTo);
			return {};
		}

		const float totalCrossfadeTime = GetCrossfadeTimeFromAnimations(crossfadeFrom, crossfadeTo, speed);
		
		const float fromDuration = myAnimations.at(crossfadeFrom)->GetDuration();
		const float toDuration = myAnimations.at(crossfadeTo)->GetDuration();
		
		const float currentTime = gem::clamp(AnimationManager::globalClock - toStartTime, 0.f, totalCrossfadeTime);
		const float currentFromTime = fmodf(AnimationManager::globalClock - fromStartTime, fromDuration);
		const float currentToTime = fmodf(AnimationManager::globalClock - toStartTime, toDuration);

		const float normalizedTime = currentFromTime / fromDuration;
		const float otherNormalizedTime = currentToTime / toDuration;

		const float t = currentTime / totalCrossfadeTime;

		return myAnimations.at(crossfadeFrom)->SampleCrossfaded(t, normalizedTime, otherNormalizedTime, mySkeleton, myAnimations.at(crossfadeTo));
	}


	const float AnimatedCharacter::GetAnimationDuration(uint32_t index) const
	{
		if (myAnimations.find(index) == myAnimations.end())
		{
			VT_CORE_WARN("Trying to access animation at invalid index {0}!", index);
			return 0.f;
		}

		return myAnimations.at(index)->GetDuration();
	}

	void AnimatedCharacter::RemoveAnimation(uint32_t index)
	{
		if (myAnimations.find(index) == myAnimations.end())
		{
			return;
		}

		myAnimations.erase(index);
	}

	const float AnimatedCharacter::GetCrossfadeTimeFromAnimations(uint32_t crossfadeFrom, uint32_t crossfadeTo, float crossfadeSpeed)
	{
		if (myAnimations.find(crossfadeFrom) == myAnimations.end() || myAnimations.find(crossfadeTo) == myAnimations.end())
		{
			VT_CORE_WARN("Trying to get crossfade time between invalid animations {0} and {1}!", crossfadeFrom, crossfadeTo);
			return 0.f;
		}

		return gem::max(myAnimations.at(crossfadeFrom)->GetDuration(), myAnimations.at(crossfadeTo)->GetDuration()) / crossfadeSpeed;
	}
}