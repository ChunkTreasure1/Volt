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

		inline void SetStateMachine(const AnimationStateMachine* aStateMachine) { myStateMachineWeak = aStateMachine; }
		inline const AnimationStateMachine* GetStateMachine() const { return myStateMachineWeak; }

		inline void SetTransitionID(const UUID& aId) { myTransitionID = aId; }
		inline const UUID& GetTransitionID() const { return myTransitionID; }
	private:
		std::string myGraphState;
		const AnimationStateMachine* myStateMachineWeak;
		UUID myTransitionID;
	};
}
