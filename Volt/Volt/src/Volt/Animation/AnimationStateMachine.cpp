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

namespace Volt
{
	AnimationStateMachine::AnimationStateMachine(const std::string& name, AssetHandle aSkeletonHandle)
		: myName(name), mySkeletonHandle(aSkeletonHandle)
	{
		AddState("Entry", true);
		AddState("Any", false, true);
	}

	void AnimationStateMachine::Update(float deltaTime)
	{
		// if no state is active, check if the entry state has any transitions and choose the first one
		if (myCurrentState == -1)
		{
			for (const auto& state : myStates)
			{
				if (state->isEntry && !state->transitions.empty())
				{
					auto transition = GetTransitionById(state->transitions.front());
					if (transition)
					{
						SetStartState(transition->toState);
						myCurrentState = myStartState;
						auto startState = myStates.at(myStartState);
						startState->startTime = AnimationManager::globalClock;
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
		for (const auto& state : myStates)
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
		}

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
			if (state->stateGraph)
			{
				state->stateGraph->OnEvent(e);
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
			if (state->stateGraph)
			{
				state->stateGraph->SetSkeletonHandle(aSkeletonHandle);
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

		const auto& currState = myStates.at(myCurrentState);
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

	void AnimationStateMachine::AddState(const std::string& name, bool isEntry, bool isAny)
	{
		auto state = CreateRef<AnimationState>(name, isEntry, isAny);
		if (!isEntry && !isAny)
		{
			state->stateGraph = CreateRef<AnimationGraphAsset>(mySkeletonHandle);
			state->stateGraph->AddNode(GraphKey::Registry::Create("OutputPoseNode"));
		}

		myStates.emplace_back(state);
	}

	Ref<AnimationState> AnimationStateMachine::CreateState(const std::string& name, bool isEntry, const UUID id)
	{
		auto state = CreateRef<AnimationState>(name, isEntry, false);
		state->id = id;

		myStates.emplace_back(state);
		return state;
	}

	Ref<AnimationTransition> AnimationStateMachine::CreateTransition(Volt::UUID id)
	{
		auto transition = CreateRef<AnimationTransition>();
		transition->id = id;

		myTransitions.emplace_back(transition);
		return transition;
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

		Ref<AnimationState> node = *it;
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

		AnimationState* fromState = GetStateById(transition->fromState);
		AnimationState* toState = GetStateById(transition->toState);

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

	AnimationState* AnimationStateMachine::GetStateById(const UUID stateId) const
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

	AnimationState* AnimationStateMachine::GetStateFromPin(const UUID pinId) const
	{
		auto it = std::find_if(myStates.begin(), myStates.end(), [pinId](const auto& lhs) { return lhs->pinId == pinId || lhs->pinId2 == pinId; });
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
			Ref<AnimationState> newState = CreateRef<AnimationState>(state->name, state->isEntry, state->isAny);
			newState->transitions = state->transitions;
			newState->id = state->id;
			newState->pinId = state->pinId;
			newState->pinId2 = state->pinId2;
			if (state->stateGraph)
			{
				newState->stateGraph = state->stateGraph->CreateCopy(entity);
				newState->stateGraph->SetParentBlackboard(&ownerGraph->GetBlackboard());
			}

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
			newTransition->transitionGraph->SetStateMachine(this);
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
		//		Ref<Animation> longestAnimation;
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
		auto state = myStates.at(stateIndex);
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

		auto statePtr = myStates.at(myCurrentState);
		statePtr->startTime = AnimationManager::globalClock;
	}
}
