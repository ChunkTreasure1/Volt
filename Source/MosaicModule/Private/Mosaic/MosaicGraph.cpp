#include "mcpch.h"
#include "Mosaic/MosaicGraph.h"

#include "Mosaic/NodeRegistry.h"
#include "Mosaic/MosaicNode.h"

namespace Mosaic
{
	MosaicGraph::MosaicGraph()
	{
	}

	MosaicGraph::~MosaicGraph()
	{
	}

	const std::string MosaicGraph::Compile() const
	{
		constexpr VoltGUID OUTPUT_GUID = "{343B2C0A-C4E3-41BB-8629-F9939795AC76}"_guid;

		for (auto& node : m_graph.GetNodes())
		{
			node.nodeData->Reset();
		}

		UUID64 outputId = 0;
		for (const auto& node : m_graph.GetNodes())
		{
			if (node.nodeData->GetGUID() == OUTPUT_GUID)
			{
				outputId = node.id;
				break;
			}
		}

		const auto& node = m_graph.GetNodeFromID(outputId);

		if (!node.nodeData)
		{
			return {};
		}

		std::string outShaderCode;
		node.nodeData->GetShaderCode(node, 0, outShaderCode);

		return outShaderCode;
	}

	Scope<MosaicGraph> MosaicGraph::CreateDefaultGraph()
	{
		Scope<MosaicGraph> graph = CreateScope<MosaicGraph>();

		constexpr VoltGUID Color4ConstantGUID = "{C032E7D5-D545-4DEF-8EC4-6A3980BC41B2}"_guid;
		constexpr VoltGUID PBROutputNodeGUID = "{343B2C0A-C4E3-41BB-8629-F9939795AC76}"_guid;

		auto colorConstantNode = graph->m_graph.AddNode(NodeRegistry::CreateNode(Color4ConstantGUID, graph.get()));
		auto pbrOutputNode = graph->m_graph.AddNode(NodeRegistry::CreateNode(PBROutputNodeGUID, graph.get()));

		graph->m_graph.LinkNodes(colorConstantNode, pbrOutputNode, CreateRef<MosaicEdge>(0, 0));

		return graph;
	}

	void MosaicGraph::AddNode(const UUID64 uuid, const VoltGUID guid)
	{
		m_graph.AddNode(uuid, NodeRegistry::CreateNode(guid, this));
	}

	void MosaicGraph::AddNode(const VoltGUID guid)
	{
		m_graph.AddNode(NodeRegistry::CreateNode(guid, this));
	}

	uint32_t MosaicGraph::GetNextVariableIndex()
	{
		return m_currentVariableCount++;
	}
	
	uint32_t MosaicGraph::GetNextTextureIndex()
	{
		m_textureCount++;

		if (!m_availiableTextureIndices.empty())
		{
			const uint32_t index = m_availiableTextureIndices.back();
			m_availiableTextureIndices.pop_back();

			return index;
		}

		return m_currentTextureIndex++;
	}

	const std::string MosaicGraph::GetNextVariableName()
	{
		return "variable" + std::to_string(GetNextVariableIndex());
	}

	void MosaicGraph::ForfeitTextureIndex(uint32_t textureIndex)
	{
		m_availiableTextureIndices.emplace_back(textureIndex);
		m_textureCount--;
	}
}
