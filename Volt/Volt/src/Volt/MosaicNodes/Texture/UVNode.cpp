#include "vtpch.h"
#include "UVNode.h"

#include <Mosaic/MosaicGraph.h>
#include <Mosaic/NodeRegistry.h>

namespace Volt::MosaicNodes
{
	UVNode::UVNode(Mosaic::MosaicGraph* ownerGraph)
		: Mosaic::MosaicNode(ownerGraph)
	{
		AddOutputParameter("UV", Mosaic::ValueBaseType::Float, 2, false);
	}

	UVNode::~UVNode()
	{
	}

	void UVNode::Reset()
	{
		m_evaluated = false;
	}

	const Mosaic::ResultInfo UVNode::GetShaderCode(const GraphNode<Ref<class Mosaic::MosaicNode>, Ref<Mosaic::MosaicEdge>>& underlyingNode, uint32_t outputIndex, std::string& appendableShaderString) const
	{
		constexpr const char* nodeStr = "const float2 {0} = evalData.texCoords; \n";

		if (m_evaluated)
		{
			return m_evaluatedResultInfo;
		}

		const std::string varName = m_graph->GetNextVariableName();

		std::string result = std::format(nodeStr, varName);
		appendableShaderString.append(result);

		Mosaic::ResultInfo resultInfo{};
		resultInfo.resultParamName = varName;
		resultInfo.resultType = Mosaic::TypeInfo{ Mosaic::ValueBaseType::Float, 2 };

		m_evaluated = true;
		m_evaluatedResultInfo = resultInfo;

		return resultInfo;
	}

	REGISTER_NODE(UVNode);
}
