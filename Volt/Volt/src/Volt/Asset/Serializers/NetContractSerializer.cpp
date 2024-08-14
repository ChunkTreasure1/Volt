#include "vtpch.h"
#include "NetContractSerializer.h"

#include <AssetSystem/AssetManager.h>
#include "Volt/Net/SceneInteraction/NetContract.h"

namespace Volt
{
	struct NetCall
	{
		eNetEvent event;
		std::string functionName;

		static void Serialize(BinaryStreamWriter& streamWriter, const NetCall& data)
		{
			streamWriter.Write(data.event);
			streamWriter.Write(data.functionName);
		}

		static void Deserialize(BinaryStreamReader& streamReader, NetCall& outData)
		{
			streamReader.Read(outData.event);
			streamReader.Read(outData.functionName);
		}
	};

	struct SerializedNetRule
	{
		std::string name;
		bool owner;
		bool other;
		bool host;

		static void Serialize(BinaryStreamWriter& streamWriter, const SerializedNetRule& data)
		{
			streamWriter.Write(data.name);
			streamWriter.Write(data.owner);
			streamWriter.Write(data.other);
			streamWriter.Write(data.host);
		}

		static void Deserialize(BinaryStreamReader& streamReader, SerializedNetRule& outData)
		{
			streamReader.Read(outData.name);
			streamReader.Read(outData.owner);
			streamReader.Read(outData.other);
			streamReader.Read(outData.host);
		}
	};

	struct EntityNetRule
	{
		EntityID entityId;
		Vector<SerializedNetRule> rules;

		static void Serialize(BinaryStreamWriter& streamWriter, const EntityNetRule& data)
		{
			streamWriter.Write(data.entityId);
			streamWriter.Write(data.rules);
		}

		static void Deserialize(BinaryStreamReader& streamReader, EntityNetRule& outData)
		{
			streamReader.Read(outData.entityId);
			streamReader.Read(outData.rules);
		}
	};

	struct NetContractSerializationData
	{
		AssetHandle prefab;
		Vector<NetCall> calls;
		Vector<EntityNetRule> rules;

		static void Serialize(BinaryStreamWriter& streamWriter, const NetContractSerializationData& data)
		{
			streamWriter.Write(data.prefab);
			streamWriter.Write(data.calls);
			streamWriter.Write(data.rules);
		}

		static void Deserialize(BinaryStreamReader& streamReader, NetContractSerializationData& outData)
		{
			streamReader.Read(outData.prefab);
			streamReader.Read(outData.calls);
			streamReader.Read(outData.rules);
		}
	};

	void NetContractSerializer::Serialize(const AssetMetadata& metadata, const Ref<Asset>& asset) const
	{
		Ref<NetContract> contract = std::reinterpret_pointer_cast<NetContract>(asset);

		NetContractSerializationData serializationData{};
		serializationData.prefab = contract->prefab;
		
		for (const auto& [event, functionName] : contract->calls)
		{
			auto& newCall = serializationData.calls.emplace_back();
			newCall.event = event;
			newCall.functionName = functionName;
		}

		for (const auto& [entityId, rules] : contract->rules)
		{
			auto& newEntityRule = serializationData.rules.emplace_back();
			newEntityRule.entityId = entityId;

			for (const auto& [name, data] : rules)
			{
				auto& newRule = newEntityRule.rules.emplace_back();
				newRule.name = name;
				newRule.owner = data.owner;
				newRule.other = data.other;
				newRule.host = data.host;
			}
		}

		BinaryStreamWriter streamWriter{};
		const size_t compressedDataOffset = AssetSerializer::WriteMetadata(metadata, asset->GetVersion(), streamWriter);

		streamWriter.Write(serializationData);

		const auto filePath = AssetManager::GetFilesystemPath(metadata.filePath);
		streamWriter.WriteToDisk(filePath, true, compressedDataOffset);
	}

	bool NetContractSerializer::Deserialize(const AssetMetadata& metadata, Ref<Asset> destinationAsset) const
	{
		const auto filePath = AssetManager::GetFilesystemPath(metadata.filePath);

		if (!std::filesystem::exists(filePath))
		{
			VT_LOG(Error, "File {0} not found!", metadata.filePath);
			destinationAsset->SetFlag(AssetFlag::Missing, true);
			return false;
		}

		BinaryStreamReader streamReader{ filePath };

		if (!streamReader.IsStreamValid())
		{
			VT_LOG(Error, "Failed to open file: {0}!", metadata.filePath);
			destinationAsset->SetFlag(AssetFlag::Invalid, true);
			return false;
		}

		SerializedAssetMetadata serializedMetadata = AssetSerializer::ReadMetadata(streamReader);
		VT_ASSERT_MSG(serializedMetadata.version == destinationAsset->GetVersion(), "Incompatible version!");

		NetContractSerializationData serializationData{};
		streamReader.Read(serializationData);

		auto contract = reinterpret_pointer_cast<NetContract>(destinationAsset);

		contract->prefab = serializationData.prefab;

		for (const auto& call : serializationData.calls)
		{
			contract->calls[call.event] = call.functionName;
		}

		for (const auto& entityRule : serializationData.rules)
		{
			std::unordered_map<std::string, NetRule> entityRules;

			for (const auto& rule : entityRule.rules)
			{
				entityRules[rule.name] = { rule.owner, rule.other, rule.host };
			}

			contract->rules[entityRule.entityId] = entityRules;
		}

		return true;
	}
}
