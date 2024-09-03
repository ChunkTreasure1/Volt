#pragma once

#include <GraphKey/Graph.h>
#include <Volt/Core/Base.h>

namespace Volt
{
	class AnimationStateMachine;
	class AnimationTransitionGraph : public GraphKey::Graph
	{
	public:
		inline AnimationTransitionGraph() = default;
		inline ~AnimationTransitionGraph() override = default;

		inline void SetState(const std::string& aState) { myGraphState = aState; }
		inline const std::string& GetState() const { return myGraphState; }

		inline void SetStateMachine(AnimationStateMachine* aStateMachine) { myStateMachineWeak = aStateMachine; }
		inline const AnimationStateMachine* GetStateMachine() const { return myStateMachineWeak; }

		inline void SetTransitionID(const UUID64& aId) { myTransitionID = aId; }
		inline const UUID64& GetTransitionID() const { return myTransitionID; }
	private:
		std::string myGraphState;
		AnimationStateMachine* myStateMachineWeak;
		UUID64 myTransitionID;
	};
}
