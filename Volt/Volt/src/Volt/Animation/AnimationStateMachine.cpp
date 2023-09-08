#include "vtpch.h"
#include "AnimationStateMachine.h"

#include "Volt/Animation/AnimationTransitionGraph.h"
#include "Volt/Animation/AnimationManager.h"
#include "Volt/Asset/Animation/AnimationGraphAsset.h"

#include "Volt/Core/Profiling.h"

#include <GraphKey/Nodes/Animation/TransitionNodes.h>
#include <GraphKey/Nodes/Animation/BaseAnimationNodes.h>
#include <GraphKey/Nodes/Animation/SequenceNodes.h>

#include <GraphKey/Registry.h>

#include "Volt/Utility/EnumUtil.h"

CREATE_ENUM(TestEnum, uint32_t,
	One,
	Two,
	Three
);

namespace Volt
{
	AnimationStateMachine::AnimationStateMachine(const std::string& name, AssetHandle aSkeletonHandle)
		: myName(name), mySkeletonHandle(aSkeletonHandle)
	{
		AddState("Entry", StateMachineStateType::EntryState);
	}

	void AnimationStateMachine::Update(float deltaTime)
	{
		// if no state is active, check if the entry state has any transitions and choose the first one
		if (myCurrentState == -1)
		{
			for (const auto& state : myStates)
			{
				if (state->stateType == StateMachineStateType::EntryState && !state->transitions.empty())
				{
					auto transition = GetTransitionById(state->transitions.front());
					if (transition)
					{
						SetStartState(transition->toState);
						myCurrentState = myStartState;
						auto startState = std::reinterpret_pointer_cast<AnimationState>(myStates.at(myStartState));
						startState->startTime = AnimationManager::globalClock;
						startState->stateGraph->SetStartTime(AnimationManager::globalClock);
					}
				}
			}
		}

		if (myCurrentState == -1 || myCurrentState >= (int32_t)myStates.size())
		{
			return;
		}

		if (myIsBlendingTransition)
		{
			myCurrentBlendingTime += deltaTime;
		}

		// Check any state for transitions
		/*for (const auto& state : myStates)
		{
			if (state->isAny)
			{
				for (const auto transitionId : state->transitions)
				{
					if (ShouldTransition(transitionId, state->id))
					{
						const auto transition = GetTransitionById(transitionId);
						if (transition->toState == myStates.at(myCurrentState)->id)
						{
							continue;
						}

						SetNextState(transition->toState, transitionId);
						return;
					}
				}
				break;
			}
		}*/

		const auto& currState = myStates.at(myCurrentState);

		for (const auto& id : currState->transitions)
		{
			const auto transition = GetTransitionById(id);
			if (!transition)
			{
				continue;
			}

			if (ShouldTransition(id, currState->id))
			{
				SetNextState(transition->toState, id);
				break;
			}
		}
	}

	void AnimationStateMachine::OnEvent(Event& e)
	{
		for (const auto& state : myStates)
		{
			if (state->stateType == StateMachineStateType::AnimationState)
			{
				AsAnimationState(state)->stateGraph->OnEvent(e);
			}
		}

		for (const auto& transition : myTransitions)
		{
			if (transition->transitionGraph)
			{
				transition->transitionGraph->OnEvent(e);
			}
		}
	}

	void AnimationStateMachine::SetSkeletonHandle(AssetHandle aSkeletonHandle)
	{
		mySkeletonHandle = aSkeletonHandle;
		for (const auto& state : myStates)
		{
			if (state->stateType == StateMachineStateType::AnimationState)
			{
				AsAnimationState(state)->stateGraph->SetSkeletonHandle(aSkeletonHandle);
			}
		}
	}

	void AnimationStateMachine::Clear()
	{
		myStates.clear();
		myTransitions.clear();
	}

	const GraphKey::AnimationOutputData AnimationStateMachine::Sample(Ref<Skeleton> skeleton)
	{
		VT_PROFILE_FUNCTION();

		if (myCurrentState == -1 || myCurrentState >= (int32_t)myStates.size())
		{
			return {};
		}

		const auto& currState = GetAnimationState(myCurrentState);
		const auto nodes = currState->stateGraph->GetNodesOfType("OutputPoseNode");
		if (nodes.empty())
		{
			return {};
		}

		if (myIsBlendingTransition && myLastState != -1)
		{
			if (myCurrentBlendingTime < myCurrentBlendTotalTime)
			{
				return CrossfadeTransition();
			}
			else
			{
				myIsBlendingTransition = false;
			}
		}

		auto outputPoseNode = std::reinterpret_pointer_cast<GraphKey::OutputPoseNode>(nodes.at(0));
		auto sample = outputPoseNode->Sample(false, currState->startTime);

		if (myStateChanged)
		{
			myStateChanged = false;
		}

		return sample;
	}

	Ref<AnimationTransition> AnimationStateMachine::CreateTransition(Volt::UUID id)
	{
		auto transition = CreateRef<AnimationTransition>();
		transition->id = id;

		myTransitions.emplace_back(transition);
		return transition;
	}

	void AnimationStateMachine::AddState(const std::string& name, StateMachineStateType aStateType, const UUID id)
	{
		switch (aStateType)
		{
			case StateMachineStateType::AnimationState:
			{
				auto animationState = CreateRef<AnimationState>(name);
				animationState->stateGraph = CreateRef<AnimationGraphAsset>(mySkeletonHandle);
				animationState->stateGraph->AddNode(GraphKey::Registry::Create("OutputPoseNode"));
				myStates.emplace_back(animationState);
				break;
			}

			case StateMachineStateType::AliasState:
			{
				auto aliasState = CreateRef<AliasState>(name);
				myStates.emplace_back(aliasState);
				break;
			}

			case StateMachineStateType::EntryState:
			{
				bool alreadyHasEntry = false;
				for (auto& state : myStates)
				{
					if (state->stateType == StateMachineStateType::EntryState)
					{
						alreadyHasEntry = true;
						break;
					}
				}

				if (!alreadyHasEntry)
				{
					auto state = CreateRef<AnimationState>("Entry", aStateType);
					myStates.emplace_back(state);
				}
				break;
			}
		}
	}

	void AnimationStateMachine::AddTransition(const UUID startStateId, const UUID endStateId)
	{
		Ref<AnimationTransition> transition = CreateRef<AnimationTransition>();
		transition->fromState = startStateId;
		transition->toState = endStateId;

		auto startState = GetStateById(startStateId);
		auto endState = GetStateById(endStateId);

		if (startState && endState)
		{
			startState->transitions.emplace_back(transition->id);
			endState->transitions.emplace_back(transition->id);
		}

		transition->transitionGraph = CreateRef<AnimationTransitionGraph>();
		transition->transitionGraph->SetStateMachine(this);
		transition->transitionGraph->SetTransitionID(transition->id);
		transition->transitionGraph->AddNode(GraphKey::Registry::Create("TransitionOutputNode"));
		myTransitions.emplace_back(transition);
	}

	void AnimationStateMachine::RemoveState(const UUID id)
	{
		auto it = std::find_if(myStates.begin(), myStates.end(), [&id](const auto& lhs)
		{
			return lhs->id == id;
		});

		if (it == myStates.end())
		{
			return;
		}

		Ref<StateMachineState> node = *it;
		for (const auto& t : node->transitions)
		{
			RemoveTransition(t);
		}

		myStates.erase(it);
	}

	void AnimationStateMachine::RemoveTransition(const UUID id)
	{
		auto it = std::find_if(myTransitions.begin(), myTransitions.end(), [&id](const auto& lhs)
		{
			return lhs->id == id;
		});

		if (it == myTransitions.end())
		{
			return;
		}

		auto transition = *it;

		StateMachineState* fromState = GetStateById(transition->fromState);
		StateMachineState* toState = GetStateById(transition->toState);

		fromState->transitions.erase(std::remove(fromState->transitions.begin(), fromState->transitions.end(), transition->id), fromState->transitions.end());
		toState->transitions.erase(std::remove(toState->transitions.begin(), toState->transitions.end(), transition->id), toState->transitions.end());

		myTransitions.erase(it);
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

	StateMachineState* AnimationStateMachine::GetStateById(const UUID stateId) const
	{
		auto it = std::find_if(myStates.begin(), myStates.end(), [stateId](const auto& lhs) { return lhs->id == stateId; });
		if (it != myStates.end())
		{
			return (*it).get();
		}

		return nullptr;
	}

	AnimationTransition* AnimationStateMachine::GetTransitionById(const UUID transitionId) const
	{
		auto it = std::find_if(myTransitions.begin(), myTransitions.end(), [transitionId](const auto& lhs) { return lhs->id == transitionId; });
		if (it != myTransitions.end())
		{
			return (*it).get();
		}

		return nullptr;
	}

	StateMachineState* AnimationStateMachine::GetStateFromPin(const UUID pinId) const
	{
		auto it = std::find_if(myStates.begin(), myStates.end(), [pinId](const auto& lhs) { return lhs->topPinId == pinId || lhs->bottomPinId == pinId; });
		if (it != myStates.end())
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

	Ref<AnimationStateMachine> AnimationStateMachine::CreateCopy(GraphKey::Graph* ownerGraph, Wire::EntityId entity) const
	{
		Ref<AnimationStateMachine> newStateMachine = CreateRef<AnimationStateMachine>(myName, mySkeletonHandle);
		newStateMachine->myStartState = myStartState;
		newStateMachine->myCurrentState = myCurrentState;

		newStateMachine->myStates.clear();

		for (const auto& state : myStates)
		{
			Ref<StateMachineState> newState;
			if (state->stateType == StateMachineStateType::AnimationState)
			{
				newState = CreateRef<AnimationState>(state->name, state->stateType);
				auto newAnimState = std::reinterpret_pointer_cast<AnimationState>(newState);
				auto oldAnimState = std::reinterpret_pointer_cast<AnimationState>(state);
				if (oldAnimState->stateGraph)
				{
					newAnimState->stateGraph = oldAnimState->stateGraph->CreateCopy(entity);
					newAnimState->stateGraph->SetParentBlackboard(&ownerGraph->GetBlackboard());
				}
			}
			else if (state->stateType == StateMachineStateType::AliasState)
			{
				newState = CreateRef<AliasState>(state->name);
				auto newAliasState = std::reinterpret_pointer_cast<AliasState>(newState);
				auto oldAliasState = std::reinterpret_pointer_cast<AliasState>(state);
				
				newAliasState->transitionFromStates = oldAliasState->transitionFromStates;
			}
			else
			{
				newState = CreateRef<StateMachineState>(state->name, state->stateType);
			}
			newState->transitions = state->transitions;
			newState->id = state->id;
			newState->topPinId = state->topPinId;
			newState->bottomPinId = state->bottomPinId;
			newState->editorState = state->editorState;

			newStateMachine->myStates.emplace_back(newState);
		}

		for (const auto& transition : myTransitions)
		{
			Ref<AnimationTransition> newTransition = CreateRef<AnimationTransition>();
			newTransition->id = transition->id;
			newTransition->fromState = transition->fromState;
			newTransition->toState = transition->toState;

			newTransition->blendTime = transition->blendTime;
			newTransition->shouldBlend = transition->shouldBlend;

			newTransition->transitionGraph = CreateRef<AnimationTransitionGraph>();
			newTransition->transitionGraph->SetStateMachine(newStateMachine.get());
			newTransition->transitionGraph->SetTransitionID(newTransition->id);
			newTransition->transitionGraph->SetEntity(entity);
			GraphKey::Graph::Copy(transition->transitionGraph, newTransition->transitionGraph);

			newTransition->transitionGraph->SetParentBlackboard(&ownerGraph->GetBlackboard());
			newStateMachine->myTransitions.emplace_back(newTransition);
		}

		return newStateMachine;
	}

	const bool AnimationStateMachine::ShouldTransition(const UUID transitionId, const UUID currentStateId) const
	{
		const auto transition = GetTransitionById(transitionId);
		const auto currentState = GetStateById(currentStateId);
		if (!transition || !currentState)
		{
			return false;
		}

		if (transition->toState == currentStateId)
		{
			return false;
		}

		const auto nodes = transition->transitionGraph->GetNodesOfType("TransitionOutputNode");
		if (nodes.empty())
		{
			return false;
		}

		bool canTransition = true;/*transition->hasExitTime ? false : true;*/

		//if (transition->hasExitTime)
		//{
		//	const auto playerNodes = currentState->stateGraph->GetNodesOfType("SequencePlayerNode");

		//	if (!playerNodes.empty())
		//	{
		//		Ref<Animation>  ;
		//		Ref<GraphKey::SequencePlayerNode> longestPlayer;
		//		float longestAnimationTime = std::numeric_limits<float>::lowest();
		//		for (const auto& player : playerNodes)
		//		{
		//			auto playerNodeType = std::reinterpret_pointer_cast<GraphKey::SequencePlayerNode>(player);
		//			float speed = 1.f;
		//			for (auto& input : playerNodeType->inputs)
		//			{
		//				if (input.name == "Speed")
		//				{
		//					speed = std::any_cast<float>(input.data);
		//				}
		//			}
		//			auto anim = playerNodeType->GetAnimation();
		//			if (!anim || !anim->IsValid())
		//			{
		//				continue;
		//			}
		//			const auto duration = (anim->GetDuration() * speed);
		//			if (duration > longestAnimationTime)
		//			{
		//				longestAnimationTime = anim->GetDuration() * speed;
		//				longestAnimation = anim;
		//				longestPlayer = playerNodeType;
		//			}
		//		}

		//		if (longestPlayer)
		//		{
		//			const float animSpeed = longestPlayer->GetInput<float>(2);
		//			const float endTime = longestAnimationTime * transition->exitStartValue;

		//			if (longestAnimation->IsAtEnd(currentState->startTime, animSpeed)/* || longestAnimation->HasPassedTime(currentState->startTime, animSpeed, endTime)*/)
		//			{
		//				canTransition = true;
		//			}
		//		}
		//	}
		//}

		auto transitionOutputNode = std::reinterpret_pointer_cast<GraphKey::TransitionOutputNode>(nodes.front());
		const bool result = transitionOutputNode->Evaluate();
		return result && canTransition;
	}

	const GraphKey::AnimationOutputData AnimationStateMachine::CrossfadeTransition()
	{
		auto currentState = myStates.at(myCurrentState);
		auto lastState = myStates.at(myLastState);

		if (!currentState || !lastState)
		{
			return {};
		}

		GraphKey::AnimationOutputData currentSample = SampleState(myCurrentState);
		GraphKey::AnimationOutputData lastSample = SampleState(myLastState);

		if (currentSample.pose.empty() || lastSample.pose.empty() || (currentSample.pose.size() != lastSample.pose.size()))
		{
			return {};
		}

		const float t = myCurrentBlendingTime / myCurrentBlendTotalTime;

		std::vector<Volt::Animation::TRS> result{ currentSample.pose.size() };
		for (size_t i = 0; i < result.size(); i++)
		{
			const auto& aTRS = lastSample.pose.at(i);
			const auto& bTRS = currentSample.pose.at(i);

			result[i].position = glm::mix(aTRS.position, bTRS.position, t);
			result[i].rotation = glm::slerp(aTRS.rotation, bTRS.rotation, t);
			result[i].scale = glm::mix(aTRS.scale, bTRS.scale, t);
		}

		GraphKey::AnimationOutputData output{};
		output.pose = result;
		output.rootTRS = currentSample.rootTRS;

		return output;
	}

	const GraphKey::AnimationOutputData AnimationStateMachine::SampleState(int32_t stateIndex)
	{
		auto state = GetAnimationState(stateIndex);
		if (!state)
		{
			return {};
		}

		const auto nodes = state->stateGraph->GetNodesOfType("OutputPoseNode");
		if (nodes.empty())
		{
			return {};
		}

		auto outputPoseNode = std::reinterpret_pointer_cast<GraphKey::OutputPoseNode>(nodes.at(0));
		const auto sample = outputPoseNode->Sample(false, state->startTime);
		return sample;
	}

	Ref<AnimationState> AnimationStateMachine::GetAnimationState(uint32_t aIndex)
	{
		auto state = myStates.at(aIndex);
		if (state->stateType == StateMachineStateType::AnimationState) [[likely]]
		{
			return std::reinterpret_pointer_cast<AnimationState>(state);
		}
		else [[unlikely]]
		{
			VT_CORE_ERROR("Tried to get AnimationState from an index that does not contain an Animation state");
			return Ref<AnimationState>();
		}
	}

	Ref<AnimationState> AnimationStateMachine::AsAnimationState(Ref<StateMachineState> aState)
	{
		if (aState->stateType != StateMachineStateType::AnimationState) [[unlikely]]
		{
			VT_CORE_ERROR("Tried to convert a StateMachineState to AnimationState that doesnt have that type");
			return Ref<AnimationState>();
		}
		return std::reinterpret_pointer_cast<AnimationState>(aState);
	}

	void AnimationStateMachine::SetNextState(const UUID targetStateId, const UUID transitionId)
	{
		const auto transition = GetTransitionById(transitionId);

		myLastState = myCurrentState;
		myCurrentState = GetStateIndexFromId(targetStateId);
		myStateChanged = true;

		if (transition->shouldBlend)
		{
			myIsBlendingTransition = true;
			myCurrentBlendTotalTime = transition->blendTime;
			myCurrentBlendingTime = 0.f;
		}

		auto statePtr = GetAnimationState(myCurrentState);
		statePtr->startTime = AnimationManager::globalClock;
		statePtr->stateGraph->SetStartTime(AnimationManager::globalClock);
	}
}
