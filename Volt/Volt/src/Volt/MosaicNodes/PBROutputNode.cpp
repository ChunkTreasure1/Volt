#include "vtpch.h"
#include "PBROutputNode.h"

#include <Mosaic/FormatterExtension.h>
#include <Mosaic/NodeRegistry.h>

#include <glm/glm.hpp>

namespace Volt::MosaicNodes
{
	PBROutputNode::PBROutputNode(Mosaic::MosaicGraph* ownerGraph)
		: Mosaic::MosaicNode(ownerGraph)
	{
		AddInputParameter("Base Color", Mosaic::ValueBaseType::Float, 4, 1.f, false);
		AddInputParameter("Metallic", Mosaic::ValueBaseType::Float, 1, 0.f, false);
		AddInputParameter("Roughness", Mosaic::ValueBaseType::Float, 1, 0.9f, false);
		AddInputParameter("Normal", Mosaic::ValueBaseType::Float, 3, glm::vec3(0.5f, 0.5f, 1.f), false);
	}

	const Mosaic::ResultInfo PBROutputNode::GetShaderCode(const GraphNode<Ref<class Mosaic::MosaicNode>, Ref<Mosaic::MosaicEdge>>& underlyingNode, uint32_t outputIndex, std::string& appendableShaderString) const
	{
		constexpr const char* nodeStr = "EvaluatedMaterial materialResult;\n"
										"materialResult.Setup();\n"
										"materialResult.albedo = {0};\n"
										"materialResult.metallic = {1};\n"
										"materialResult.roughness = {2};\n"
										"materialResult.normal = {3};\n"
										"materialResult.emissive = {4};\n\n"
			
										"return materialResult;\n";

		std::string paramStrings[4];

		paramStrings[0] = std::format("{}", GetInputParameter(0).Get<glm::vec4>());
		paramStrings[1] = std::format("{}", GetInputParameter(1).Get<float>());
		paramStrings[2] = std::format("{}", GetInputParameter(2).Get<float>());
		paramStrings[3] = std::format("{}", GetInputParameter(3).Get<glm::vec3>());

		for (const auto& edgeId : underlyingNode.GetInputEdges())
		{
			const auto& edge = underlyingNode.GetEdgeFromID(edgeId);
			const uint32_t paramIndex = edge.metaDataType->GetParameterInputIndex();

			const auto& node = underlyingNode.GetNodeFromID(edge.startNode);
			const Mosaic::ResultInfo info = node.nodeData->GetShaderCode(node, edge.metaDataType->GetParameterOutputIndex(), appendableShaderString);
		
			paramStrings[paramIndex] = info.resultParamName;
		}

		std::string result = std::format(nodeStr, paramStrings[0], paramStrings[1], paramStrings[2], paramStrings[3], glm::vec3(0.f));
		appendableShaderString.append(result);

		Mosaic::ResultInfo resultInfo{};
		resultInfo.resultParamName = "";

		return resultInfo;
	}

	REGISTER_NODE(PBROutputNode);
}
