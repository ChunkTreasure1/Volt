#pragma once

#include "Volt/Asset/Animation/Animation.h"

#include <GraphKey/Nodes/Animation/BaseAnimationNodes.h>
#include <Wire/Entity.h>

#include <any>

namespace GraphKey
{
	class Graph;
}

namespace Volt
{
	class Animation;
	class AnimationTransitionGraph;
	class AnimationGraphAsset;
	class Event;

	struct AnimationTransition
	{
		UUID id;
		UUID fromState;
		UUID toState;

		bool shouldBlend = true;
		float blendTime = 1.f;

		Ref<AnimationTransitionGraph> transitionGraph;
	};

	enum class StateMachineStateType : uint8_t
	{
		AnimationState,
		AliasState,
		EntryState
	};

	struct StateMachineState
	{
		StateMachineState(const std::string& aName, StateMachineStateType aStateType)
			:name(aName), stateType(aStateType)
		{

		}

		std::vector<UUID> transitions;
		std::string name;

		UUID id{};
		UUID topPinId{};
		UUID bottomPinId{};

		const StateMachineStateType stateType;

		//This is used by the node library to save the positions and stuff for the nodes
		std::string editorState;
	};
	struct AnimationState : public StateMachineState
	{
		AnimationState(const std::string& aName)
			: StateMachineState(aName, StateMachineStateType::AnimationState)
		{
		}

		Ref<AnimationGraphAsset> stateGraph;

		float startTime = 0.f;
		float speed = 1.f;
	};

	struct AliasState : public StateMachineState
	{
		AliasState(const std::string& aName)
			: StateMachineState(aName, StateMachineStateType::AliasState)
		{
		}

		std::vector<UUID> transitionFromStates;

	};

	class AnimationStateMachine
	{
	public:
		AnimationStateMachine(const std::string& name, AssetHandle aSkeletonHandle);

		void Update(float deltaTime);
		const GraphKey::AnimationOutputData Sample(Ref<Skeleton> skeleton);

		void AddState(const std::string& name, StateMachineStateType aStateType, const UUID id = 0);
		void AddTransition(const UUID startState, const UUID endState);

		Ref<AnimationTransition> CreateTransition(Volt::UUID id);

		void RemoveState(const UUID id);
		void RemoveTransition(const UUID id);

		void SetStartState(const UUID stateId);

		StateMachineState* GetStateById(const UUID stateId) const;
		AnimationTransition* GetTransitionById(const UUID transitionId) const;

		StateMachineState* GetStateFromPin(const UUID outputId) const;
		const int32_t GetStateIndexFromId(const UUID stateId) const;

		Ref<AnimationStateMachine> CreateCopy(GraphKey::Graph* ownerGraph, Wire::EntityId entity = 0) const;

		void OnEvent(Event& e);

		inline const std::vector<Ref<StateMachineState>>& GetStates() const { return myStates; }
		inline const std::vector<Ref<AnimationTransition>>& GetTransitions() const { return myTransitions; }
		inline const std::string& GetEditorState() const { return myState; }
		inline const AssetHandle GetSkeletonHandle() const { return mySkeletonHandle; }

		inline const std::string& GetName() const { return myName; }
		inline void SetEditorState(const std::string& state) { myState = state; }
		inline void SetName(const std::string& name) { myName = name; }

		void SetSkeletonHandle(AssetHandle aSkeletonHandle);

		void Clear();

	private:
		const bool ShouldTransition(const UUID transitionId, const UUID currentStateId) const;
		const GraphKey::AnimationOutputData CrossfadeTransition();
		const GraphKey::AnimationOutputData SampleState(int32_t stateIndex);

		Ref<AnimationState> GetAnimationState(uint32_t aIndex);
		Ref<AnimationState> AsAnimationState(Ref<StateMachineState> aState);

		void SetNextState(const UUID targetStateId, const UUID transitionId);

		std::string myState;
		std::string myName;

		int32_t myStartState = -1;
		int32_t myCurrentState = -1;
		int32_t myLastState = -1;

		bool myIsBlendingTransition = false;
		bool myStateChanged = false;
		float myCurrentBlendTotalTime = 0.f;
		float myCurrentBlendingTime = 0.f;

		std::vector<Ref<StateMachineState>> myStates;
		std::vector<Ref<AnimationTransition>> myTransitions;

		AssetHandle mySkeletonHandle = 0;
	};
}
