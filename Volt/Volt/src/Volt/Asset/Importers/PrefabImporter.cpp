#include "vtpch.h"
#include "PrefabImporter.h"

#include "Volt/Asset/Prefab.h"
#include "Volt/Log/Log.h"
#include "Volt/Components/Components.h"
#include "Volt/Project/ProjectManager.h"

#include "Volt/Utility/YAMLSerializationHelpers.h"
#include "Volt/Utility/SerializationMacros.h"

#include "Volt/Asset/Importers/SceneImporter.h"
#include "Volt/Asset/AssetManager.h"

#include "Volt/Scripting/Mono/MonoScriptEngine.h"
#include "Volt/Scripting/Mono/MonoScriptClass.h"

#include <GraphKey/Graph.h>

#include <yaml-cpp/yaml.h>

namespace Volt
{
	bool PrefabImporter::Load(const AssetMetadata& metadata, Ref<Asset>& asset) const
	{
		asset = CreateRef<Prefab>();
		const auto filePath = AssetManager::GetFilesystemPath(metadata.filePath);

		Wire::Registry registry;

		if (!std::filesystem::exists(filePath))
		{
			VT_CORE_ERROR("File {0} not found!", metadata.filePath);
			asset->SetFlag(AssetFlag::Missing, true);
			return false;
		}

		std::ifstream file(filePath);
		if (!file.is_open())
		{
			VT_CORE_ERROR("Failed to open file: {0}!", metadata.filePath);
			asset->SetFlag(AssetFlag::Invalid, true);
			return false;
		}

		std::stringstream sstream;
		sstream << file.rdbuf();
		file.close();

		YAML::Node root;

		try
		{
			root = YAML::Load(sstream.str());
		}
		catch (std::exception& e)
		{
			VT_CORE_ERROR("{0} contains invalid YAML! Please correct it! Error: {1}", metadata.filePath, e.what());
			asset->SetFlag(AssetFlag::Invalid, true);
			return false;
		}

		Ref<Prefab> prefab = std::reinterpret_pointer_cast<Prefab>(asset);

		uint32_t version = 0;
		YAML::Node prefabNode = root["Prefab"];
		VT_DESERIALIZE_PROPERTY(version, version, prefabNode, 0);

		YAML::Node entitiesNode = prefabNode["entities"];

		for (const auto& entityNode : entitiesNode)
		{
			entt::entity entityId = Wire::NullID;
			VT_DESERIALIZE_PROPERTY(id, entityId, entityNode, (entt::entity)Wire::NullID);

			if (entityId == Wire::NullID)
			{
				continue;
			}

			registry.AddEntity(entityId);

			YAML::Node componentsNode = entityNode["components"];
			if (componentsNode)
			{
				for (auto compNode : componentsNode)
				{
					WireGUID componentGUID;
					VT_DESERIALIZE_PROPERTY(guid, componentGUID, compNode, WireGUID::Null());
					if (componentGUID == WireGUID::Null())
					{
						continue;
					}

					auto* componentData = (uint8_t*)registry.AddComponent(componentGUID, entityId);

					YAML::Node propertiesNode = compNode["properties"];
					if (propertiesNode)
					{
						auto regInfo = Wire::ComponentRegistry::GetRegistryDataFromGUID(componentGUID);

						for (auto propNode : propertiesNode)
						{
							Wire::ComponentRegistry::PropertyType type;
							Wire::ComponentRegistry::PropertyType vectorType;
							std::string name;

							VT_DESERIALIZE_PROPERTY(type, type, propNode, Wire::ComponentRegistry::PropertyType::Unknown);
							VT_DESERIALIZE_PROPERTY(vectorType, vectorType, propNode, Wire::ComponentRegistry::PropertyType::Unknown);
							VT_DESERIALIZE_PROPERTY(name, name, propNode, std::string("Null"));

							if (type == Wire::ComponentRegistry::PropertyType::Unknown)
							{
								continue;
							}

							// Try to find property
							auto it = std::find_if(regInfo.properties.begin(), regInfo.properties.end(), [name, type](const auto& prop)
							{
								return prop.name == name && prop.type == type;
							});

							if (it != regInfo.properties.end())
							{
								SceneImporter::GetPropertyDeserializers()[type](componentData, it->offset, it->vectorType, propNode);
							}
						}
					}
				}
			}

			YAML::Node monoNode = entityNode["MonoScripts"];
			if (monoNode && registry.HasComponent<MonoScriptComponent>(entityId))
			{
				SceneImporter::DeserializeMono(entityId, prefab->myScriptFieldCache, entityNode);
			}

			YAML::Node graphNode = entityNode["Graph"];
			if (graphNode && registry.HasComponent<VisualScriptingComponent>(entityId))
			{
				auto& vsComp = registry.GetComponent<VisualScriptingComponent>(entityId);
				vsComp.graph = CreateRef<GraphKey::Graph>(entityId);

				GraphKey::Graph::Deserialize(vsComp.graph, graphNode);
			}

			if (!registry.HasComponent<EntityDataComponent>(entityId))
			{
				registry.AddComponent<EntityDataComponent>(entityId);
			}

			if (!registry.HasComponent<PrefabComponent>(entityId))
			{
				registry.AddComponent<PrefabComponent>(entityId);
			}

			auto& prefabComp = registry.GetComponent<PrefabComponent>(entityId);
			prefabComp.prefabAsset = metadata.handle;
			prefabComp.prefabEntity = entityId;

			if (prefab->myRootId == 0 && registry.HasComponent<RelationshipComponent>(entityId))
			{
				auto parent = registry.GetComponent<RelationshipComponent>(entityId).Parent;
				if (parent == Wire::NullID)
				{
					prefab->myRootId = entityId;
				}
			}
		}

		prefab->myRegistry = registry;
		prefab->myVersion = version;

		return true;
	}

	void PrefabImporter::Save(const AssetMetadata& metadata, const Ref<Asset>& asset) const
	{
		Ref<Prefab> prefab = std::reinterpret_pointer_cast<Prefab>(asset);
		auto& registry = prefab->myRegistry;

		YAML::Emitter out;
		out << YAML::BeginMap;
		out << YAML::Key << "Prefab" << YAML::Value;
		{
			out << YAML::BeginMap;
			VT_SERIALIZE_PROPERTY(version, prefab->myVersion, out);

			out << YAML::Key << "entities" << YAML::BeginSeq;
			for (const auto& id : registry.GetAllEntities())
			{
				out << YAML::BeginMap;
				VT_SERIALIZE_PROPERTY(id, id, out);
				{

					out << YAML::Key << "components" << YAML::BeginSeq;
					for (const auto& [guid, pool] : registry.GetPools())
					{
						if (!pool->HasComponent(id))
						{
							continue;
						}

						auto* componentData = (uint8_t*)pool->GetComponent(id);
						const auto& compInfo = Wire::ComponentRegistry::GetRegistryDataFromGUID(guid);

						out << YAML::BeginMap;
						out << YAML::Key << "guid" << YAML::Value << guid;
						out << YAML::Key << "properties" << YAML::BeginSeq;
						for (const auto& prop : compInfo.properties)
						{
							if (prop.serializable)
							{
								out << YAML::BeginMap;
								VT_SERIALIZE_PROPERTY(type, prop.type, out);
								VT_SERIALIZE_PROPERTY(vectorType, prop.vectorType, out);
								VT_SERIALIZE_PROPERTY(name, prop.name, out);

								SceneImporter::GetPropertySerializers()[prop.type](componentData, prop.offset, prop.vectorType, out);

								out << YAML::EndMap;
							}
						}
						out << YAML::EndSeq;
						out << YAML::EndMap;
					}
					out << YAML::EndSeq;

					if (registry.HasComponent<MonoScriptComponent>(id))
					{
						SceneImporter::SerializeMono(id, prefab->myScriptFieldCache, registry, out);
					}

					if (registry.HasComponent<VisualScriptingComponent>(id))
					{
						auto* vsComp = (VisualScriptingComponent*)registry.GetComponentPtr(VisualScriptingComponent::comp_guid, id);
						if (vsComp->graph)
						{
							GraphKey::Graph::Serialize(vsComp->graph, out);
						}
					}
				}
				out << YAML::EndMap;
			}
			out << YAML::EndSeq;
			out << YAML::EndMap;
		}
		out << YAML::EndMap;

		std::ofstream fout(AssetManager::GetFilesystemPath(metadata.filePath));
		fout << out.c_str();
		fout.close();
	}
}
