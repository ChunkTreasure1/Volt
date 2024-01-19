#include "mcpch.h"
#include "PBROutputNode.h"

#include "Mosaic/FormatterExtension.h"
#include "Mosaic/NodeRegistry.h"

#include <glm/glm.hpp>

namespace Mosaic
{
	PBROutputNode::PBROutputNode(MosaicGraph* ownerGraph)
		: MosaicNode(ownerGraph)
	{
		AddInputParameter("Base Color", ValueBaseType::Float, 4);
		AddInputParameter("Roughness", ValueBaseType::Float, 1);
		AddInputParameter("Metallic", ValueBaseType::Float, 1);
		AddInputParameter("Normal", ValueBaseType::Float, 3);
	}

	const ResultInfo PBROutputNode::GetShaderCode(const GraphNode<Ref<class MosaicNode>, Ref<MosaicEdge>>& underlyingNode, uint32_t outputIndex, std::string& appendableShaderString) const
	{
		constexpr const char* nodeStr = "EvaluatedMaterial materialResult;\n"
										"materialResult.Setup();\n"
										"materialResult.albedo = {0};\n"
										//"materialResult.materialEmissive = float4({1}, {2}, {3}, {4});\n"
										//"materialResult.normalEmissive = float4({5}, {6});\n"
										"return materialResult;\n";

		std::string paramStrings[4];

		paramStrings[0] = std::format("{}", GetInputParameter(0).Get<glm::vec4>());
		paramStrings[1] = std::format("{}", GetInputParameter(1).Get<float>());
		paramStrings[2] = std::format("{}", GetInputParameter(2).Get<float>());
		paramStrings[3] = std::format("{}", GetInputParameter(3).Get<glm::vec3>());

		for (const auto& edgeId : underlyingNode.GetInputEdges())
		{
			const auto edge = underlyingNode.GetEdgeFromID(edgeId);
			const uint32_t paramIndex = edge.metaDataType->GetParameterInputIndex();

			const auto& node = underlyingNode.GetNodeFromID(edge.startNode);
			const ResultInfo info = node.nodeData->GetShaderCode(node, edge.metaDataType->GetParameterOutputIndex(), appendableShaderString);
		
			paramStrings[paramIndex] = info.resultParamName;
		}

		std::string result = std::format(nodeStr, paramStrings[0], paramStrings[1], paramStrings[2], 0.f, 0.f, paramStrings[3], 0.f);
		appendableShaderString.append(result);

		ResultInfo resultInfo{};
		resultInfo.resultParamName = "";

		return resultInfo;
	}

	REGISTER_NODE(PBROutputNode);
}
