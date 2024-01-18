#include "mcpch.h"
#include "MathNodes.h"

#include "Mosaic/MosaicGraph.h"
#include "Mosaic/MosaicHelpers.h"
#include "Mosaic/NodeRegistry.h"

namespace Mosaic
{
	AddNode::AddNode(MosaicGraph* ownerGraph)
		: MosaicNode(ownerGraph)
	{
		AddInputParameter("A", ValueBaseType::Dynamic, 1);
		AddInputParameter("B", ValueBaseType::Dynamic, 1);

		AddOutputParameter("", ValueBaseType::Dynamic, 1);
	}

	const ResultInfo AddNode::GetShaderCode(const GraphNode<Ref<class MosaicNode>, Ref<MosaicEdge>>& underlyingNode, uint32_t outputIndex, std::string& appendableShaderString) const
	{
		constexpr const char* nodeStr = "const {0} {1} = Add({2}, {3}); \n";
		constexpr TypeInfo DEFAULT_PARAM_TYPEINFO{ ValueBaseType::Float, 1 };

		std::string A = std::to_string(GetInputParameter(0).Get<float>());
		std::string B = std::to_string(GetInputParameter(1).Get<float>());

		TypeInfo AInfo = DEFAULT_PARAM_TYPEINFO;
		TypeInfo BInfo = DEFAULT_PARAM_TYPEINFO;

		for (const auto& edgeId : underlyingNode.GetInputEdges())
		{
			const auto edge = underlyingNode.GetEdgeFromID(edgeId);
			const uint32_t paramIndex = edge.metaDataType->GetParameterInputIndex();
		
			const auto& node = underlyingNode.GetNodeFromID(edge.startNode);
			const ResultInfo info = node.nodeData->GetShaderCode(node, edge.metaDataType->GetParameterOutputIndex(), appendableShaderString);

			if (paramIndex == 0)
			{
				A = info.resultParamName;
				AInfo = info.resultType;
			}
			else if (paramIndex == 1)
			{
				B = info.resultParamName;
				BInfo = info.resultType;
			}
		}

		const std::string varName = m_graph->GetNextVariableName();
		
		const TypeInfo resultType = Helpers::GetPromotedTypeInfo(AInfo, BInfo);

		std::string result = std::format(nodeStr, Helpers::GetTypeNameFromTypeInfo(resultType), varName, A, B);
		appendableShaderString.append(result);

		ResultInfo resultInfo{};
		resultInfo.resultParamName = varName;
		resultInfo.resultType = resultType;

		return resultInfo;
	}

	REGISTER_NODE(AddNode);
}
