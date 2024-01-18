#include "mcpch.h"
#include "Testing.h"

#include "Nodes/ConstantNodes.h"
#include "Nodes/MathNodes.h"
#include "Nodes/TextureNodes.h"
#include "Nodes/PBROutputNode.h"

#include <glm/glm.hpp>

namespace Mosaic
{
	TestClass::TestClass()
	{
		Ref<MosaicGraph> mosaicGraph = CreateRef<MosaicGraph>();

		auto constantNode = mosaicGraph->m_graph.AddNode(CreateRef<ConstantNode<glm::vec4, ValueBaseType::Float, 4, "{75DD7D2C-3B74-48C4-9F5C-062EBF53F44A}"_guid>>(mosaicGraph.get()));
		auto addNode = mosaicGraph->m_graph.AddNode(CreateRef<AddNode>(mosaicGraph.get()));
		auto sampleTextureNode = mosaicGraph->m_graph.AddNode(CreateRef<SampleTextureNode>(mosaicGraph.get()));

		auto outputNode = mosaicGraph->m_graph.AddNode(CreateRef<PBROutputNode>(mosaicGraph.get()));

		mosaicGraph->m_graph.LinkNodes(constantNode, addNode, CreateRef<MosaicEdge>(0, 0));
		mosaicGraph->m_graph.LinkNodes(addNode, outputNode, CreateRef<MosaicEdge>(1, 0));
		mosaicGraph->m_graph.LinkNodes(sampleTextureNode, outputNode, CreateRef<MosaicEdge>(0, 5));

		const auto& n = mosaicGraph->m_graph.GetNodeFromID(outputNode);

		std::string output;
		n.nodeData->GetShaderCode(n, 0, output);
	}
}
