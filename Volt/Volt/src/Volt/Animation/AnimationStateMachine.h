#pragma once

#include "Volt/Asset/Animation/Animation.h"
#include <any>

namespace Volt
{
	class Animation;

	enum class AnimationEvalType
	{
		Float,
		Bool
	};

	enum class AnimationEvalMode
	{
		Equal,
		NotEqual,
		Less,
		Greater
	};

	struct AnimationTransitionEvaluation
	{
		AnimationEvalType type;
		AnimationEvalMode mode;
		std::string parameterName;
		std::any value;
	};

	struct AnimationTransition
	{
		UUID id;
		UUID fromState;
		UUID toState;

		std::vector<AnimationTransitionEvaluation> evaluations;
	};

	struct AnimationState
	{
		std::vector<UUID> transitions;
		Ref<Animation> animation;
		UUID id{};

		bool isLooping;
	};

	class AnimationStateMachine
	{
	public:
		void Update(float deltaTime);
		const std::vector<Animation::TRS> Sample(float startTime, Ref<Skeleton> skeleton) const;

		void SetStartState(const UUID stateId);

		AnimationState* GetStateFromId(const UUID stateId) const;
		AnimationTransition* GetTransitionFromId(const UUID transitionId) const;
		const int32_t GetStateIndexFromId(const UUID stateId) const;

	private:
		const bool ShouldTransition(const UUID transitionId) const;
		const bool EvaluateTransition(const AnimationTransitionEvaluation& evalParam) const;

		int32_t myStartState = -1;
		int32_t myCurrentState = -1;
		std::vector<Ref<AnimationState>> myStates;
		std::vector<Ref<AnimationTransition>> myTransitions;

		std::unordered_map<std::string, std::any> myBlackboard;
	};
}