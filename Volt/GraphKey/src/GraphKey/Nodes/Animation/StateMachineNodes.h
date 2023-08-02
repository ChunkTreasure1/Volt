#pragma once

#include "GraphKey/Node.h"
#include <Volt/Animation/AnimationStateMachine.h>

namespace GraphKey
{
	class StateMachineNode : public Node
	{
	public:
		StateMachineNode();
		~StateMachineNode() override = default;

		void OnEvent(Volt::Event& e) override;
		void Initialize() override;

		void Serialize(YAML::Emitter& out) override;
		void Deserialize(const YAML::Node& node) override;

		const std::string GetName() override;
		Ref<Node> CreateCopy(Graph* ownerGraph, Wire::EntityId entity = 0) override;

		inline const glm::vec4 GetColor() override { return { 1.f }; }
		inline const Ref<Volt::AnimationStateMachine> GetStateMachine() const { return myStateMachine; }

	private:
		Ref<Volt::AnimationStateMachine> myStateMachine;
		void SampleStateMachine();
	};
	
}
