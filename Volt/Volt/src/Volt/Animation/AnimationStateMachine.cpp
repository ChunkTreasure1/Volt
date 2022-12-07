#include "vtpch.h"
#include "AnimationStateMachine.h"

#include "Volt/Asset/Animation/AnimatedCharacter.h"
#include "Volt/Animation/AnimationState.h"
#include "Volt/Animation/AnimationManager.h"

namespace Volt
{
	AnimationStateMachine::AnimationStateMachine(Ref<AnimatedCharacter> character)
		: myCharacter(character)
	{}

	void AnimationStateMachine::SetStartSate(uint32_t index)
	{
		if ((uint32_t)myStates.size() <= index)
		{
			VT_CORE_ERROR("State with index {0} is not valid!", index);
			return;
		}

		myCurrentState = myStates.at(index).get();
		myCurrentState->OnEnter(AnimationManager::globalClock);
	}

	void AnimationStateMachine::SetCurrentState(uint32_t index)
	{
		if ((uint32_t)myStates.size() <= index)
		{
			VT_CORE_ERROR("State with index {0} is not valid!", index);
			return;
		}

		if (myCurrentState)
		{
			myCurrentState->OnExit();
		}
		myCurrentState = myStates.at(index).get();
		myCurrentState->OnEnter(AnimationManager::globalClock);
	}

	Ref<AnimationState> AnimationStateMachine::AddState(const std::string& name, uint32_t animationIndex)
	{
		return myStates.emplace_back(CreateRef<AnimationState>(name, animationIndex, myCharacter));
	}

	void AnimationStateMachine::Update()
	{
		if (!myCurrentState)
		{
			return;
		}

		if (myIsCrossfading)
		{
			if (myCrossfadeToStartTime + myCrossfadeTime < AnimationManager::globalClock)
			{
				myIsCrossfading = false;
				myCurrentState->OnEnter(myCrossfadeToStartTime);
			}
			else
			{
				return;
			}
		}

		AnimationState::Result result{};
		if (myCurrentState->Update(result))
		{
			if (result.nextState != -1)
			{
				if (result.nextState >= (int32_t)myStates.size())
				{
					return;
				}

				if (result.shouldBlend)
				{
					myIsCrossfading = true;
					myCrossfadeFromStartTime = myCurrentState->GetCurrentStartTime();
					myCrossfadeToStartTime = AnimationManager::globalClock;
					myCrossfadeTime = myCharacter->GetCrossfadeTimeFromAnimations(myCurrentState->GetAnimationIndex(), myStates.at(result.nextState)->GetAnimationIndex(), result.blendSpeed);
					myCrossfadeFrom = myCurrentState->GetAnimationIndex();
					myCrossfadeSpeed = result.blendSpeed;
					myCurrentState = myStates.at(result.nextState).get();
				}
				else
				{
					myCurrentState->OnExit();
					myCurrentState = myStates.at(result.nextState).get();
					myCurrentState->OnEnter(AnimationManager::globalClock);
				}

			}
		}
	}

	const std::vector<gem::mat4> AnimationStateMachine::Sample()
	{
		if (myCurrentState)
		{
			if (myIsCrossfading)
			{
				return myCharacter->SampleCrossfadingAnimation(myCrossfadeFrom, myCurrentState->GetAnimationIndex(), myCrossfadeFromStartTime, myCrossfadeToStartTime, myCrossfadeSpeed);
			}
		
			return myCharacter->SampleAnimation(myCurrentState->GetAnimationIndex(), myCurrentState->GetCurrentStartTime(), myCurrentState->ShouldLoop());
		}

		return {};
	}
}