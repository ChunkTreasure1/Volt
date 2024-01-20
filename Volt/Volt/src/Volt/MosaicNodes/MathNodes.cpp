#include "vtpch.h"
#include "MathNodes.h"

#include <Mosaic/MosaicGraph.h>
#include <Mosaic/MosaicHelpers.h>
#include <Mosaic/NodeRegistry.h>

namespace Volt
{
	AddNode::AddNode(Mosaic::MosaicGraph* ownerGraph)
		: Mosaic::MosaicNode(ownerGraph)
	{
		AddInputParameter("A", Mosaic::ValueBaseType::Dynamic, 1);
		AddInputParameter("B", Mosaic::ValueBaseType::Dynamic, 1);

		AddOutputParameter("", Mosaic::ValueBaseType::Dynamic, 1);
	}

	const Mosaic::ResultInfo AddNode::GetShaderCode(const GraphNode<Ref<class Mosaic::MosaicNode>, Ref<Mosaic::MosaicEdge>>& underlyingNode, uint32_t outputIndex, std::string& appendableShaderString) const
	{
		constexpr const char* nodeStr = "const {0} {1} = Add({2}, {3}); \n";
		constexpr Mosaic::TypeInfo DEFAULT_PARAM_TYPEINFO{ Mosaic::ValueBaseType::Float, 1 };

		std::string A = std::to_string(GetInputParameter(0).Get<float>());
		std::string B = std::to_string(GetInputParameter(1).Get<float>());

		Mosaic::TypeInfo AInfo = DEFAULT_PARAM_TYPEINFO;
		Mosaic::TypeInfo BInfo = DEFAULT_PARAM_TYPEINFO;

		for (const auto& edgeId : underlyingNode.GetInputEdges())
		{
			const auto edge = underlyingNode.GetEdgeFromID(edgeId);
			const uint32_t paramIndex = edge.metaDataType->GetParameterInputIndex();
		
			const auto& node = underlyingNode.GetNodeFromID(edge.startNode);
			const Mosaic::ResultInfo info = node.nodeData->GetShaderCode(node, edge.metaDataType->GetParameterOutputIndex(), appendableShaderString);

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
		
		const Mosaic::TypeInfo resultType = Mosaic::Helpers::GetPromotedTypeInfo(AInfo, BInfo);

		std::string result = std::format(nodeStr, Mosaic::Helpers::GetTypeNameFromTypeInfo(resultType), varName, A, B);
		appendableShaderString.append(result);

		Mosaic::ResultInfo resultInfo{};
		resultInfo.resultParamName = varName;
		resultInfo.resultType = resultType;

		return resultInfo;
	}

	REGISTER_NODE(AddNode);
}
