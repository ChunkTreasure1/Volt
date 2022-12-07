#include "vtpch.h"
#include "AnimationState.h"

#include "Volt/Animation/AnimationManager.h"
#include "Volt/Asset/Animation/AnimatedCharacter.h"

namespace Volt
{
	AnimationState::AnimationState(const std::string& name, uint32_t animationIndex, Ref<AnimatedCharacter> character)
		: myName(name), myAnimationIndex(animationIndex), myCharacter(character)
	{}

	void AnimationState::OnEnter(float startTime)
	{
		myCurrentStartTime = startTime;
	}

	void AnimationState::OnExit()
	{}

	bool AnimationState::Update(Result& outTargetState)
	{
		for (const auto& transition : myTransitions)
		{
			if (outTargetState.nextState = transition.func(); outTargetState.nextState != -1)
			{
				outTargetState.shouldBlend = transition.blendToNext;
				outTargetState.blendSpeed = transition.blendSpeed;
				return true;
			}
		}

		return false;
	}

	void AnimationState::AddTransition(std::function<int32_t()>&& func, bool blendTo, float blendSpeed)
	{
		auto& transition = myTransitions.emplace_back();
		transition.func = func;
		transition.blendToNext = blendTo;
		transition.blendSpeed = blendSpeed;
	}
}