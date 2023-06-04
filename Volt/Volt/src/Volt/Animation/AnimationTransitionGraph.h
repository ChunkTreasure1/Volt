#pragma once

#include <GraphKey/Graph.h>

namespace Volt
{
	class AnimationTransitionGraph : public GraphKey::Graph
	{
	public:
		inline AnimationTransitionGraph() = default;
		inline ~AnimationTransitionGraph() override = default;

		inline void SetState(const std::string& state) { myGraphState = state; }
		inline const std::string& GetState() const { return myGraphState; }

	private:
		std::string myGraphState;
	};
}