#include "vtpch.h"
#include "TextureNodes.h"

#include "Volt/Asset/AssetManager.h"
#include "Volt/RenderingNew/RendererNew.h"
#include "Volt/Utility/UIUtility.h"

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
		AddInputParameter("UV", Mosaic::ValueBaseType::Float, 2, false);
	
		AddOutputParameter("RGB", Mosaic::ValueBaseType::Float, 3, false);
		AddOutputParameter("R", Mosaic::ValueBaseType::Float, 1, false);
		AddOutputParameter("G", Mosaic::ValueBaseType::Float, 1, false);
		AddOutputParameter("B", Mosaic::ValueBaseType::Float, 1, false);
		AddOutputParameter("A", Mosaic::ValueBaseType::Float, 1, false);
		AddOutputParameter("RGBA", Mosaic::ValueBaseType::Float, 4, false);
	
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

	void SampleTextureNode::RenderCustomWidget()
	{
		std::string assetFileName = "Null";

		const Ref<Volt::Asset> rawAsset = Volt::AssetManager::Get().GetAssetRaw(m_textureHandle);
		if (rawAsset)
		{
			assetFileName = rawAsset->assetName;
		}

		const ImVec2 width = ImGui::CalcTextSize(assetFileName.c_str());
		ImGui::PushItemWidth(std::max(width.x, 20.f) + 5.f);

		const std::string id = "##" + std::to_string(UI::GetID());
		ImGui::InputTextString(id.c_str(), &assetFileName, ImGuiInputTextFlags_ReadOnly);
		ImGui::PopItemWidth();

		if (auto ptr = UI::DragDropTarget("ASSET_BROWSER_ITEM"))
		{
			Volt::AssetHandle newHandle = *(Volt::AssetHandle*)ptr;
			m_textureHandle = newHandle;
		}
	}

	void SampleTextureNode::SerializeCustom(YAMLStreamWriter& streamWriter) const
	{
		streamWriter.SetKey("textureHandle", m_textureHandle);
	}

	void SampleTextureNode::DeserializeCustom(YAMLStreamReader& streamReader)
	{
		m_textureHandle = streamReader.ReadAtKey("textureHandle", Asset::Null());
	}

	const Mosaic::ResultInfo SampleTextureNode::GetShaderCode(const GraphNode<Ref<class Mosaic::MosaicNode>, Ref<Mosaic::MosaicEdge>>& underlyingNode, uint32_t outputIndex, std::string& appendableShaderString) const
	{
		constexpr const char* nodeStr = "TextureSampler {0} = material.samplers[{1}]; \n"
										"TTexture<float4> {2} = material.textures[{3}]; \n"
										"const float4 {4} = {5}.SampleGrad2D({6}, {7}, evalData.texCoordsDX, evalData.texCoordsDY); \n";
										//"const float4 {6} = {7}.SampleLevel2D({8}, {9}, 0.f); \n";

		if (m_evaluated)
		{
			Mosaic::ResultInfo tempInfo = m_evaluatedResultInfo;
			GetCorrectedVariableName(tempInfo, outputIndex);

			return tempInfo;
		}

		const std::string texSamplerVarName = m_graph->GetNextVariableName();
		const std::string textureVarName = m_graph->GetNextVariableName();
		const std::string valueVarName = m_graph->GetNextVariableName();

		const uint32_t index = m_textureIndex;

		const bool hasUVInput = !underlyingNode.GetInputEdges().empty();

		std::string texCoordsVarName = "evalData.texCoords";
		if (hasUVInput)
		{
			const auto& uvEdge = underlyingNode.GetEdgeFromID(underlyingNode.GetInputEdges().front());
			const auto& uvNode = underlyingNode.GetNodeFromID(uvEdge.startNode);

			const Mosaic::ResultInfo info = uvNode.nodeData->GetShaderCode(uvNode, uvEdge.metaDataType->GetParameterOutputIndex(), appendableShaderString);
			texCoordsVarName = info.resultParamName;
		}

		std::string result = std::format(nodeStr, texSamplerVarName, index, textureVarName, index, valueVarName, textureVarName, texSamplerVarName, texCoordsVarName);
		appendableShaderString.append(result);

		Mosaic::ResultInfo resultInfo{};
		resultInfo.resultParamName = valueVarName;
		resultInfo.resultType = Mosaic::TypeInfo{ Mosaic::ValueBaseType::Float, 1 };

		m_evaluated = true;
		m_evaluatedResultInfo = resultInfo;

		GetCorrectedVariableName(resultInfo, outputIndex);

		return resultInfo;
	}

	const TextureInfo SampleTextureNode::GetTextureInfo() const
	{
		return { m_textureIndex, m_textureHandle };
	}

	REGISTER_NODE(SampleTextureNode);
}
