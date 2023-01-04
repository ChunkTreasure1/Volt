#pragma once

#include "Volt/Asset/Animation/Animation.h"

namespace Volt
{

	namespace AnimationGraph
	{
		class Node;
		class Graph
		{
		public:
			const std::vector<Animation::TRS> Sample() const;

		private:
			std::vector<Node> myNodes;
		};
	}

}