#include "mcpch.h"
#include "TestNode.h"

namespace Mosaic
{
	const std::string TestAddNode::GetShaderCode(const GraphNode<Ref<class MosaicNode>, Ref<MosaicEdge>>& underlyingNode) const
	{
		constexpr const char* nodeStr = "%v %r = Add(%v1, %v2);\n";

		//const auto& inputEdges = underlyingNode.GetInputEdges();
		std::string result = nodeStr;

		//for (const auto& edgeId : inputEdges)
		//{
		//	const auto& edge = underlyingNode.GetEdgeFromID(edgeId);
		//
		//	
		//}

		return result;
	}
}
