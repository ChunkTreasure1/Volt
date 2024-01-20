#include "vtpch.h"
#include "TextureNodes.h"

#include <Mosaic/MosaicGraph.h>
#include <Mosaic/NodeRegistry.h>

namespace Volt
{
	static void GetCorrectedVariableName(Mosaic::ResultInfo& resultInfo, uint32_t outputIndex)
	{
		if (outputIndex == 0)
		{
			resultInfo.resultParamName += ".rgb";
			resultInfo.resultType.vectorSize = 3;
		}
		else if (outputIndex == 1)
		{
			resultInfo.resultParamName += ".r";
		}
		else if (outputIndex == 2)
		{
			resultInfo.resultParamName += ".g";
		}
		else if (outputIndex == 3)
		{
			resultInfo.resultParamName += ".b";
		}
		else if (outputIndex == 4)
		{
			resultInfo.resultParamName += ".a";
		}
		else if (outputIndex == 5)
		{
			resultInfo.resultType.vectorSize = 4;
		}
	}

	SampleTextureNode::SampleTextureNode(Mosaic::MosaicGraph* ownerGraph)
		: Mosaic::MosaicNode(ownerGraph)
	{
		AddInputParameter("UV", Mosaic::ValueBaseType::Float, 2);
	
		AddOutputParameter("RGB", Mosaic::ValueBaseType::Float, 3);
		AddOutputParameter("R", Mosaic::ValueBaseType::Float, 1);
		AddOutputParameter("G", Mosaic::ValueBaseType::Float, 1);
		AddOutputParameter("B", Mosaic::ValueBaseType::Float, 1);
		AddOutputParameter("A", Mosaic::ValueBaseType::Float, 1);
		AddOutputParameter("RGBA", Mosaic::ValueBaseType::Float, 4);
	
		if (m_graph)
		{
			m_textureIndex = m_graph->GetNextTextureIndex();
		}
	}

	SampleTextureNode::~SampleTextureNode()
	{
		if (m_graph)
		{
			m_graph->ForfeitTextureIndex(m_textureIndex);
		}
	}

	void SampleTextureNode::Reset()
	{
		m_evaluated = false;
	}

	const Mosaic::ResultInfo SampleTextureNode::GetShaderCode(const GraphNode<Ref<class Mosaic::MosaicNode>, Ref<Mosaic::MosaicEdge>>& underlyingNode, uint32_t outputIndex, std::string& appendableShaderString) const
	{
		constexpr const char* nodeStr = "SamplerState {0} = material.samplers[{1}]; \n"
										"TextureT<float4> {2} = material.textures[{3}]; \n"
										"const float4 {4} = {5}.SampleGrad2D({6}, {7}, texCoordsDX, texCoordsDY); \n";

		if (m_evaluated)
		{
			Mosaic::ResultInfo tempInfo = m_evaluatedResultInfo;
			GetCorrectedVariableName(tempInfo, outputIndex);

			return tempInfo;
		}

		const std::string samplerVarName = m_graph->GetNextVariableName();
		const std::string textureVarName = m_graph->GetNextVariableName();
		const std::string valueVarName = m_graph->GetNextVariableName();

		const uint32_t index = m_textureIndex;

		const bool hasUVInput = !underlyingNode.GetInputEdges().empty();

		std::string texCoordsVarName = "texCoords";
		if (hasUVInput)
		{
			const auto& uvEdge = underlyingNode.GetEdgeFromID(underlyingNode.GetInputEdges().front());
			const auto& uvNode = underlyingNode.GetNodeFromID(uvEdge.startNode);

			const Mosaic::ResultInfo info = uvNode.nodeData->GetShaderCode(uvNode, uvEdge.metaDataType->GetParameterOutputIndex(), appendableShaderString);
			texCoordsVarName = info.resultParamName;
		}

		std::string result = std::format(nodeStr, samplerVarName, index, textureVarName, index, valueVarName, textureVarName, samplerVarName, texCoordsVarName);
		appendableShaderString.append(result);

		Mosaic::ResultInfo resultInfo{};
		resultInfo.resultParamName = valueVarName;
		resultInfo.resultType = Mosaic::TypeInfo{ Mosaic::ValueBaseType::Float, 1 };

		const_cast<bool&>(m_evaluated) = true;
		const_cast<Mosaic::ResultInfo&>(m_evaluatedResultInfo) = resultInfo;

		GetCorrectedVariableName(resultInfo, outputIndex);

		return resultInfo;
	}

	REGISTER_NODE(SampleTextureNode);
}
