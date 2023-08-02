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
					VT_SERIALIZE_PROPERTY(isEntry, state->isEntry, out);
					VT_SERIALIZE_PROPERTY(isAny, state->isAny, out);
					VT_SERIALIZE_PROPERTY(editorState, state->editorState, out);

					VT_SERIALIZE_PROPERTY(id, state->id, out);
					VT_SERIALIZE_PROPERTY(pinId, state->pinId, out);
					VT_SERIALIZE_PROPERTY(pinId2, state->pinId2, out);

					out << YAML::Key << "Transitions" << YAML::BeginSeq;
					for (const auto& transition : state->transitions)
					{
						out << transition;
					}
					out << YAML::EndSeq;

					if (state->stateGraph)
					{
						VT_SERIALIZE_PROPERTY(skeletonHandle, state->stateGraph->GetSkeletonHandle(), out);
						Graph::Serialize(state->stateGraph, out);
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
			bool stateIsEntry;
			Volt::UUID stateId = Volt::UUID(0);

			VT_DESERIALIZE_PROPERTY(name, stateName, stateNode, std::string("Null"));
			VT_DESERIALIZE_PROPERTY(isEntry, stateIsEntry, stateNode, false);
			VT_DESERIALIZE_PROPERTY(id, stateId, stateNode, Volt::UUID(0));

			auto newState = myStateMachine->CreateState(stateName, stateIsEntry, stateId);

			VT_DESERIALIZE_PROPERTY(editorState, newState->editorState, stateNode, std::string(""));
			VT_DESERIALIZE_PROPERTY(pinId, newState->pinId, stateNode, Volt::UUID(0));
			VT_DESERIALIZE_PROPERTY(pinId2, newState->pinId2, stateNode, Volt::UUID(0));
			VT_DESERIALIZE_PROPERTY(isAny, newState->isAny, stateNode, false);

			for (const auto& transitionNode : stateNode["Transitions"])
			{
				newState->transitions.emplace_back(transitionNode.as<Volt::UUID>());
			}

			if (stateNode["Graph"])
			{
				Volt::AssetHandle characterHandle;
				VT_DESERIALIZE_PROPERTY(characterHandle, characterHandle, stateNode, Volt::AssetHandle(0));
				newState->stateGraph = CreateRef<Volt::AnimationGraphAsset>(characterHandle);
				Graph::Deserialize(newState->stateGraph, stateNode["Graph"]);
			}
		}

		for (const auto& transitionNode : rootNode["Transitions"])
		{
			Volt::UUID transitionId;
			Volt::UUID transitionFromState;
			Volt::UUID transitionToState;

			VT_DESERIALIZE_PROPERTY(id, transitionId, transitionNode, Volt::UUID(0));
			VT_DESERIALIZE_PROPERTY(fromState, transitionFromState, transitionNode, Volt::UUID(0));
			VT_DESERIALIZE_PROPERTY(toState, transitionToState, transitionNode, Volt::UUID(0));

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

		bool hasAnyState = false;
		bool hasEntryState = false;

		for (const auto& state : myStateMachine->GetStates())
		{
			hasAnyState |= state->isAny;
			hasEntryState |= state->isEntry;

			if (hasEntryState && hasAnyState)
			{
				break;
			}
		}

		if (!hasAnyState)
		{
			myStateMachine->AddState("Any", false, true);
		}

		if (!hasEntryState)
		{
			myStateMachine->AddState("Entry", true);
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

	Ref<Node> StateMachineNode::CreateCopy(Graph* ownerGraph, Wire::EntityId entity)
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

