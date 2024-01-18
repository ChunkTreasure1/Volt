#include "vtpch.h"
#include "MosaicGraphImporter.h"

#include "Volt/Asset/Rendering/Material.h"

#include "Volt/Asset/AssetManager.h"

#include <Mosaic/MosaicGraph.h>
#include <Mosaic/MosaicNode.h>
#include <Mosaic/NodeRegistry.h>

#include <Volt/Utility/FileIO/YAMLStreamReader.h>
#include <Volt/Utility/FileIO/YAMLStreamWriter.h>

namespace Volt
{
	bool MosaicGraphImporter::Load(const AssetMetadata& metadata, Ref<Asset>& asset) const
	{
		asset = CreateRef<Material>();
		Ref<Material> mosaicAsset = std::reinterpret_pointer_cast<Material>(asset);

		const auto filePath = AssetManager::GetFilesystemPath(metadata.filePath);

		if (!std::filesystem::exists(filePath))
		{
			VT_CORE_ERROR("File {0} not found!", metadata.filePath);
			asset->SetFlag(AssetFlag::Missing, true);
			return false;
		}

		YAMLStreamReader streamReader{};

		if (!streamReader.OpenFile(filePath))
		{
			VT_CORE_ERROR("Failed to open file {0}!", metadata.filePath);
			asset->SetFlag(AssetFlag::Invalid, true);
			return false;
		}

		mosaicAsset->m_graph = CreateScope<Mosaic::MosaicGraph>();

		streamReader.EnterScope("MosaicGraph");
		
		mosaicAsset->m_graph->GetEditorState() = streamReader.ReadKey("state", std::string(""));
		
		auto& underlyingGraph = mosaicAsset->m_graph->GetUnderlyingGraph();

		streamReader.ForEach("Nodes", [&]() 
		{
			const UUID64 nodeId = streamReader.ReadKey("id", UUID64(0));
			const VoltGUID guid = streamReader.ReadKey("guid", VoltGUID::Null());
			const std::string state = streamReader.ReadKey("state", std::string());

			mosaicAsset->m_graph->AddNode(nodeId, guid);
			underlyingGraph.GetNodeFromID(nodeId).nodeData->GetEditorState() = state;
		});

		streamReader.ForEach("Edges", [&]() 
		{
			const UUID64 edgeId = streamReader.ReadKey("id", UUID64(0));
			const UUID64 startNodeId = streamReader.ReadKey("startNode", UUID64(0));
			const UUID64 endNodeId = streamReader.ReadKey("endNode", UUID64(0));
			const uint32_t inputIndex = streamReader.ReadKey("inputIndex", uint32_t(0));
			const uint32_t outputIndex = streamReader.ReadKey("outputIndex", uint32_t(0));
			
			underlyingGraph.LinkNodes(edgeId, startNodeId, endNodeId, CreateRef<Mosaic::MosaicEdge>(inputIndex, outputIndex));
		});

		streamReader.ExitScope();
		return true;
	}

	void MosaicGraphImporter::Save(const AssetMetadata& metadata, const Ref<Asset>& asset) const
	{
		const Ref<Material> mosaicAsset = std::reinterpret_pointer_cast<Material>(asset);
		const auto graph = mosaicAsset->GetGraph();

		YAMLStreamWriter streamWriter{ AssetManager::GetFilesystemPath(metadata.filePath) };
		streamWriter.BeginMap();
		streamWriter.BeginMapNamned("MosaicGraph");
		streamWriter.SetKey("state", graph.GetEditorState());

		streamWriter.BeginSequence("Nodes");
		for (const auto& node : graph.GetUnderlyingGraph().GetNodes())
		{
			streamWriter.BeginMap();
			streamWriter.SetKey("id", node.id);
			streamWriter.SetKey("guid", node.nodeData->GetGUID());
			streamWriter.SetKey("state", node.nodeData->GetEditorState());

			streamWriter.EndMap();
		}
		streamWriter.EndSequence();

		streamWriter.BeginSequence("Edges");
		for (const auto& edge : graph.GetUnderlyingGraph().GetEdges())
		{
			streamWriter.BeginMap();
			streamWriter.SetKey("id", edge.id);
			streamWriter.SetKey("startNode", edge.startNode);
			streamWriter.SetKey("endNode", edge.endNode);
			streamWriter.SetKey("inputIndex", edge.metaDataType->GetParameterInputIndex());
			streamWriter.SetKey("outputIndex", edge.metaDataType->GetParameterOutputIndex());
			streamWriter.EndMap();
		}
		streamWriter.EndSequence();

		streamWriter.EndMap();
		streamWriter.EndMap();

		streamWriter.WriteToDisk();
	}
}
