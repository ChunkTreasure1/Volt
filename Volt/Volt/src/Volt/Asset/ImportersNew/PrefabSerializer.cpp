#include "vtpch.h"
#include "PrefabSerializer.h"

#include "Volt/Asset/AssetManager.h"
#include "Volt/Asset/Prefab.h"

#include "Volt/Asset/ImportersNew/SceneSerializer.h"

#include "Volt/Utility/FileIO/YAMLMemoryStreamWriter.h"
#include "Volt/Utility/FileIO/YAMLMemoryStreamReader.h"

namespace Volt
{
	PrefabSerializer::PrefabSerializer()
	{
		s_instance = this;
	}

	PrefabSerializer::~PrefabSerializer()
	{
		s_instance = nullptr;
	}

	void PrefabSerializer::Serialize(const AssetMetadata& metadata, const Ref<Asset>& asset) const
	{
		const Ref<Prefab> prefab = std::reinterpret_pointer_cast<Prefab>(asset);

		YAMLMemoryStreamWriter yamlStreamWriter{};
		yamlStreamWriter.BeginMap();
		yamlStreamWriter.BeginMapNamned("Prefab");

		yamlStreamWriter.SetKey("version", prefab->m_version);
		yamlStreamWriter.SetKey("rootEntityId", prefab->m_rootEntityId);

		yamlStreamWriter.BeginSequence("Entities");
		{
			for (const auto id : prefab->m_prefabScene->GetAllEntities())
			{
				SceneSerializer::Get().SerializeEntity(id, metadata, prefab->m_prefabScene, yamlStreamWriter);
			}
		}
		yamlStreamWriter.EndSequence();

		yamlStreamWriter.BeginSequence("PrefabReferences");
		{
			for (const auto& ref : prefab->m_prefabReferencesMap)
			{
				yamlStreamWriter.BeginMap();
				yamlStreamWriter.SetKey("entity", ref.first);
				yamlStreamWriter.SetKey("prefabHandle", ref.second.prefabAsset);
				yamlStreamWriter.SetKey("orefabEntityReference", ref.second.prefabReferenceEntity);
				yamlStreamWriter.EndMap();
			}
		}
		yamlStreamWriter.EndSequence();

		yamlStreamWriter.EndMap();
		yamlStreamWriter.EndMap();

		BinaryStreamWriter streamWriter{};
		const size_t compressedDataOffset = AssetSerializer::WriteMetadata(metadata, asset->GetVersion(), streamWriter);

		Buffer buffer = yamlStreamWriter.WriteAndGetBuffer();
		streamWriter.Write(buffer);
		buffer.Release();

		const auto filePath = AssetManager::GetFilesystemPath(metadata.filePath);
		streamWriter.WriteToDisk(filePath, true, compressedDataOffset);
	}

	bool PrefabSerializer::Deserialize(const AssetMetadata& metadata, Ref<Asset> destinationAsset) const
	{
		const auto filePath = AssetManager::GetFilesystemPath(metadata.filePath);

		if (!std::filesystem::exists(filePath))
		{
			VT_CORE_ERROR("File {0} not found!", metadata.filePath);
			destinationAsset->SetFlag(AssetFlag::Missing, true);
			return false;
		}

		BinaryStreamReader streamReader{ filePath };

		if (!streamReader.IsStreamValid())
		{
			VT_CORE_ERROR("Failed to open file: {0}!", metadata.filePath);
			destinationAsset->SetFlag(AssetFlag::Invalid, true);
			return false;
		}

		SerializedAssetMetadata serializedMetadata = AssetSerializer::ReadMetadata(streamReader);
		VT_CORE_ASSERT(serializedMetadata.version == destinationAsset->GetVersion(), "Incompatible version!")

		Buffer buffer{};
		streamReader.Read(buffer);

		YAMLMemoryStreamReader yamlStreamReader{};
		if (!yamlStreamReader.ConsumeBuffer(buffer))
		{
			destinationAsset->SetFlag(AssetFlag::Invalid, true);
			return false;
		}

		Ref<Prefab> prefab = std::reinterpret_pointer_cast<Prefab>(destinationAsset);
		Ref<Scene> prefabScene = CreateRef<Scene>();

		yamlStreamReader.EnterScope("Prefab");
		{
			prefab->m_version = yamlStreamReader.ReadKey("version", uint32_t(0));
			prefab->m_rootEntityId = yamlStreamReader.ReadKey("rootEntityId", Entity::NullID());

			yamlStreamReader.ForEach("Entities", [&]() 
			{
				SceneSerializer::Get().DeserializeEntity(prefabScene, metadata, yamlStreamReader);
			});

			yamlStreamReader.ForEach("PrefabReferences", [&]() 
			{
				EntityID entityId = yamlStreamReader.ReadKey("entity", Entity::NullID());
				AssetHandle prefabHandle = yamlStreamReader.ReadKey("prefabHandle", Asset::Null());
				EntityID prefabEntityReference = yamlStreamReader.ReadKey("prefabEntityReferences", Entity::NullID());

				auto& prefabRefData = prefab->m_prefabReferencesMap[entityId];
				prefabRefData.prefabAsset = prefabHandle;
				prefabRefData.prefabReferenceEntity = prefabEntityReference;
			});
		}
		yamlStreamReader.ExitScope();

		prefab->m_prefabScene = prefabScene;
		return true;
	}
}