#include "vtpch.h"
#include "BehaviorTreeImporter.h"

#include "Volt/Asset/AssetManager.h"
#include "Volt/BehaviorTree/BehaviorTree.hpp"
#include "Volt/Utility/SerializationMacros.h"
#include "Volt/Utility/YAMLSerializationHelpers.h"

#include <yaml-cpp/yaml.h>

using namespace Volt::BehaviorTree;

namespace Volt
{
	bool BehaviorTreeImporter::Load(const std::filesystem::path& path, Ref<Asset>& asset) const
	{
		asset = CreateRef<Volt::BehaviorTree::Tree>();
		Ref<Volt::BehaviorTree::Tree> behaviorTree = std::reinterpret_pointer_cast<Volt::BehaviorTree::Tree>(asset);

		const auto filePath = AssetManager::GetContextPath(path) / path;

		if (!std::filesystem::exists(filePath)) [[unlikely]]
		{
			VT_CORE_ERROR("File {0} not found!", path.string().c_str());
			asset->SetFlag(AssetFlag::Missing, true);
			return false;
		}

		std::ifstream input(filePath, std::ios::binary | std::ios::in);
		if (!input.is_open())
		{
			VT_CORE_ERROR("File {0} not found!", path.string().c_str());
			asset->SetFlag(AssetFlag::Invalid, true);
			return false;
		}

		std::stringstream sstream;
		sstream << input.rdbuf();
		input.close();

		YAML::Node root;
		try
		{
			root = YAML::Load(sstream.str());
		}
		catch (std::exception& e)
		{
			VT_CORE_ASSERT("{0} conains invalid YAML! Please correct it! Error: {1}", path, e.what());
			asset->SetFlag(AssetFlag::Invalid, true);
			return false;
		}

		root = root["Behavior Tree"];

		YAML::Node sequenceNodes = root["SequenceIDs"];
		for (const auto& n : sequenceNodes.as<std::vector<UUID>>())
			behaviorTree->CreateNode<BehaviorTree::Sequence>(n);
		YAML::Node selectorNodes = root["SelectorIDs"];
		for (const auto& n : selectorNodes.as<std::vector<UUID>>())
			behaviorTree->CreateNode<BehaviorTree::Selector>(n);

		// OH NO...
		YAML::Node decoraterNodes = root["DecoratorIDs"];
		for (const auto& n : decoraterNodes)
		{
			int decType;
			UUID thing;
			std::string ifFun1;
			VT_DESERIALIZE_PROPERTY(uuid, thing, n, uint64_t(0));
			VT_DESERIALIZE_PROPERTY(ifFun, ifFun1, n, std::string(""));
			VT_DESERIALIZE_PROPERTY(type, decType, n, 0);
			reinterpret_cast<BehaviorTree::Decorator*>(behaviorTree->GetNodeManager().GetNodeFromUUID(behaviorTree->CreateNode<BehaviorTree::Decorator>(thing)).get())->m_type = static_cast<BehaviorTree::eDecoratorType>(decType);
			reinterpret_cast<BehaviorTree::Decorator*>(behaviorTree->GetNodeManager().GetNodeFromUUID(thing).get())->m_if = ifFun1;
		}

		YAML::Node leafNodes = root["LeafIDs"];
		for (const auto& n : leafNodes)
		{
			UUID thing;
			std::string funcName;
			VT_DESERIALIZE_PROPERTY(uuid, thing, n, uint64_t(0));
			VT_DESERIALIZE_PROPERTY(functionName, funcName, n, std::string(""));
			reinterpret_cast<BehaviorTree::Leaf*>(behaviorTree->GetNodeManager().GetNodeFromUUID(behaviorTree->CreateNode<BehaviorTree::Leaf>(thing)).get())->m_monoScriptFunctonName = funcName;
		}

		YAML::Node linkNodes = root["Links"];
		for (const auto& n : linkNodes)
		{
			UUID lnkID;
			UUID parentID;
			UUID childID;

			VT_DESERIALIZE_PROPERTY(LinkID, lnkID, n, UUID(0));
			VT_DESERIALIZE_PROPERTY(ParentID, parentID, n, UUID(0));
			VT_DESERIALIZE_PROPERTY(ChildID, childID, n, UUID(0));

			behaviorTree->GetNodeManager().RegisterLink(parentID, childID, lnkID);
			//behaviorTree-><BehaviorTree::Selector>(n);
		}

		for (const auto& positionNode : root["Positions"])
		{
			UUID nodeID;
			std::string position;
			VT_DESERIALIZE_PROPERTY(uuid, nodeID, positionNode, Volt::UUID(0));
			VT_DESERIALIZE_PROPERTY(pos, position, positionNode, std::string(""));
			behaviorTree->GetNodeManager().GetNodeFromUUID(nodeID)->SetPos(position);
		}
		return true;
	}

	void BehaviorTreeImporter::Save(const Ref<Asset>& asset) const
	{
		Ref<BehaviorTree::Tree> tree = std::reinterpret_pointer_cast<BehaviorTree::Tree>(asset);
		Volt::UUID rootID = tree->GetRoot();

		std::vector<Volt::UUID> decoratorVec;
		std::vector<Volt::UUID> selectorVec;
		std::vector<Volt::UUID> sequenceVec;
		std::vector<Volt::UUID> leafVec;

		for (const auto& n : tree->GetNodeManager().m_nodes)
		{
			switch (n.second->m_kind)
			{
				case Volt::BehaviorTree::eNodeKind::DECORATOR: decoratorVec.push_back(n.first); break;
				case Volt::BehaviorTree::eNodeKind::SELECTOR: selectorVec.push_back(n.first); break;
				case Volt::BehaviorTree::eNodeKind::SEQUENCE: sequenceVec.push_back(n.first); break;
				case Volt::BehaviorTree::eNodeKind::LEAF: leafVec.push_back(n.first); break;
				default:break;
			}
		}

		YAML::Emitter out;
		out << YAML::BeginMap;
		out << YAML::Key << "Behavior Tree" << YAML::Value;
		{
			out << YAML::BeginMap;
			out << YAML::Key << "SequenceIDs" << YAML::BeginSeq;
			for (auto nID : sequenceVec)
			{
				out << nID;
			}
			out << YAML::EndSeq;
			//out << YAML::EndMap;

			out << YAML::Key << "SelectorIDs" << YAML::BeginSeq;
			for (auto nID : selectorVec)
			{
				out << nID;
			}
			out << YAML::EndSeq;

			out << YAML::Key << "DecoratorIDs" << YAML::BeginSeq;
			for (auto nID : decoratorVec)
			{
				out << YAML::BeginMap;
				VT_SERIALIZE_PROPERTY(uuid, nID, out);
				VT_SERIALIZE_PROPERTY(ifFun, reinterpret_cast<Volt::BehaviorTree::Decorator*>(tree->GetNodeManager().GetNodeFromUUID(nID).get())->m_if, out);
				VT_SERIALIZE_PROPERTY(type, (int)reinterpret_cast<Volt::BehaviorTree::Decorator*>(tree->GetNodeManager().GetNodeFromUUID(nID).get())->m_type, out);
				out << YAML::EndMap;
			}
			out << YAML::EndSeq;

			out << YAML::Key << "LeafIDs" << YAML::BeginSeq;
			for (auto nID : leafVec)
			{
				out << YAML::BeginMap;
				VT_SERIALIZE_PROPERTY(uuid, nID, out);
				VT_SERIALIZE_PROPERTY(functionName, reinterpret_cast<Volt::BehaviorTree::Leaf*>(tree->GetNodeManager().GetNodeFromUUID(nID).get())->m_monoScriptFunctonName, out);
				out << YAML::EndMap;
			}
			out << YAML::EndSeq;

			//out << YAML::BeginMap;
			out << YAML::Key << "Links" << YAML::BeginSeq;
			for (const auto& _linkList : tree->GetNodeManager().m_links)
				for (const auto& _link : _linkList.second)
				{
					out << YAML::BeginMap;
					out << YAML::Key << "LinkID" << YAML::Value << _link.m_uuid;
					out << YAML::Key << "ParentID" << YAML::Value << _link.m_parentID;
					out << YAML::Key << "ChildID" << YAML::Value << _link.m_childID;
					out << YAML::EndMap;
				}
			out << YAML::EndSeq;

			out << YAML::Key << "Positions" << YAML::BeginSeq;
			for (const auto& _pair : tree->GetNodeManager().m_nodes)
			{
				out << YAML::BeginMap;
				out << YAML::Key << "uuid" << YAML::Value << _pair.first;
				out << YAML::Key << "pos" << YAML::Value << _pair.second->GetPos();
				out << YAML::EndMap;
			}
			out << YAML::EndSeq;
			out << YAML::EndMap;
		}
		out << YAML::EndMap;

		std::ofstream fout(AssetManager::GetContextPath(asset->path) / asset->path);
		fout << out.c_str();
		fout.close();
	}

	void BehaviorTreeImporter::SaveBinary(uint8_t* buffer, const Ref<Asset>& asset) const
	{

	}

	bool BehaviorTreeImporter::LoadBinary(const uint8_t* buffer, const AssetPacker::AssetHeader& header, Ref<Asset>& asset) const
	{
		return false;
	}
}