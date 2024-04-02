#include "vtpch.h"
#include "MosaicGraphImporter.h"

#include "Volt/Asset/Rendering/Material.h"

#include "Volt/Asset/AssetManager.h"
#include "Volt/Utility/YAMLSerializationHelpers.h"

#include <Mosaic/MosaicGraph.h>
#include <Mosaic/MosaicNode.h>
#include <Mosaic/NodeRegistry.h>

#include <CoreUtilities/FileIO/YAMLFileStreamWriter.h>
#include <CoreUtilities/FileIO/YAMLFileStreamReader.h>

namespace Volt
{
	bool MosaicGraphImporter::Load(const AssetMetadata& metadata, Ref<Asset>& asset) const
	{
		Ref<Material> mosaicAsset = std::reinterpret_pointer_cast<Material>(asset);

		const auto filePath = AssetManager::GetFilesystemPath(metadata.filePath);

		if (!std::filesystem::exists(filePath))
		{
			VT_CORE_ERROR("File {0} not found!", metadata.filePath);
			asset->SetFlag(AssetFlag::Missing, true);
			return false;
		}

		YAMLFileStreamReader streamReader{};

		if (!streamReader.OpenFile(filePath))
		{
			VT_CORE_ERROR("Failed to open file {0}!", metadata.filePath);
			asset->SetFlag(AssetFlag::Invalid, true);
			return false;
		}

		mosaicAsset->m_graph = CreateScope<Mosaic::MosaicGraph>();

		streamReader.EnterScope("MosaicGraph");
		
		mosaicAsset->m_materialGUID = streamReader.ReadAtKey("guid", VoltGUID::Null());
		mosaicAsset->m_graph->GetEditorState() = streamReader.ReadAtKey("state", std::string(""));
		
		auto& underlyingGraph = mosaicAsset->m_graph->GetUnderlyingGraph();

		streamReader.ForEach("Nodes", [&]() 
		{
			const UUID64 nodeId = streamReader.ReadAtKey("id", UUID64(0));
			const VoltGUID guid = streamReader.ReadAtKey("guid", VoltGUID::Null());
			const std::string state = streamReader.ReadAtKey("state", std::string());

			mosaicAsset->m_graph->AddNode(nodeId, guid); 
			auto& node = underlyingGraph.GetNodeFromID(nodeId);

			node.nodeData->GetEditorState() = state;

			if (streamReader.HasKey("custom"))
			{
				streamReader.EnterScope("custom");
				node.nodeData->DeserializeCustom(streamReader);
				streamReader.ExitScope();
			}

			streamReader.ForEach("inputParams", [&]()
			{
				const uint32_t index = streamReader.ReadAtKey("index", 0u);
				auto& param = node.nodeData->GetInputParameter(index);

				if (param.deserializationFunc)
				{
					param.deserializationFunc(streamReader, param);
				}
			});

			streamReader.ForEach("outputParams", [&]()
			{
				const uint32_t index = streamReader.ReadAtKey("index", 0u);
				auto& param = node.nodeData->GetOutputParameter(index);

				if (param.deserializationFunc)
				{
					param.deserializationFunc(streamReader, param);
				}
			});
		});

		streamReader.ForEach("Edges", [&]() 
		{
			const UUID64 edgeId = streamReader.ReadAtKey("id", UUID64(0));
			const UUID64 startNodeId = streamReader.ReadAtKey("startNode", UUID64(0));
			const UUID64 endNodeId = streamReader.ReadAtKey("endNode", UUID64(0));
			const uint32_t inputIndex = streamReader.ReadAtKey("inputIndex", uint32_t(0));
			const uint32_t outputIndex = streamReader.ReadAtKey("outputIndex", uint32_t(0));
			
			underlyingGraph.LinkNodes(edgeId, startNodeId, endNodeId, CreateRef<Mosaic::MosaicEdge>(inputIndex, outputIndex));
		});

		streamReader.ExitScope();

		// #TODO_Ivar: Should probably not happen here
		mosaicAsset->Compile();

		return true;
	}

	void MosaicGraphImporter::Save(const AssetMetadata& metadata, const Ref<Asset>& asset) const
	{
		const Ref<Material> mosaicAsset = std::reinterpret_pointer_cast<Material>(asset);
		const auto graph = mosaicAsset->GetGraph();

		YAMLFileStreamWriter streamWriter{ AssetManager::GetFilesystemPath(metadata.filePath) };
		streamWriter.BeginMap();
		streamWriter.BeginMapNamned("MosaicGraph");
		streamWriter.SetKey("guid", mosaicAsset->m_materialGUID);
		streamWriter.SetKey("state", graph.GetEditorState());

		streamWriter.BeginSequence("Nodes");
		for (const auto& node : graph.GetUnderlyingGraph().GetNodes())
		{
			streamWriter.BeginMap();
			streamWriter.SetKey("id", node.id);
			streamWriter.SetKey("guid", node.nodeData->GetGUID());
			streamWriter.SetKey("state", node.nodeData->GetEditorState());
			
			streamWriter.BeginMapNamned("custom");
			node.nodeData->SerializeCustom(streamWriter);
			streamWriter.EndMap();

			streamWriter.BeginSequence("inputParams");
			for (const auto& input : node.nodeData->GetInputParameters())
			{
				streamWriter.BeginMap();
				streamWriter.SetKey("index", input.index);

				if (input.serializationFunc)
				{
					input.serializationFunc(streamWriter, input);
				}

				streamWriter.EndMap();
			}
			streamWriter.EndSequence();

			streamWriter.BeginSequence("outputParams");
			for (const auto& output : node.nodeData->GetOutputParameters())
			{
				streamWriter.BeginMap();
				streamWriter.SetKey("index", output.index);

				if (output.serializationFunc)
				{
					output.serializationFunc(streamWriter, output);
				}

				streamWriter.EndMap();
			}
			streamWriter.EndSequence();

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
