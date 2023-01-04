#include "vtpch.h"
#include "AnimationStateMachine.h"

namespace Volt
{
	void AnimationStateMachine::Update(float deltaTime)
	{
		if (myCurrentState != -1 || myCurrentState >= (int32_t)myStates.size())
		{
			return;
		}

		const auto& currState = myStates.at(myCurrentState);

		for (const auto& id : currState->transitions)
		{
			const auto transition = GetTransitionFromId(id);
			if (!transition)
			{
				continue;
			}

			if (transition->fromState != currState->id)
			{
				continue; // This transition is going to this state
			}

			if (ShouldTransition(id))
			{
				myCurrentState = GetStateIndexFromId(transition->toState);
				break;
			}
		}
	}

	const std::vector<Animation::TRS> AnimationStateMachine::Sample(float startTime, Ref<Skeleton> skeleton) const
	{
		if (myCurrentState == -1 || myCurrentState >= (int32_t)myStates.size())
		{
			return {};
		}

		const auto& currState = myStates.at(myCurrentState);
		if (!currState->animation || !currState->animation->IsValid())
		{
			return {};
		}

		return currState->animation->SampleTRS(startTime, skeleton, currState->isLooping);
	}

	void AnimationStateMachine::SetStartState(const UUID stateId)
	{
		auto it = std::find_if(myStates.begin(), myStates.end(), [stateId](const auto& lhs) { return lhs->id == stateId; });
		if (it != myStates.end())
		{
			const ptrdiff_t index = std::distance(myStates.begin(), it);
			myStartState = (int32_t)index;
		}
	}

	AnimationState* AnimationStateMachine::GetStateFromId(const UUID stateId) const
	{
		auto it = std::find_if(myStates.begin(), myStates.end(), [stateId](const auto& lhs) { return lhs->id == stateId; });
		if (it != myStates.end())
		{
			return (*it).get();
		}

		return nullptr;
	}

	AnimationTransition* AnimationStateMachine::GetTransitionFromId(const UUID transitionId) const
	{
		auto it = std::find_if(myTransitions.begin(), myTransitions.end(), [transitionId](const auto& lhs) { return lhs->id == transitionId; });
		if (it != myTransitions.end())
		{
			return (*it).get();
		}

		return nullptr;
	}

	const int32_t AnimationStateMachine::GetStateIndexFromId(const UUID stateId) const
	{
		auto it = std::find_if(myStates.begin(), myStates.end(), [stateId](const auto& lhs) { return lhs->id == stateId; });
		if (it == myStates.end())
		{
			return -1;
		}

		const auto diff = std::distance(myStates.begin(), it);
		return (int32_t)diff;
	}

	template<typename T>
	inline static bool EvaluateParam(const T& evalData, const T& srcData, AnimationEvalMode evalMode)
	{
		switch (evalMode)
		{
			case AnimationEvalMode::Equal: return evalData == srcData;
			case AnimationEvalMode::NotEqual: return evalData != srcData;
			case AnimationEvalMode::Less: return evalData < srcData;
			case AnimationEvalMode::Greater: return evalData > srcData;
		}
		return false;
	}

	const bool AnimationStateMachine::EvaluateTransition(const AnimationTransitionEvaluation& evalParam) const
	{
		if (!myBlackboard.contains(evalParam.parameterName))
		{
			return false;
		}

		const auto& param = myBlackboard.at(evalParam.parameterName);

		switch (evalParam.type)
		{
			case AnimationEvalType::Bool:
			{
				const bool& bSrcParam = std::any_cast<const bool&>(param);
				const bool& bParam = std::any_cast<const bool&>(evalParam.value);

				return EvaluateParam(bParam, bSrcParam, evalParam.mode);
			}

			case AnimationEvalType::Float:
			{
				const float& bSrcParam = std::any_cast<const float&>(param);
				const float& bParam = std::any_cast<const float&>(evalParam.value);

				return EvaluateParam(bParam, bSrcParam, evalParam.mode);
			}
		}

		return false;
	}

	const bool AnimationStateMachine::ShouldTransition(const UUID transitionId) const
	{
		const auto transition = GetTransitionFromId(transitionId);
		if (!transition)
		{
			return;
		}

		bool result = true;
		for (const auto& e : transition->evaluations)
		{
			result &= EvaluateTransition(e);
		}

		return result;
	}
}