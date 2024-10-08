#include "vtpch.h"
#include "Volt/MosaicNodes/Texture/SampleTextureNode.h"

#include <AssetSystem/AssetManager.h>
#include "Volt/Rendering/Renderer.h"
#include "Volt/Utility/UIUtility.h"

#include <Mosaic/MosaicGraph.h>
#include <Mosaic/NodeRegistry.h>

namespace Volt::MosaicNodes
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
		AddInputParameter("Tiling", Mosaic::ValueBaseType::Float, 2, glm::vec2(1.f), false);
	
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
		constexpr const char* nodeStr = "vt::TextureSampler {} = material.samplers[{}]; \n"
										"vt::Tex2D<float4> {} = material.textures[{}]; \n"
										"const float2 {} = {}; \n"
										"const float4 {} = {}.SampleGrad({}, {} * {}, evalData.texCoordsDX * {}.x, evalData.texCoordsDY * {}.y); \n";

		if (m_evaluated)
		{
			Mosaic::ResultInfo tempInfo = m_evaluatedResultInfo;
			GetCorrectedVariableName(tempInfo, outputIndex);

			return tempInfo;
		}

		const std::string texSamplerVarName = m_graph->GetNextVariableName();
		const std::string textureVarName = m_graph->GetNextVariableName();
		const std::string valueVarName = m_graph->GetNextVariableName();
		const std::string tilingVarName = m_graph->GetNextVariableName();

		std::string texCoordsVarName = "evalData.texCoords";
		std::string tilingParamString = std::format("{}", GetInputParameter(1).Get<glm::vec2>());

		const uint32_t index = m_textureIndex;

		for (const auto& edgeId : underlyingNode.GetInputEdges())
		{
			const auto& edge = underlyingNode.GetEdgeFromID(edgeId);
			const uint32_t paramIndex = edge.metaDataType->GetParameterInputIndex();
			const auto& node = underlyingNode.GetNodeFromID(edge.startNode);

			const Mosaic::ResultInfo info = node.nodeData->GetShaderCode(node, edge.metaDataType->GetParameterOutputIndex(), appendableShaderString);

			// UV
			if (paramIndex == 0)
			{
				texCoordsVarName = info.resultParamName;
			}
			// Tiling
			else if (paramIndex == 1)
			{
				tilingParamString = info.resultParamName;
			}
		}


		std::string result = std::format(nodeStr, texSamplerVarName, index, textureVarName, index, tilingVarName, tilingParamString, valueVarName, textureVarName, texSamplerVarName, texCoordsVarName, tilingVarName, tilingVarName, tilingVarName);
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
