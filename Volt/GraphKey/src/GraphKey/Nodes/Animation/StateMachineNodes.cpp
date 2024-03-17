#include "gkpch.h"
#include "StateMachineNodes.h"

#include <Volt/Asset/Animation/AnimationGraphAsset.h>
#include <Volt/Asset/Animation/Skeleton.h>
#include <Volt/Animation/AnimationTransitionGraph.h>
#include <Volt/Asset/AssetManager.h>

#include <Volt/Events/ApplicationEvent.h>
#include <Volt/Utility/SerializationMacros.h>
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

	void StateMachineNode::Serialize(YAML::Emitter& out)
	{
		out << YAML::Key << "StateMachine" << YAML::Value;
		out << YAML::BeginMap;
		{
			VT_SERIALIZE_PROPERTY(name, myStateMachine->GetName(), out);
			VT_SERIALIZE_PROPERTY(editorState, myStateMachine->GetEditorState(), out);
			VT_SERIALIZE_PROPERTY(skeletonHandle, myStateMachine->GetSkeletonHandle(), out);

			out << YAML::Key << "States" << YAML::BeginSeq;
			for (const auto& state : myStateMachine->GetStates())
			{
				out << YAML::BeginMap;
				{
					VT_SERIALIZE_PROPERTY(name, state->name, out);
					VT_SERIALIZE_PROPERTY(editorState, state->editorState, out);
					VT_SERIALIZE_PROPERTY(stateType, ToString(state->stateType), out);

					VT_SERIALIZE_PROPERTY(id, state->id, out);
					VT_SERIALIZE_PROPERTY(topPinId, state->topPinId, out);
					VT_SERIALIZE_PROPERTY(bottomPinId, state->bottomPinId, out);

					out << YAML::Key << "Transitions" << YAML::BeginSeq;
					for (const auto& transition : state->transitions)
					{
						out << transition;
					}
					out << YAML::EndSeq;
					if (state->stateType == Volt::StateMachineStateType::AnimationState)
					{
						auto animState = Volt::AnimationStateMachine::AsAnimationState(state);
						if (animState->stateGraph)
						{
							VT_SERIALIZE_PROPERTY(skeletonHandle, animState->stateGraph->GetSkeletonHandle(), out);
							Graph::Serialize(animState->stateGraph, out);
						}
					}
					else if (state->stateType == Volt::StateMachineStateType::AliasState)
					{
						auto aliasState = std::reinterpret_pointer_cast<Volt::AliasState>(state);
						out << YAML::Key << "TransitionFromStates" << YAML::BeginSeq;
						for (const auto& transition : aliasState->transitionFromStates)
						{
							out << transition;
						}
						out << YAML::EndSeq;
					}
				}
				out << YAML::EndMap;
			}
			out << YAML::EndSeq;

			out << YAML::Key << "Transitions" << YAML::BeginSeq;
			for (const auto& transition : myStateMachine->GetTransitions())
			{
				out << YAML::BeginMap;
				VT_SERIALIZE_PROPERTY(id, transition->id, out);
				VT_SERIALIZE_PROPERTY(fromState, transition->fromState, out);
				VT_SERIALIZE_PROPERTY(toState, transition->toState, out);
				VT_SERIALIZE_PROPERTY(shouldBlend, transition->shouldBlend, out);
				VT_SERIALIZE_PROPERTY(blendTime, transition->blendTime, out);

				if (transition->transitionGraph)
				{
					Graph::Serialize(std::reinterpret_pointer_cast<Graph>(transition->transitionGraph), out);
				}
				out << YAML::EndMap;
			}
			out << YAML::EndSeq;
		}
		out << YAML::EndMap;
	}

	void StateMachineNode::Deserialize(const YAML::Node& node)
	{
		if (!node["StateMachine"])
		{
			return;
		}

		std::string stateMachineName;
		std::string stateMachineEditorState;
		Volt::AssetHandle stateMachineCharacterHandle;

		auto rootNode = node["StateMachine"];
		VT_DESERIALIZE_PROPERTY(name, stateMachineName, rootNode, std::string("Null"));
		VT_DESERIALIZE_PROPERTY(editorState, stateMachineEditorState, rootNode, std::string(""));
		VT_DESERIALIZE_PROPERTY(characterHandle, stateMachineCharacterHandle, rootNode, Volt::AssetHandle(0));

		myStateMachine = CreateRef<Volt::AnimationStateMachine>(stateMachineName, stateMachineCharacterHandle);
		myStateMachine->Clear();

		for (const auto& stateNode : rootNode["States"])
		{
			std::string stateName;
			UUID64 stateId = UUID64(0);
			std::string stateTypeString;

			VT_DESERIALIZE_PROPERTY(name, stateName, stateNode, std::string("Null"));
			VT_DESERIALIZE_PROPERTY(id, stateId, stateNode, UUID64(0));
			VT_DESERIALIZE_PROPERTY(stateType, stateTypeString, stateNode, std::string("Null"));

			Volt::StateMachineStateType stateType = ToEnum<Volt::StateMachineStateType>(stateTypeString);

			auto newState = myStateMachine->AddState(stateName, stateType, stateId);

			VT_DESERIALIZE_PROPERTY(editorState, newState->editorState, stateNode, std::string(""));
			VT_DESERIALIZE_PROPERTY(topPinId, newState->topPinId, stateNode, UUID64(0));
			VT_DESERIALIZE_PROPERTY(bottomPinId, newState->bottomPinId, stateNode, UUID64(0));

			for (const auto& transitionNode : stateNode["Transitions"])
			{
				newState->transitions.emplace_back(transitionNode.as<UUID64>());
			}

			if (stateType == Volt::StateMachineStateType::AnimationState)
			{
				auto animState = Volt::AnimationStateMachine::AsAnimationState(newState);
				if (stateNode["Graph"])
				{
					Volt::AssetHandle characterHandle;
					VT_DESERIALIZE_PROPERTY(characterHandle, characterHandle, stateNode, Volt::AssetHandle(0));
					animState->stateGraph = CreateRef<Volt::AnimationGraphAsset>(characterHandle);
					Graph::Deserialize(animState->stateGraph, stateNode["Graph"]);
				}
			}
			else if (stateType == Volt::StateMachineStateType::AliasState)
			{
				auto aliasState = std::reinterpret_pointer_cast<Volt::AliasState>(newState);
				for (const auto& transitionNode : stateNode["TransitionFromStates"])
				{
					aliasState->transitionFromStates.emplace_back(transitionNode.as<UUID64>());
				}
			}

		}

		for (const auto& transitionNode : rootNode["Transitions"])
		{
			UUID64 transitionId;
			UUID64 transitionFromState;
			UUID64 transitionToState;

			VT_DESERIALIZE_PROPERTY(id, transitionId, transitionNode, UUID64(0));
			VT_DESERIALIZE_PROPERTY(fromState, transitionFromState, transitionNode, UUID64(0));
			VT_DESERIALIZE_PROPERTY(toState, transitionToState, transitionNode, UUID64(0));

			auto newTransition = myStateMachine->CreateTransition(transitionId);
			newTransition->fromState = transitionFromState;
			newTransition->toState = transitionToState;

			VT_DESERIALIZE_PROPERTY(shouldBlend, newTransition->shouldBlend, transitionNode, false);
			VT_DESERIALIZE_PROPERTY(blendTime, newTransition->blendTime, transitionNode, 1.f);

			if (transitionNode["Graph"])
			{
				newTransition->transitionGraph = CreateRef<Volt::AnimationTransitionGraph>();
				newTransition->transitionGraph->SetStateMachine(myStateMachine.get());
				newTransition->transitionGraph->SetTransitionID(transitionId);

				Graph::Deserialize(std::reinterpret_pointer_cast<Graph>(newTransition->transitionGraph), transitionNode["Graph"]);
			}
		}

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

