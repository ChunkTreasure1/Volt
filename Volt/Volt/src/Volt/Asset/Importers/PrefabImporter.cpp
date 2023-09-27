#include "vtpch.h"
#include "PrefabImporter.h"

#include "Volt/Asset/AssetManager.h"
#include "Volt/Asset/Prefab.h"

#include "Volt/Asset/Importers/SceneImporter.h"

#include "Volt/Utility/FileIO/YAMLStreamWriter.h"
#include "Volt/Utility/FileIO/YAMLStreamReader.h"

namespace Volt
{
	bool PrefabImporter::Load(const AssetMetadata& metadata, Ref<Asset>& asset) const
	{
		asset = CreateRef<Prefab>();
		Ref<Prefab> prefab = std::reinterpret_pointer_cast<Prefab>(asset);

		const auto filePath = AssetManager::GetFilesystemPath(metadata.filePath);

		Ref<Scene> prefabScene = CreateRef<Scene>();

		YAMLStreamReader streamReader{};
		if (!streamReader.OpenFile(filePath))
		{
			VT_CORE_ERROR("Failed to open file {0}!", metadata.filePath);
			asset->SetFlag(AssetFlag::Invalid, true);
			return false;
		}

		streamReader.EnterScope("Prefab");
		{
			prefab->m_version = streamReader.ReadKey("version", uint32_t(0));
			prefab->m_rootEntityId = streamReader.ReadKey("rootEntityId", (entt::entity)entt::null);

			streamReader.ForEach("Entities", [&]() 
			{
				SceneImporter::Get().DeserializeEntity(prefabScene, metadata, streamReader);
			});
		}
		streamReader.ExitScope();

		prefab->m_prefabScene = prefabScene;
		return true;
	}

	void PrefabImporter::Save(const AssetMetadata& metadata, const Ref<Asset>& asset) const
	{
		const Ref<Prefab> prefab = std::reinterpret_pointer_cast<Prefab>(asset);
	
		YAMLStreamWriter streamWriter{ AssetManager::GetFilesystemPath(metadata.filePath) };

		streamWriter.BeginMap();
		streamWriter.BeginMapNamned("Prefab");

		streamWriter.SetKey("version", prefab->m_version);
		streamWriter.SetKey("rootEntityId", prefab->m_rootEntityId);

		streamWriter.BeginSequence("Entities");
		{
			for (const auto id : prefab->m_prefabScene->GetAllEntities())
			{
				SceneImporter::Get().SerializeEntity(id, prefab->m_prefabScene, streamWriter);
			}
		}
		streamWriter.EndSequence();

		streamWriter.EndMap();
		streamWriter.EndMap();

		streamWriter.WriteToDisk();
	}
}
