#pragma once

#include "Volt/Asset/Animation/Animation.h"

#include <GraphKey/Nodes/Animation/BaseAnimationNodes.h>

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
		UUID64 id;
		UUID64 fromState;
		UUID64 toState;

		bool hasExitTime = false;
		bool shouldBlend = true;
		float exitStartValue = 1.f;

		float blendTime = 1.f;

		Ref<AnimationTransitionGraph> transitionGraph;
	};

	struct AnimationState
	{
		AnimationState(const std::string& aName, bool aIsEntry, bool aIsAny)
			: name(aName), isEntry(aIsEntry), isAny(aIsAny)
		{}

		std::vector<UUID64> transitions;
		std::string name;

		Ref<AnimationGraphAsset> stateGraph;
		UUID64 id{};
		UUID64 pinId{};
		UUID64 pinId2{};

		std::string editorState;
		bool isEntry = false;
		bool isAny = false;
		float startTime = 0.f;
		float speed = 1.f;
	};

	class AnimationStateMachine
	{
	public:
		AnimationStateMachine(const std::string& name, AssetHandle characterHandle);

		void Update(float deltaTime);
		const GraphKey::AnimationOutputData Sample(Ref<Skeleton> skeleton);

		void AddState(const std::string& name, bool isEntry = false, bool isAny = false);
		void AddTransition(const UUID64 startState, const UUID64 endState);

		Ref<AnimationState> CreateState(const std::string& name, bool isEntry, const UUID64 id);
		Ref<AnimationTransition> CreateTransition(UUID64 id);

		void RemoveState(const UUID64 id);
		void RemoveTransition(const UUID64 id);

		void SetStartState(const UUID64 stateId);

		AnimationState* GetStateById(const UUID64 stateId) const;
		AnimationTransition* GetTransitionById(const UUID64 transitionId) const;

		AnimationState* GetStateFromPin(const UUID64 outputId) const;
		const int32_t GetStateIndexFromId(const UUID64 stateId) const;

		Ref<AnimationStateMachine> CreateCopy(GraphKey::Graph* ownerGraph, entt::entity entity = entt::null) const;

		void OnEvent(Event& e);

		inline const std::vector<Ref<AnimationState>>& GetStates() const { return myStates; }
		inline const std::vector<Ref<AnimationTransition>>& GetTransitions() const { return myTransitions; }
		inline const std::string& GetEditorState() const { return myState; }
		inline const AssetHandle GetCharacterHandle() const { return myCharacterHandle; }

		inline const std::string& GetName() const { return myName; }
		inline void SetEditorState(const std::string& state) { myState = state; }
		inline void SetName(const std::string& name) { myName = name; }

		void SetCharacterHandle(AssetHandle handle);

		void Clear();

	private:
		const bool ShouldTransition(const UUID64 transitionId, const UUID64 currentStateId) const;
		const GraphKey::AnimationOutputData CrossfadeTransition();
		const GraphKey::AnimationOutputData SampleState(int32_t stateIndex);
		
		void SetNextState(const UUID64 targetStateId, const UUID64 transitionId);

		std::string myState;
		std::string myName;
		
		int32_t myStartState = -1;
		int32_t myCurrentState = -1;
		int32_t myLastState = -1;

		bool myIsBlendingTransition = false;
		bool myStateChanged = false;
		float myCurrentBlendTotalTime = 0.f;
		float myCurrentBlendingTime = 0.f;

		std::vector<Ref<AnimationState>> myStates;
		std::vector<Ref<AnimationTransition>> myTransitions;

		AssetHandle myCharacterHandle = 0;
	};
}
