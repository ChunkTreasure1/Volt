#include "vtpch.h"
#include "BehaviourTreeSerializer.h"

#include "Volt/Asset/AssetManager.h"
#include "Volt/BehaviorTree/BehaviorTree.hpp"

namespace Volt
{
	struct SerializedDecorator
	{
		UUID64 uuid;
		std::string compareFunctionName;
		BehaviorTree::eDecoratorType type;
	
		static void Serialize(BinaryStreamWriter& streamWriter, const SerializedDecorator& data)
		{
			streamWriter.Write(data.uuid);
			streamWriter.Write(data.compareFunctionName);
			streamWriter.Write(data.type);
		}

		static void Deserialize(BinaryStreamReader& streamReader, SerializedDecorator& outData)
		{
			streamReader.Read(outData.uuid);
			streamReader.Read(outData.compareFunctionName);
			streamReader.Read(outData.type);
		}
	};

	struct SerializedLeaf
	{
		UUID64 uuid;
		std::string functionName;

		static void Serialize(BinaryStreamWriter& streamWriter, const SerializedLeaf& data)
		{
			streamWriter.Write(data.uuid);
			streamWriter.Write(data.functionName);
		}

		static void Deserialize(BinaryStreamReader& streamReader, SerializedLeaf& outData)
		{
			streamReader.Read(outData.uuid);
			streamReader.Read(outData.functionName);
		}
	};

	struct SerializedLink
	{
		UUID64 id;
		UUID64 parentId;
		UUID64 childId;
	};

	struct SerializedPosition
	{
		UUID64 id;
		std::string positionString;

		static void Serialize(BinaryStreamWriter& streamWriter, const SerializedPosition& data)
		{
			streamWriter.Write(data.id);
			streamWriter.Write(data.positionString);
		}

		static void Deserialize(BinaryStreamReader& streamReader, SerializedPosition& outData)
		{
			streamReader.Read(outData.id);
			streamReader.Read(outData.positionString);
		}
	};

	struct BehaviourTreeSerializationData
	{
		Vector<UUID64> sequenceIDs;
		Vector<UUID64> selectorIDs;
		Vector<SerializedDecorator> decorators;
		Vector<SerializedLeaf> leafs;
		Vector<SerializedLink> links;
		Vector<SerializedPosition> positions;

		static void Serialize(BinaryStreamWriter& streamWriter, const BehaviourTreeSerializationData& data)
		{
			streamWriter.Write(data.sequenceIDs);
			streamWriter.Write(data.selectorIDs);
			streamWriter.Write(data.decorators);
			streamWriter.Write(data.leafs);
			streamWriter.WriteRaw(data.links);
			streamWriter.Write(data.positions);
		}

		static void Deserialize(BinaryStreamReader& streamReader, BehaviourTreeSerializationData& outData)
		{
			streamReader.Read(outData.sequenceIDs);
			streamReader.Read(outData.selectorIDs);
			streamReader.Read(outData.decorators);
			streamReader.Read(outData.leafs);
			streamReader.ReadRaw(outData.links);
			streamReader.Read(outData.positions);
		}
	};

	void BehaviourTreeSerializer::Serialize(const AssetMetadata& metadata, const Ref<Asset>& asset) const
	{
		Ref<BehaviorTree::Tree> tree = std::reinterpret_pointer_cast<BehaviorTree::Tree>(asset);
		UUID64 rootID = tree->GetRoot();

		BehaviourTreeSerializationData serializationData{};

		for (const auto& [id, node] : tree->GetNodeManager().m_nodes)
		{
			switch (node->m_kind)
			{
				case BehaviorTree::eNodeKind::DECORATOR: 
				{
					auto decorator = std::reinterpret_pointer_cast<BehaviorTree::Decorator>(tree->GetNodeManager().GetNodeFromUUID(id));

					auto& serDecorator = serializationData.decorators.emplace_back();
					serDecorator.uuid = id;
					serDecorator.compareFunctionName = decorator->m_if;
					serDecorator.type = decorator->m_type;

					break;
				}

				case BehaviorTree::eNodeKind::LEAF: 
				{
					auto leaf = std::reinterpret_pointer_cast<BehaviorTree::Leaf>(tree->GetNodeManager().GetNodeFromUUID(id));

					auto& serLeaf = serializationData.leafs.emplace_back();
					serLeaf.uuid = leaf->GetUUID();
					serLeaf.functionName = leaf->m_monoScriptFunctonName;

					break;
				}

				case BehaviorTree::eNodeKind::SELECTOR: serializationData.selectorIDs.push_back(id); break;
				case BehaviorTree::eNodeKind::SEQUENCE: serializationData.sequenceIDs.push_back(id); break;
			}
		}

		for (const auto& [id, links] : tree->GetNodeManager().m_links)
		{
			for (const auto& link : links)
			{
				auto& newLink = serializationData.links.emplace_back();
				newLink.id = link.m_uuid;
				newLink.parentId = link.m_parentID;
				newLink.childId = link.m_childID;
			}
		}

		for (const auto& [id, node] : tree->GetNodeManager().m_nodes)
		{
			auto& pos = serializationData.positions.emplace_back();
			pos.id = id;
			pos.positionString = node->GetPos();
		}

		BinaryStreamWriter streamWriter{};
		const size_t compressedDataOffset = AssetSerializer::WriteMetadata(metadata, asset->GetVersion(), streamWriter);

		streamWriter.Write(serializationData);

		const auto filePath = AssetManager::GetFilesystemPath(metadata.filePath);
		streamWriter.WriteToDisk(filePath, true, compressedDataOffset);
	}

	bool BehaviourTreeSerializer::Deserialize(const AssetMetadata& metadata, Ref<Asset> destinationAsset) const
	{
		const auto filePath = AssetManager::GetFilesystemPath(metadata.filePath);

		if (!std::filesystem::exists(filePath))
		{
			VT_LOG(LogVerbosity::Error, "File {0} not found!", metadata.filePath);
			destinationAsset->SetFlag(AssetFlag::Missing, true);
			return false;
		}

		BinaryStreamReader streamReader{ filePath };

		if (!streamReader.IsStreamValid())
		{
			VT_LOG(LogVerbosity::Error, "Failed to open file: {0}!", metadata.filePath);
			destinationAsset->SetFlag(AssetFlag::Invalid, true);
			return false;
		}

		SerializedAssetMetadata serializedMetadata = AssetSerializer::ReadMetadata(streamReader);
		VT_ASSERT_MSG(serializedMetadata.version == destinationAsset->GetVersion(), "Incompatible version!");

		BehaviourTreeSerializationData serializationData{};
		streamReader.Read(serializationData);

		Ref<BehaviorTree::Tree> behaviorTree = std::reinterpret_pointer_cast<BehaviorTree::Tree>(destinationAsset);

		for (const auto& node : serializationData.sequenceIDs)
		{
			behaviorTree->CreateNode<BehaviorTree::Sequence>(node);
		}

		for (const auto& node : serializationData.selectorIDs)
		{
			behaviorTree->CreateNode<BehaviorTree::Selector>(node);
		}

		for (const auto& node : serializationData.decorators)
		{
			auto newNodeId = behaviorTree->CreateNode<BehaviorTree::Decorator>(node.uuid);
			auto newNode = std::reinterpret_pointer_cast<BehaviorTree::Decorator>(behaviorTree->GetNodeManager().GetNodeFromUUID(newNodeId));

			newNode->m_type = node.type;
			newNode->m_if = node.compareFunctionName;
		}

		for (const auto& node : serializationData.leafs)
		{
			auto newNodeId = behaviorTree->CreateNode<BehaviorTree::Leaf>(node.uuid);
			auto newNode = std::reinterpret_pointer_cast<BehaviorTree::Leaf>(behaviorTree->GetNodeManager().GetNodeFromUUID(newNodeId));

			newNode->m_monoScriptFunctonName = node.functionName;
		}

		for (const auto& link : serializationData.links)
		{
			behaviorTree->GetNodeManager().RegisterLink(link.parentId, link.childId, link.id);
		}

		for (const auto& node : serializationData.positions)
		{
			behaviorTree->GetNodeManager().GetNodeFromUUID(node.id)->SetPos(node.positionString);
		}

		return true;
	}
}
