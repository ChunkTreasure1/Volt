#include "vtpch.h"
#include "PrefabImporter.h"

#include "Volt/Asset/AssetManager.h"
#include "Volt/Asset/Prefab.h"

#include "Volt/Asset/Importers/SceneImporter.h"

#include "Volt/Utility/FileIO/YAMLFileStreamWriter.h"
#include "Volt/Utility/FileIO/YAMLFileStreamReader.h"

namespace Volt
{
	PrefabImporter::PrefabImporter()
	{
		s_instance = this;
	}

	PrefabImporter::~PrefabImporter()
	{
		s_instance = nullptr;
	}

	bool PrefabImporter::Load(const AssetMetadata& metadata, Ref<Asset>& asset) const
	{
		asset = CreateRef<Prefab>();
		Ref<Prefab> prefab = std::reinterpret_pointer_cast<Prefab>(asset);

		const auto filePath = AssetManager::GetFilesystemPath(metadata.filePath);

		YAMLFileStreamReader streamReader{};
		if (!streamReader.OpenFile(filePath))
		{
			VT_CORE_ERROR("Failed to open file {0}!", metadata.filePath);
			asset->SetFlag(AssetFlag::Invalid, true);
			return false;
		}

		Ref<Scene> prefabScene = CreateRef<Scene>();

		streamReader.EnterScope("Prefab");
		{
			prefab->m_version = streamReader.ReadKey("version", uint32_t(0));
			prefab->m_rootEntityId = streamReader.ReadKey("rootEntityId", Entity::NullID());

			streamReader.ForEach("Entities", [&]() 
			{
				SceneImporter::Get().DeserializeEntity(prefabScene, metadata, streamReader);
			});
		
			streamReader.ForEach("PrefabReferences", [&]() 
			{
				EntityID entityId = streamReader.ReadKey("entity", Entity::NullID());
				AssetHandle prefabHandle = streamReader.ReadKey("prefabHandle", Asset::Null());
				EntityID prefabEntityReference = streamReader.ReadKey("prefabEntityReference", Entity::NullID());

				auto& prefabRefData = prefab->m_prefabReferencesMap[entityId];
				prefabRefData.prefabAsset = prefabHandle;
				prefabRefData.prefabReferenceEntity = prefabEntityReference;
			});
		}
		streamReader.ExitScope();

		prefab->m_prefabScene = prefabScene;
		return true;
	}

	void PrefabImporter::Save(const AssetMetadata& metadata, const Ref<Asset>& asset) const
	{
		const Ref<Prefab> prefab = std::reinterpret_pointer_cast<Prefab>(asset);
	
		YAMLFileStreamWriter streamWriter{ AssetManager::GetFilesystemPath(metadata.filePath) };

		streamWriter.BeginMap();
		streamWriter.BeginMapNamned("Prefab");

		streamWriter.SetKey("version", prefab->m_version);
		streamWriter.SetKey("rootEntityId", prefab->m_rootEntityId);

		streamWriter.BeginSequence("Entities");
		{
			for (const auto id : prefab->m_prefabScene->GetAllEntities())
			{
				SceneImporter::Get().SerializeEntity(id, metadata, prefab->m_prefabScene, streamWriter);
			}
		}
		streamWriter.EndSequence();

		streamWriter.BeginSequence("PrefabReferences");
		{
			for (const auto& ref : prefab->m_prefabReferencesMap)
			{
				streamWriter.BeginMap();
				streamWriter.SetKey("entity", ref.first);
				streamWriter.SetKey("prefabHandle", ref.second.prefabAsset);
				streamWriter.SetKey("prefabEntityReference", ref.second.prefabReferenceEntity);
				streamWriter.EndMap();
			}
		}
		streamWriter.EndSequence();

		streamWriter.EndMap();
		streamWriter.EndMap();

		streamWriter.WriteToDisk();
	}
}
