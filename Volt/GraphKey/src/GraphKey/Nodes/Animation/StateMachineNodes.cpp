#include "gkpch.h"
#include "StateMachineNodes.h"

#include <Volt/Asset/Animation/AnimationGraphAsset.h>
#include <Volt/Asset/Animation/Skeleton.h>
#include <Volt/Animation/AnimationTransitionGraph.h>
#include <Volt/Asset/AssetManager.h>

#include <Volt/Events/ApplicationEvent.h>
#include <Volt/Utility/YAMLSerializationHelpers.h>

namespace GraphKey
{
	StateMachineNode::StateMachineNode()
	{
		outputs =
		{
			AttributeConfigAnimationPose<AnimationOutputData>("Output", AttributeDirection::Output, GK_BIND_FUNCTION(StateMachineNode::SampleStateMachine))
		};

	}

	void StateMachineNode::OnEvent(Volt::Event& e)
	{
		Volt::EventDispatcher dispatcher(e);
		dispatcher.Dispatch<Volt::AppUpdateEvent>([&](Volt::AppUpdateEvent& event)
		{
			if (myStateMachine)
			{
				myStateMachine->Update(event.GetTimestep());
			}

			return false;
		});

		if (myStateMachine)
		{
			myStateMachine->OnEvent(e);
		}
	}

	void StateMachineNode::Initialize()
	{
		Volt::AnimationGraphAsset* animGraph = reinterpret_cast<Volt::AnimationGraphAsset*>(myGraph);
		if (!myStateMachine)
		{
			myStateMachine = CreateRef<Volt::AnimationStateMachine>("New State Machine", animGraph->GetSkeletonHandle());
		}
	}

	void StateMachineNode::Serialize(YAMLStreamWriter& out)
	{
		out.BeginMapNamned("StateMachine");
		out.SetKey("name", myStateMachine->GetName());
		out.SetKey("editorState", myStateMachine->GetEditorState());
		out.SetKey("skeletonHandle", myStateMachine->GetSkeletonHandle());

		out.BeginSequence("States");
		for (const auto& state : myStateMachine->GetStates())
		{
			out.BeginMap();
			out.SetKey("name", state->name);
			out.SetKey("editorState", state->editorState);
			out.SetKey("stateType", ToString(state->stateType));

			out.SetKey("id", state->id);
			out.SetKey("topPinId", state->topPinId);
			out.SetKey("bottomPinId", state->bottomPinId);

			out.BeginSequence("Transitions");
			for (const auto& transition : state->transitions)
			{
				out.AddValue(transition);
			}
			out.EndSequence();

			if (state->stateType == Volt::StateMachineStateType::AnimationState)
			{
				auto animState = Volt::AnimationStateMachine::AsAnimationState(state);
				if (animState->stateGraph)
				{
					out.SetKey("skeletonHandle", animState->stateGraph->GetSkeletonHandle());
					Graph::Serialize(animState->stateGraph, out);
				}
			}
			else if (state->stateType == Volt::StateMachineStateType::AliasState)
			{
				auto aliasState = std::reinterpret_pointer_cast<Volt::AliasState>(state);
				out.BeginSequence("TransitionFromStates");
				for (const auto& transition : aliasState->transitionFromStates)
				{
					out.AddValue(transition);
				}
				out.EndSequence();
			}
			out.EndMap();
		}
		out.EndSequence();
		out.EndMap();
	}

	void StateMachineNode::Deserialize(YAMLStreamReader& node)
	{
		if (!node.HasKey("StateMachine"))
		{
			return;
		}

		std::string stateMachineName;
		std::string stateMachineEditorState;
		Volt::AssetHandle stateMachineCharacterHandle;

		node.EnterScope("StateMachine");
		stateMachineName = node.ReadAtKey("name", std::string("Null"));
		stateMachineEditorState = node.ReadAtKey("editorState", std::string());
		stateMachineCharacterHandle = node.ReadAtKey("characterHandle", Volt::AssetHandle(0));
		node.ExitScope();

		myStateMachine = CreateRef<Volt::AnimationStateMachine>(stateMachineName, stateMachineCharacterHandle);
		myStateMachine->Clear();

		node.ForEach("States", [&]()
		{
			std::string stateName = node.ReadAtKey("name", std::string("Null"));
			UUID64 stateId = node.ReadAtKey("id", UUID64(0));
			std::string stateTypeString = node.ReadAtKey("stateType", std::string("Null"));

			Volt::StateMachineStateType stateType = ToEnum<Volt::StateMachineStateType>(stateTypeString);
			auto newState = myStateMachine->AddState(stateName, stateType, stateId);

			newState->editorState = node.ReadAtKey("editorState", std::string());
			newState->topPinId = node.ReadAtKey("topPinId", UUID64(0));
			newState->bottomPinId = node.ReadAtKey("bottomPinId", UUID64(0));

			node.ForEach("Transitions", [&]()
			{
				newState->transitions.emplace_back(node.ReadValue<UUID64>());
			});

			if (stateType == Volt::StateMachineStateType::AnimationState)
			{
				auto animState = Volt::AnimationStateMachine::AsAnimationState(newState);
				if (node.HasKey("Graph"))
				{
					Volt::AssetHandle characterHandle = node.ReadAtKey("characterHandle", Volt::AssetHandle(0));
					animState->stateGraph = CreateRef<Volt::AnimationGraphAsset>(characterHandle);

					node.EnterScope("Graph");
					Graph::Deserialize(animState->stateGraph, node);
					node.ExitScope();
				}
			}
			else if (stateType == Volt::StateMachineStateType::AliasState)
			{
				auto aliasState = std::reinterpret_pointer_cast<Volt::AliasState>(newState);
				node.ForEach("TransitionFromStates", [&]()
				{
					aliasState->transitionFromStates.emplace_back(node.ReadValue<UUID64>());
				});
			}
		});

		node.ForEach("Transitions", [&]()
		{
			UUID64 transitionId = node.ReadAtKey("id", UUID64(0));
			UUID64 transitionFromState = node.ReadAtKey("fromState", UUID64(0));
			UUID64 transitionToState = node.ReadAtKey("toState", UUID64(0));

			auto newTransition = myStateMachine->CreateTransition(transitionId);
			newTransition->fromState = transitionFromState;
			newTransition->toState = transitionToState;

			newTransition->shouldBlend = node.ReadAtKey("shouldBlend", false);
			newTransition->blendTime = node.ReadAtKey("blendTime", 1.f);

			if (node.HasKey("Graph"))
			{
				newTransition->transitionGraph = CreateRef<Volt::AnimationTransitionGraph>();
				newTransition->transitionGraph->SetStateMachine(myStateMachine.get());
				newTransition->transitionGraph->SetTransitionID(transitionId);

				node.EnterScope("Graph");
				Graph::Deserialize(std::reinterpret_pointer_cast<Graph>(newTransition->transitionGraph), node);
				node.ExitScope();
			}
		});

		bool hasEntryState = false;

		for (const auto& state : myStateMachine->GetStates())
		{
			hasEntryState |= state->stateType == Volt::StateMachineStateType::EntryState;

			if (hasEntryState)
			{
				break;
			}
		}

		if (!hasEntryState)
		{
			myStateMachine->AddState("Entry", Volt::StateMachineStateType::EntryState);
		}
	}

	const std::string StateMachineNode::GetName()
	{
		if (myStateMachine)
		{
			return myStateMachine->GetName();
		}
		return "State Machine";
	}

	Ref<Node> StateMachineNode::CreateCopy(Graph* ownerGraph, Volt::EntityID entity)
	{
		Ref<Node> copy = Node::CreateCopy(ownerGraph, entity);
		Ref<StateMachineNode> stateMachineNode = std::reinterpret_pointer_cast<StateMachineNode>(copy);
		stateMachineNode->myStateMachine = myStateMachine->CreateCopy(ownerGraph, entity);

		return copy;
	}

	void StateMachineNode::SampleStateMachine()
	{
		if (!myStateMachine)
		{
			return;
		}

		Volt::AnimationGraphAsset* animGraph = reinterpret_cast<Volt::AnimationGraphAsset*>(myGraph);
		const auto skeleton = Volt::AssetManager::GetAsset<Volt::Skeleton>(animGraph->GetSkeletonHandle());

		if (!skeleton || !skeleton->IsValid())
		{
			return;
		}

		SetOutputData(0, myStateMachine->Sample(skeleton));
	}
}

