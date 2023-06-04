#include "vtpch.h"
#include "NetContractImporter.h"
#include "Volt/Asset/AssetManager.h"
#include "Volt/Net/SceneInteraction/NetContract.h"

#include <yaml-cpp/yaml.h>
#include "Volt/Utility/SerializationMacros.h"
#include "Volt/Utility/YAMLSerializationHelpers.h"

namespace Volt
{
	bool NetContractImporter::Load(const std::filesystem::path& path, Ref<Asset>& asset) const
	{
		const auto filePath = AssetManager::GetContextPath(path) / path;

		if (!std::filesystem::exists(filePath)) [[unlikely]]
		{
			VT_CORE_ERROR("File {0} not found!", path.string().c_str());
			asset->SetFlag(AssetFlag::Missing, true);
			return false;
		}

		std::ifstream file(filePath);
		if (!file.is_open()) [[unlikely]]
		{
			VT_CORE_ERROR("Failed to open file: {0}!", path.string().c_str());
			asset->SetFlag(AssetFlag::Invalid, true);
			return false;
		}
		asset = CreateRef<NetContract>();
		auto rAsset = reinterpret_pointer_cast<NetContract>(asset);

		std::stringstream sstream;
		sstream << file.rdbuf();

		YAML::Node root = YAML::Load(sstream.str());
		YAML::Node node = root["Contract"];

		{
			VT_DESERIALIZE_PROPERTY(prefab, rAsset->prefab, node, AssetHandle(0));

			if (node["Calls"])
			{
				for (YAML::const_iterator it = node["Calls"].begin(); it != node["Calls"].end(); ++it)
				{
					auto key = it->first.as<uint8_t>();
					auto value = it->second.as<std::string>();
					std::pair<eNetEvent, std::string> entry{ (eNetEvent)key,value };
					rAsset->calls.insert(entry);
				}
			}

			if (node["Rules"])
			{
				auto ruleNodeOuter = node["Rules"];
				for (YAML::const_iterator it = ruleNodeOuter.begin(); it != ruleNodeOuter.end(); ++it)
				{
					auto outerKey = it->first.as<Wire::EntityId>();
					std::unordered_map<std::string, NetRule> outerValue;
					auto ruleNodeInner = ruleNodeOuter[std::to_string(outerKey).c_str()];

					for (YAML::const_iterator it = ruleNodeInner.begin(); it != ruleNodeInner.end(); ++it)
					{
						std::string innerKey = it->first.as<std::string>();
						NetRule innerValue;
						VT_DESERIALIZE_PROPERTY(owner, innerValue.owner, ruleNodeInner[innerKey], false);
						VT_DESERIALIZE_PROPERTY(other, innerValue.other, ruleNodeInner[innerKey], false);
						VT_DESERIALIZE_PROPERTY(host, innerValue.host, ruleNodeInner[innerKey], false);

						outerValue.insert({ innerKey, innerValue });
					}

					std::pair<Wire::EntityId, std::unordered_map<std::string, NetRule>> entry{ outerKey,outerValue };
					rAsset->rules.insert(entry);
				}
			}

		}
		return true;
	}

	void NetContractImporter::Save(const Ref<Asset>& asset) const
	{
		Ref<NetContract> p = std::reinterpret_pointer_cast<NetContract>(asset);
		YAML::Emitter out;
		out << YAML::BeginMap;
		out << YAML::Key << "Contract" << YAML::Value;
		{
			out << YAML::BeginMap;
			VT_SERIALIZE_PROPERTY(prefab, p->prefab, out);

			// CALLS
			out << YAML::Key << "Calls" << YAML::BeginMap;
			for (const auto& call : p->calls)
			{
				out << YAML::Key << (uint8_t)call.first << YAML::Value << call.second;
			}
			out << YAML::EndMap;

			// RULES
			out << YAML::Key << "Rules" << YAML::BeginMap;
			for (const auto& ruleOuter : p->rules)
			{
				out << YAML::Key << ruleOuter.first << YAML::BeginMap;
				for (const auto& ruleInner : ruleOuter.second)
				{
					out << YAML::Key << ruleInner.first << YAML::BeginMap;
					VT_SERIALIZE_PROPERTY(owner, ruleInner.second.owner, out);
					VT_SERIALIZE_PROPERTY(other, ruleInner.second.other, out);
					VT_SERIALIZE_PROPERTY(host, ruleInner.second.host, out);
					out << YAML::EndMap;
				}
				out << YAML::EndMap;
			}
			out << YAML::EndMap;
			out << YAML::EndMap;
		}
		out << YAML::EndMap;
		std::ofstream fout(AssetManager::GetContextPath(asset->path) / asset->path);
		fout << out.c_str();
		fout.close();
	}
}
