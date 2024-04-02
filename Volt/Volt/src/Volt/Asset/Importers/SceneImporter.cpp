#include "vtpch.h"
#include "SceneImporter.h"

#include "Volt/Asset/AssetManager.h"

#include "Volt/Components/CoreComponents.h"
#include "Volt/Components/RenderingComponents.h"

#include "Volt/Scene/Scene.h"
#include "Volt/Scene/Entity.h"
#include "Volt/Scene/Reflection/ComponentReflection.h"
#include "Volt/Scene/Reflection/ComponentRegistry.h"
#include "Volt/Asset/Prefab.h"

#include "Volt/Scripting/Mono/MonoScriptClass.h"
#include "Volt/Scripting/Mono/MonoScriptEngine.h"

#include "Volt/Core/BinarySerializer.h"

#include "Volt/Utility/YAMLSerializationHelpers.h"

#include "Volt/Utility/Algorithms.h"
#include "Volt/Utility/FileSystem.h"

namespace Volt
{
	template<typename T>
	void RegisterSerializationFunction(std::unordered_map<std::type_index, std::function<void(YAMLFileStreamWriter&, const uint8_t*, const size_t)>>& outTypes)
	{
		VT_PROFILE_FUNCTION();

		outTypes[std::type_index{ typeid(T) }] = [](YAMLFileStreamWriter& streamWriter, const uint8_t* data, const size_t offset)
		{
			const T& var = *reinterpret_cast<const T*>(&data[offset]);
			streamWriter.SetKey("data", var);
		};
	}

	template<typename T>
	void RegisterDeserializationFunction(std::unordered_map<std::type_index, std::function<void(YAMLFileStreamReader&, uint8_t*, const size_t)>>& outTypes)
	{
		VT_PROFILE_FUNCTION();

		outTypes[std::type_index{ typeid(T) }] = [](YAMLFileStreamReader& streamReader, uint8_t* data, const size_t offset)
		{
			*reinterpret_cast<T*>(&data[offset]) = streamReader.ReadAtKey("data", T());
		};
	}

	SceneImporter::SceneImporter()
	{
		RegisterSerializationFunction<int8_t>(s_typeSerializers);
		RegisterSerializationFunction<uint8_t>(s_typeSerializers);
		RegisterSerializationFunction<int16_t>(s_typeSerializers);
		RegisterSerializationFunction<uint16_t>(s_typeSerializers);
		RegisterSerializationFunction<int32_t>(s_typeSerializers);
		RegisterSerializationFunction<uint32_t>(s_typeSerializers);

		RegisterSerializationFunction<float>(s_typeSerializers);
		RegisterSerializationFunction<double>(s_typeSerializers);
		RegisterSerializationFunction<bool>(s_typeSerializers);

		RegisterSerializationFunction<glm::vec2>(s_typeSerializers);
		RegisterSerializationFunction<glm::vec3>(s_typeSerializers);
		RegisterSerializationFunction<glm::vec4>(s_typeSerializers);

		RegisterSerializationFunction<glm::uvec2>(s_typeSerializers);
		RegisterSerializationFunction<glm::uvec3>(s_typeSerializers);
		RegisterSerializationFunction<glm::uvec4>(s_typeSerializers);

		RegisterSerializationFunction<glm::ivec2>(s_typeSerializers);
		RegisterSerializationFunction<glm::ivec3>(s_typeSerializers);
		RegisterSerializationFunction<glm::ivec4>(s_typeSerializers);

		RegisterSerializationFunction<glm::quat>(s_typeSerializers);
		RegisterSerializationFunction<glm::mat4>(s_typeSerializers);
		RegisterSerializationFunction<VoltGUID>(s_typeSerializers);

		RegisterSerializationFunction<std::string>(s_typeSerializers);
		RegisterSerializationFunction<std::filesystem::path>(s_typeSerializers);

		RegisterSerializationFunction<Volt::EntityID>(s_typeSerializers);
		RegisterSerializationFunction<AssetHandle>(s_typeSerializers);

		RegisterDeserializationFunction<int8_t>(s_typeDeserializers);
		RegisterDeserializationFunction<uint8_t>(s_typeDeserializers);
		RegisterDeserializationFunction<int16_t>(s_typeDeserializers);
		RegisterDeserializationFunction<uint16_t>(s_typeDeserializers);
		RegisterDeserializationFunction<int32_t>(s_typeDeserializers);
		RegisterDeserializationFunction<uint32_t>(s_typeDeserializers);

		RegisterDeserializationFunction<float>(s_typeDeserializers);
		RegisterDeserializationFunction<double>(s_typeDeserializers);
		RegisterDeserializationFunction<bool>(s_typeDeserializers);

		RegisterDeserializationFunction<glm::vec2>(s_typeDeserializers);
		RegisterDeserializationFunction<glm::vec3>(s_typeDeserializers);
		RegisterDeserializationFunction<glm::vec4>(s_typeDeserializers);

		RegisterDeserializationFunction<glm::uvec2>(s_typeDeserializers);
		RegisterDeserializationFunction<glm::uvec3>(s_typeDeserializers);
		RegisterDeserializationFunction<glm::uvec4>(s_typeDeserializers);

		RegisterDeserializationFunction<glm::ivec2>(s_typeDeserializers);
		RegisterDeserializationFunction<glm::ivec3>(s_typeDeserializers);
		RegisterDeserializationFunction<glm::ivec4>(s_typeDeserializers);

		RegisterDeserializationFunction<glm::quat>(s_typeDeserializers);
		RegisterDeserializationFunction<glm::mat4>(s_typeDeserializers);
		RegisterDeserializationFunction<VoltGUID>(s_typeDeserializers);

		RegisterDeserializationFunction<std::string>(s_typeDeserializers);
		RegisterDeserializationFunction<std::filesystem::path>(s_typeDeserializers);

		RegisterDeserializationFunction<Volt::EntityID>(s_typeDeserializers);
		RegisterDeserializationFunction<AssetHandle>(s_typeDeserializers);

		s_instance = this;
	}

	SceneImporter::~SceneImporter()
	{
		s_instance = nullptr;
	}

	bool SceneImporter::Load(const AssetMetadata& metadata, Ref<Asset>& asset) const
	{
		VT_PROFILE_FUNCTION();

		asset = CreateRef<Scene>();
		Ref<Scene> scene = reinterpret_pointer_cast<Scene>(asset);

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

		streamReader.EnterScope("Scene");
		scene->m_name = streamReader.ReadAtKey("name", std::string("New Scene"));

		streamReader.EnterScope("Settings");
		scene->m_sceneSettings.useWorldEngine = streamReader.ReadAtKey("useWorldEngine", false);
		streamReader.ExitScope();

		if (scene->m_sceneSettings.useWorldEngine)
		{
			DeserializeWorldEngine(scene, streamReader);
		}

		streamReader.ExitScope();

		const std::filesystem::path& scenePath = filePath;
		std::filesystem::path directoryPath = scenePath.parent_path();

		if (scene->m_sceneSettings.useWorldEngine)
		{
			LoadCellEntities(metadata, scene, directoryPath);
			//LoadEntities(metadata, scene, directoryPath);
		}
		else
		{
			LoadSceneLayers(metadata, scene, directoryPath);
		}

		scene->SortScene();
		scene->InvalidateRenderScene();
		return true;
	}

	void SceneImporter::Save(const AssetMetadata& metadata, const Ref<Asset>& asset) const
	{
		const Ref<Scene> scene = std::reinterpret_pointer_cast<Scene>(asset);

		std::filesystem::path directoryPath = AssetManager::GetFilesystemPath(metadata.filePath);
		if (!std::filesystem::is_directory(directoryPath))
		{
			directoryPath = directoryPath.parent_path();
		}

		std::filesystem::path scenePath = directoryPath / (metadata.filePath.stem().string() + ".vtscene");

		YAMLFileStreamWriter streamWriter{ scenePath };
		streamWriter.BeginMap();
		streamWriter.BeginMapNamned("Scene");
		streamWriter.SetKey("name", metadata.filePath.stem().string());

		streamWriter.BeginMapNamned("Settings");
		streamWriter.SetKey("useWorldEngine", scene->m_sceneSettings.useWorldEngine);
		streamWriter.EndMap();

		if (scene->m_sceneSettings.useWorldEngine)
		{
			SerializeWorldEngine(scene, streamWriter);
		}

		streamWriter.EndMap();
		streamWriter.EndMap();
		streamWriter.WriteToDisk();

		if (scene->m_sceneSettings.useWorldEngine)
		{
			SaveEntities(metadata, scene, directoryPath);
		}
		else
		{
			SaveSceneLayers(metadata, scene, directoryPath);
		}
	}

	void SceneImporter::LoadSceneLayers(const AssetMetadata& metadata, const Ref<Scene>& scene, const std::filesystem::path& sceneDirectory) const
	{
		VT_PROFILE_FUNCTION();

		std::filesystem::path layersFolderPath = sceneDirectory / "Layers";
		if (!std::filesystem::exists(layersFolderPath))
		{
			return;
		}

		std::vector<std::filesystem::path> layerPaths;

		for (const auto& it : std::filesystem::directory_iterator(layersFolderPath))
		{
			if (!it.is_directory() && it.path().extension().string() == ".vtlayer")
			{
				layerPaths.emplace_back(it.path());
			}
		}

		std::vector<SceneLayer> sceneLayers;

		for (const auto& layerPath : layerPaths)
		{
			VT_PROFILE_SCOPE("Layer");

			auto& sceneLayer = sceneLayers.emplace_back();

			YAMLFileStreamReader streamReader{};
			if (!streamReader.OpenFile(layerPath))
			{
				continue;
			}

			streamReader.EnterScope("Layer");
			{
				sceneLayer.name = streamReader.ReadAtKey("name", std::string("Null"));
				sceneLayer.id = streamReader.ReadAtKey("id", 0u);
				sceneLayer.visible = streamReader.ReadAtKey("visible", true);
				sceneLayer.locked = streamReader.ReadAtKey("locked", false);

				streamReader.ForEach("Entities", [&]()
				{
					DeserializeEntity(scene, metadata, streamReader);
				});
			}
			streamReader.ExitScope();
		}

		for (const auto& layer : sceneLayers)
		{
			scene->m_lastLayerId = std::max(scene->m_lastLayerId, layer.id);
		}

		scene->m_sceneLayers = sceneLayers;
	}

	void SceneImporter::SaveSceneLayers(const AssetMetadata& metadata, const Ref<Scene>& scene, const std::filesystem::path& sceneDirectory) const
	{
		std::filesystem::path layerFolderPath = sceneDirectory / "Layers";
		if (!std::filesystem::exists(layerFolderPath))
		{
			std::filesystem::create_directories(layerFolderPath);
		}

		for (const auto& layer : scene->GetLayers())
		{
			const auto layerPath = layerFolderPath / ("layer_" + std::to_string(layer.id) + ".vtlayer");

			YAMLFileStreamWriter streamWriter{ layerPath };
			streamWriter.BeginMap();
			streamWriter.BeginMapNamned("Layer");
			{
				streamWriter.SetKey("name", layer.name);
				streamWriter.SetKey("id", layer.id);
				streamWriter.SetKey("visible", layer.visible);
				streamWriter.SetKey("locked", layer.locked);

				streamWriter.BeginSequence("Entities");
				{
					for (const auto entity : scene->GetAllEntities())
					{
						const uint32_t layerId = entity.GetComponent<CommonComponent>().layerId;

						if (layerId != layer.id)
						{
							continue;
						}

						SerializeEntity(entity, metadata, scene, streamWriter);
					}
				}
				streamWriter.EndSequence();
			}
			streamWriter.EndMap();
			streamWriter.EndMap();
			streamWriter.WriteToDisk();
		}
	}

	void SceneImporter::SaveEntities(const AssetMetadata& metadata, const Ref<Scene>& scene, const std::filesystem::path& sceneDirectory) const
	{
		std::filesystem::path entitiesDirectoryPath = sceneDirectory / "Entities";
		if (!std::filesystem::exists(entitiesDirectoryPath))
		{
			std::filesystem::create_directories(entitiesDirectoryPath);
		}

		const auto entities = scene->GetAllEditedEntities();
		if (!entities.empty())
		{
			Algo::ForEachParallel([entitiesDirectoryPath, entities, metadata, scene, this](uint32_t threadIdx, uint32_t i)
			{
				Entity entity = entities.at(i);
				const auto entityPath = entitiesDirectoryPath / (entity.ToString() + ".entity");

				YAMLFileStreamWriter streamWriter{ entityPath };
				SerializeEntity(entity, metadata, scene, streamWriter);

				streamWriter.WriteToDisk();
			},
			static_cast<uint32_t>(entities.size()));
		}

		const auto removedEntities = scene->GetAllRemovedEntities();
		if (!removedEntities.empty())
		{
			Algo::ForEachParallel([removedEntities, entitiesDirectoryPath](uint32_t threadIdx, uint32_t i)
			{
				EntityID id = removedEntities.at(i);

				const std::filesystem::path entityFilePath = entitiesDirectoryPath / (std::to_string(id) + ".entity");
				if (std::filesystem::exists(entityFilePath))
				{
					FileSystem::MoveToRecycleBin(entityFilePath);
				}

			}, static_cast<uint32_t>(removedEntities.size()));
		}

		VT_CORE_INFO("[SceneImporter]: Saved {0} entities!", entities.size());
		VT_CORE_INFO("[SceneImporter]: Removed {0} entities!", removedEntities.size());

		scene->ClearEditedEntities();
	}

	void SceneImporter::LoadEntities(const AssetMetadata& metadata, const Ref<Scene>& scene, const std::filesystem::path& sceneDirectory) const
	{
		VT_PROFILE_FUNCTION();

		std::filesystem::path layersFolderPath = sceneDirectory / "Entities";
		if (!std::filesystem::exists(layersFolderPath))
		{
			return;
		}

		std::vector<std::filesystem::path> entityPaths;

		for (const auto& it : std::filesystem::directory_iterator(layersFolderPath))
		{
			if (!it.is_directory() && it.path().extension().string() == ".entity")
			{
				entityPaths.emplace_back(it.path());
			}
		}

		if (entityPaths.empty())
		{
			return;
		}

		const uint32_t iterationCount = static_cast<uint32_t>(entityPaths.size());

		std::vector<Ref<Scene>> dummyScenes{};
		dummyScenes.resize(iterationCount);

		auto futures = Algo::ForEachParallelLockable([&dummyScenes, entityPaths, metadata, this](uint32_t threadIdx, uint32_t i)
		{
			Ref<Scene> dummyScene = CreateRef<Scene>();
			dummyScenes[threadIdx] = dummyScene;

			const auto& path = entityPaths.at(i);

			YAMLFileStreamReader streamReader{};
			if (!streamReader.OpenFile(path))
			{
				return;
			}

			DeserializeEntity(dummyScene, metadata, streamReader);
		},
		static_cast<uint32_t>(entityPaths.size()));

		for (auto& f : futures)
		{
			f.wait();
		}

		for (const auto& dummyScene : dummyScenes)
		{
			for (const auto& entity : dummyScene->GetAllEntities())
			{
				Entity realEntity = scene->CreateEntityWithUUID(entity.GetID());
				Entity::Copy(entity, realEntity, Volt::EntityCopyFlags::None);
			}
		}
	}

	void SceneImporter::SerializeWorldEngine(const Ref<Scene>& scene, YAMLFileStreamWriter& streamWriter) const
	{
		auto& worldEngine = scene->m_worldEngine;

		streamWriter.BeginMapNamned("WorldEngine");
		streamWriter.SetKey("cellSize", worldEngine.GetSettings().cellSize);
		streamWriter.SetKey("worldSize", worldEngine.GetSettings().worldSize);

		streamWriter.EndMap();
	}

	void SceneImporter::DeserializeWorldEngine(const Ref<Scene>& scene, YAMLFileStreamReader& streamReader) const
	{
		auto& worldEngine = scene->m_worldEngine;

		streamReader.EnterScope("WorldEngine");
		worldEngine.GetSettingsMutable().cellSize = streamReader.ReadAtKey("cellSize", 256);
		worldEngine.GetSettingsMutable().worldSize = streamReader.ReadAtKey("worldSize", glm::uvec2{ 1280 });
		
		worldEngine.GenerateCells();

		streamReader.ExitScope();
	}

	void SceneImporter::LoadCellEntities(const AssetMetadata& metadata, const Ref<Scene>& scene, const std::filesystem::path& sceneDirectory) const
	{
		VT_PROFILE_FUNCTION();

		std::filesystem::path layersFolderPath = sceneDirectory / "Entities";
		if (!std::filesystem::exists(layersFolderPath))
		{
			return;
		}

		std::vector<std::filesystem::path> entityPaths;

		for (const auto& it : std::filesystem::directory_iterator(layersFolderPath))
		{
			if (!it.is_directory() && it.path().extension().string() == ".entity")
			{
				entityPaths.emplace_back(it.path());
			}
		}

		if (entityPaths.empty())
		{
			return;
		}

		const uint32_t iterationCount = static_cast<uint32_t>(entityPaths.size());

		std::vector<std::unordered_map<WorldCellID, std::vector<EntityID>>> threadCellEntities{};
		threadCellEntities.resize(Algo::GetThreadCountFromIterationCount(iterationCount));

		auto futures = Algo::ForEachParallelLockable([&threadCellEntities, entityPaths, metadata, this](uint32_t threadIdx, uint32_t i)
		{
			const auto& path = entityPaths.at(i);

			YAMLFileStreamReader streamReader{};
			if (!streamReader.OpenFile(path))
			{
				return;
			}

			streamReader.EnterScope("Entity");

			EntityID entityId = streamReader.ReadAtKey("id", Entity::NullID());
			WorldCellID cellId = streamReader.ReadAtKey("cellId", INVALID_WORLD_CELL_ID);

			if (entityId == Entity::NullID())
			{
				return;
			}

			if (cellId == INVALID_WORLD_CELL_ID)
			{
				cellId = 0;
			}

			streamReader.ExitScope();
		
			threadCellEntities[threadIdx][cellId].push_back(entityId);
		},
		iterationCount);

		for (auto& f : futures)
		{
			f.wait();
		}

		auto& worldEngine = scene->m_worldEngine;
		for (const auto& tCellEntities : threadCellEntities)
		{
			for (const auto& [cellId, entities] : tCellEntities)
			{
				worldEngine.AddEntitiesToCell(cellId, entities);
			}
		}
	}

	Entity SceneImporter::CreateEntityFromUUIDThreadSafe(EntityID entityId, const Ref<Scene>& scene) const
	{
		static std::mutex createEntityMutex;
		std::scoped_lock lock{ createEntityMutex };

		return scene->CreateEntityWithUUID(entityId);
	}

	void SceneImporter::SerializeEntity(entt::entity id, const AssetMetadata& metadata, const Ref<Scene>& scene, YAMLFileStreamWriter& streamWriter) const
	{
		streamWriter.BeginMap();
		streamWriter.BeginMapNamned("Entity");

		auto& registry = scene->GetRegistry();

		Entity entity{ id, scene };

		streamWriter.SetKey("id", entity.GetID());
		streamWriter.SetKey("cellId", scene->m_worldEngine.GetCellIDFromEntity(entity));
		streamWriter.BeginSequence("components");
		{
			for (auto&& curr : registry.storage())
			{
				auto& storage = curr.second;

				if (!storage.contains(id))
				{
					continue;
				}

				const IComponentTypeDesc* componentDesc = reinterpret_cast<const IComponentTypeDesc*>(ComponentRegistry::GetTypeDescFromName(storage.type().name()));
				if (!componentDesc)
				{
					continue;
				}

				const uint8_t* componentPtr = reinterpret_cast<const uint8_t*>(storage.get(id));
				SerializeClass(componentPtr, 0, componentDesc, streamWriter, false);
			}
		}
		streamWriter.EndSequence();

		if (registry.any_of<MonoScriptComponent>(id))
		{
			SerializeMono(id, scene, streamWriter);
		}

		if (registry.any_of<VertexPaintedComponent>(id))
		{
			std::filesystem::path vpPath = (ProjectManager::GetDirectory() / metadata.filePath.parent_path() / "Layers" / ("ent_" + std::to_string((uint32_t)registry.get<IDComponent>(id).id) + ".entVp"));
			auto& vpComp = registry.get<VertexPaintedComponent>(id);

			// #TODO_Ivar: This is kind of questionable after TGA
			if (std::filesystem::exists(vpPath))
			{
				using std::filesystem::perms;
				std::filesystem::permissions(vpPath, perms::_All_write);
			}

			BinarySerializer binaryVp(vpPath, sizeof(uint32_t) * vpComp.vertexColors.size() + sizeof(vpComp.meshHandle));

			binaryVp.Serialize(vpComp.vertexColors.data(), sizeof(uint32_t) * vpComp.vertexColors.size());
			binaryVp.Serialize(vpComp.meshHandle);
			binaryVp.WriteToFile();
		}

		streamWriter.EndMap();
		streamWriter.EndMap();
	}

	void SceneImporter::SerializeClass(const uint8_t* data, const size_t offset, const IComponentTypeDesc* compDesc, YAMLFileStreamWriter& streamWriter, bool isSubComponent) const
	{
		if (!isSubComponent)
		{
			streamWriter.BeginMap();
		}

		streamWriter.SetKey("guid", compDesc->GetGUID());
		streamWriter.BeginSequence("members");

		for (const auto& member : compDesc->GetMembers())
		{
			if ((member.flags & ComponentMemberFlag::NoSerialize) != ComponentMemberFlag::None)
			{
				continue;
			}

			streamWriter.BeginMap();
			streamWriter.SetKey("name", member.name);

			if (member.typeDesc != nullptr)
			{
				switch (member.typeDesc->GetValueType())
				{
					case ValueType::Component:
						streamWriter.SetKey("data", "component");
						SerializeClass(data, offset + member.offset, reinterpret_cast<const IComponentTypeDesc*>(member.typeDesc), streamWriter, true);
						break;

					case ValueType::Enum:
						streamWriter.SetKey("data", "enum");
						streamWriter.SetKey("enumValue", *reinterpret_cast<const int32_t*>(&data[offset + member.offset]));
						break;

					case ValueType::Array:
					{
						streamWriter.SetKey("data", "array");
						SerializeArray(data, offset + member.offset, reinterpret_cast<const IArrayTypeDesc*>(member.typeDesc), streamWriter);
						break;
					}
				}
			}
			else
			{
				if (s_typeSerializers.contains(member.typeIndex))
				{
					s_typeSerializers.at(member.typeIndex)(streamWriter, data, offset + member.offset);
				}
			}

			streamWriter.EndMap();
		}

		streamWriter.EndSequence();

		if (!isSubComponent)
		{
			streamWriter.EndMap();
		}
	}

	void SceneImporter::SerializeArray(const uint8_t* data, const size_t offset, const IArrayTypeDesc* arrayDesc, YAMLFileStreamWriter& streamWriter) const
	{
		const void* arrayPtr = &data[offset];

		const bool isNonDefaultType = arrayDesc->GetElementTypeDesc() != nullptr;
		const auto& typeIndex = arrayDesc->GetElementTypeIndex();

		if (!isNonDefaultType && !s_typeSerializers.contains(typeIndex))
		{
			return;
		}

		streamWriter.BeginSequence("values");
		for (size_t i = 0; i < arrayDesc->Size(arrayPtr); i++)
		{
			const uint8_t* elementData = reinterpret_cast<const uint8_t*>(arrayDesc->At(arrayPtr, i));

			streamWriter.BeginMap();
			if (isNonDefaultType)
			{
				switch (arrayDesc->GetElementTypeDesc()->GetValueType())
				{
					case ValueType::Component:
						streamWriter.SetKey("value", "component");
						SerializeClass(elementData, 0, reinterpret_cast<const IComponentTypeDesc*>(arrayDesc->GetElementTypeDesc()), streamWriter, true);
						break;

					case ValueType::Enum:
						streamWriter.SetKey("value", *reinterpret_cast<const int32_t*>(elementData));
						break;

					case ValueType::Array:
						streamWriter.SetKey("value", "array");
						SerializeArray(elementData, 0, reinterpret_cast<const IArrayTypeDesc*>(arrayDesc->GetElementTypeDesc()), streamWriter);
						break;
				}
			}
			else
			{
				if (s_typeSerializers.contains(typeIndex))
				{
					s_typeSerializers.at(typeIndex)(streamWriter, elementData, 0);
				}
			}
			streamWriter.EndMap();
		}
		streamWriter.EndSequence();
	}

	void SceneImporter::SerializeMono(entt::entity id, const Ref<Scene>& scene, YAMLFileStreamWriter& streamWriter) const
	{
		const auto& scriptFieldCache = scene->GetScriptFieldCache();

		streamWriter.BeginSequence("MonoScripts");
		{
			MonoScriptComponent& monoScriptComp = scene->GetRegistry().get<MonoScriptComponent>(id);
			if (monoScriptComp.scriptIds.size() == monoScriptComp.scriptNames.size())
			{
				for (size_t i = 0; i < monoScriptComp.scriptIds.size(); ++i)
				{
					const auto& scriptId = monoScriptComp.scriptIds.at(i);
					const auto& scriptName = monoScriptComp.scriptNames.at(i);

					streamWriter.BeginMap();
					streamWriter.BeginMapNamned("ScriptEntry");

					streamWriter.SetKey("name", scriptName);
					streamWriter.SetKey("id", scriptId);

					if (scriptFieldCache.GetCache().contains(scriptId))
					{
						streamWriter.BeginSequence("members");
						const auto& fieldMap = scriptFieldCache.GetCache().at(scriptId);
						for (const auto& [name, value] : fieldMap)
						{
							streamWriter.BeginMap();
							streamWriter.SetKey("name", name);

							if (value->field.type.IsString())
							{
								auto cStr = value->data.As<const char>();
								std::string str(cStr);

								streamWriter.SetKey("data", str);
							}
							else
							{
								if (s_typeSerializers.contains(value->field.type.typeIndex))
								{
									s_typeSerializers.at(value->field.type.typeIndex)(streamWriter, value->data.As<const uint8_t>(), 0);
								}
							}

							streamWriter.EndMap();
						}

						streamWriter.EndSequence();
					}

					streamWriter.EndMap();
					streamWriter.EndMap();
				}
			}
		}
		streamWriter.EndSequence();
	}

	void SceneImporter::DeserializeEntity(const Ref<Scene>& scene, const AssetMetadata& metadata, YAMLFileStreamReader& streamReader) const
	{
		VT_PROFILE_FUNCTION();

		streamReader.EnterScope("Entity");

		EntityID entityId = streamReader.ReadAtKey("id", Entity::NullID());

		if (entityId == Entity::NullID())
		{
			return;
		}

		auto entity = CreateEntityFromUUIDThreadSafe(entityId, scene);

		streamReader.ForEach("components", [&]()
		{
			VT_PROFILE_SCOPE("Component");

			VoltGUID compGuid = streamReader.ReadAtKey("guid", VoltGUID::Null());
			if (compGuid == VoltGUID::Null())
			{
				return;
			}

			const ICommonTypeDesc* typeDesc = ComponentRegistry::GetTypeDescFromGUID(compGuid);
			if (!typeDesc)
			{
				return;
			}

			switch (typeDesc->GetValueType())
			{
				case ValueType::Component:
				{
					if (!ComponentRegistry::Helpers::HasComponentWithGUID(compGuid, scene->GetRegistry(), entity))
					{
						ComponentRegistry::Helpers::AddComponentWithGUID(compGuid, scene->GetRegistry(), entity);
					}

					void* voidCompPtr = ComponentRegistry::Helpers::GetComponentWithGUID(compGuid, scene->GetRegistry(), entity);
					uint8_t* componentData = reinterpret_cast<uint8_t*>(voidCompPtr);

					const IComponentTypeDesc* componentDesc = reinterpret_cast<const IComponentTypeDesc*>(typeDesc);
					DeserializeClass(componentData, 0, componentDesc, streamReader);
					break;
				}
			}

		});

		if (scene->GetRegistry().any_of<MonoScriptComponent>(entity))
		{
			DeserializeMono(entity, scene, streamReader);
		}

		if (scene->GetRegistry().any_of<PrefabComponent>(entity))
		{
			auto& prefabComp = scene->GetRegistry().get<PrefabComponent>(entity);
		
			// We need to check that it's not the entity being referenced in the prefab, as this means we are loading the prefab
			if (entity.GetID() != prefabComp.prefabEntity)
			{
				auto prefab = AssetManager::GetAsset<Prefab>(prefabComp.prefabAsset);
				if (prefab && prefab->IsValid())
				{
					if (prefab->GetVersion() > prefabComp.version)
					{
						prefab->UpdateEntityInScene(entity);
					}
				}
			}
		}

		if (scene->GetRegistry().any_of<VertexPaintedComponent>(entity))
		{
			std::filesystem::path vpPath = metadata.filePath.parent_path();
			vpPath = ProjectManager::GetDirectory() / vpPath / "Layers" / ("ent_" + std::to_string((uint32_t)entityId) + ".entVp");

			if (std::filesystem::exists(vpPath))
			{
				auto& vpComp = scene->GetRegistry().get<VertexPaintedComponent>(entity);

				std::ifstream vpFile(vpPath, std::ios::in | std::ios::binary);
				if (!vpFile.is_open())
				{
					VT_CORE_ERROR("Could not open entVp file!");
				}

				std::vector<uint8_t> totalData;
				const size_t srcSize = vpFile.seekg(0, std::ios::end).tellg();
				totalData.resize(srcSize);
				vpFile.seekg(0, std::ios::beg);
				vpFile.read(reinterpret_cast<char*>(totalData.data()), totalData.size());
				vpFile.close();

				memcpy_s(&vpComp.meshHandle, sizeof(vpComp.meshHandle), totalData.data() + totalData.size() - sizeof(vpComp.meshHandle), sizeof(vpComp.meshHandle));
				totalData.resize(totalData.size() - sizeof(vpComp.meshHandle));

				vpComp.vertexColors.reserve(totalData.size() / sizeof(uint32_t));
				for (size_t offset = 0; offset < totalData.size(); offset += sizeof(uint32_t))
				{
					uint32_t vpColor;
					memcpy_s(&vpColor, sizeof(uint32_t), totalData.data() + offset, sizeof(uint32_t));
					vpComp.vertexColors.push_back(vpColor);
				}
			}
		}

		streamReader.ExitScope();
	}

	void SceneImporter::DeserializeClass(uint8_t* data, const size_t offset, const IComponentTypeDesc* compDesc, YAMLFileStreamReader& streamReader) const
	{
		VT_PROFILE_FUNCTION();

		streamReader.ForEach("members", [&]()
		{
			const std::string memberName = streamReader.ReadAtKey("name", std::string(""));
			if (memberName.empty())
			{
				return;
			}

			const ComponentMember* componentMember = const_cast<IComponentTypeDesc*>(compDesc)->FindMemberByName(memberName);
			if (!componentMember)
			{
				return;
			}

			if (componentMember->typeDesc != nullptr)
			{
				switch (componentMember->typeDesc->GetValueType())
				{
					case ValueType::Component:
					{
						const IComponentTypeDesc* memberCompType = reinterpret_cast<const IComponentTypeDesc*>(componentMember->typeDesc);
						DeserializeClass(data, offset + componentMember->offset, memberCompType, streamReader);
						break;
					}

					case ValueType::Enum:
					{
						*reinterpret_cast<int32_t*>(&data[offset + componentMember->offset]) = streamReader.ReadAtKey("enumValue", int32_t(0));
						break;
					}

					case ValueType::Array:
					{
						const IArrayTypeDesc* arrayTypeDesc = reinterpret_cast<const IArrayTypeDesc*>(componentMember->typeDesc);
						DeserializeArray(data, offset + componentMember->offset, arrayTypeDesc, streamReader);
						break;
					}
				}
			}
			else
			{
				if (s_typeDeserializers.contains(componentMember->typeIndex))
				{
					s_typeDeserializers.at(componentMember->typeIndex)(streamReader, data, offset + componentMember->offset);
				}
			}
		});
	}

	void SceneImporter::DeserializeArray(uint8_t* data, const size_t offset, const IArrayTypeDesc* arrayDesc, YAMLFileStreamReader& streamReader) const
	{
		VT_PROFILE_FUNCTION();

		void* arrayPtr = &data[offset];

		const bool isNonDefaultType = arrayDesc->GetElementTypeDesc() != nullptr;
		const auto& typeIndex = arrayDesc->GetElementTypeIndex();

		if (!isNonDefaultType && !s_typeSerializers.contains(typeIndex))
		{
			return;
		}

		streamReader.ForEach("values", [&]()
		{
			void* tempDataStorage = nullptr;
			arrayDesc->DefaultConstructElement(tempDataStorage);

			uint8_t* tempBytePtr = reinterpret_cast<uint8_t*>(tempDataStorage);

			if (isNonDefaultType)
			{
				switch (arrayDesc->GetElementTypeDesc()->GetValueType())
				{
					case ValueType::Component:
					{
						const IComponentTypeDesc* compType = reinterpret_cast<const IComponentTypeDesc*>(arrayDesc->GetElementTypeDesc());
						DeserializeClass(tempBytePtr, 0, compType, streamReader);
						break;
					}

					case ValueType::Enum:
						*reinterpret_cast<int32_t*>(&tempDataStorage) = streamReader.ReadAtKey("enumValue", int32_t(0));
						break;

					case ValueType::Array:
					{
						const IArrayTypeDesc* arrayTypeDesc = reinterpret_cast<const IArrayTypeDesc*>(arrayDesc->GetElementTypeDesc());
						DeserializeArray(tempBytePtr, 0, arrayTypeDesc, streamReader);
						break;
					}
				}
			}
			else
			{
				if (s_typeDeserializers.contains(typeIndex))
				{
					s_typeDeserializers.at(typeIndex)(streamReader, tempBytePtr, 0);
				}
			}

			arrayDesc->PushBack(arrayPtr, tempDataStorage);
			arrayDesc->DestroyElement(tempDataStorage);
		});
	}

	void SceneImporter::DeserializeMono(entt::entity id, const Ref<Scene>& scene, YAMLFileStreamReader& streamReader) const
	{
		VT_PROFILE_FUNCTION();

		Entity entity{ id, scene };

		streamReader.ForEach("MonoScripts", [&]()
		{
			streamReader.EnterScope("ScriptEntry");

			if (!entity.HasComponent<MonoScriptComponent>())
			{
				entity.AddComponent<MonoScriptComponent>();
			}

			const std::string scriptName = streamReader.ReadAtKey("name", std::string(""));
			const UUID64 scriptId = streamReader.ReadAtKey("id", UUID64(0));

			auto scriptClass = MonoScriptEngine::GetScriptClass(scriptName);
			if (!scriptClass)
			{
				streamReader.ExitScope();
				return;
			}

			auto& scriptComp = entity.GetComponent<MonoScriptComponent>();
			scriptComp.scriptIds.emplace_back(scriptId);
			scriptComp.scriptNames.emplace_back(scriptName);

			const auto& classFields = scriptClass->GetFields();
			auto& fieldCache = scene->GetScriptFieldCache().GetCache()[scriptId];

			streamReader.ForEach("members", [&]()
			{
				const std::string memberName = streamReader.ReadAtKey("name", std::string(""));

				if (!classFields.contains(memberName))
				{
					return;
				}

				fieldCache[memberName] = CreateRef<MonoScriptFieldInstance>();

				auto field = fieldCache[memberName];
				field->field = classFields.at(memberName);

				if (field->field.type.IsString())
				{
					const std::string str = streamReader.ReadAtKey("data", std::string(""));
					field->SetValue(str, str.size());
				}
				else
				{
					if (s_typeDeserializers.contains(field->field.type.typeIndex))
					{
						field->data.Allocate(field->field.type.typeSize);
						s_typeDeserializers.at(field->field.type.typeIndex)(streamReader, field->data.As<uint8_t>(), 0);
					}
				}
			});
			streamReader.ExitScope();
		});
	}

	void SceneImporter::LoadWorldCell(const Ref<Scene>& scene, const WorldCell& worldCell) const
	{
		VT_PROFILE_FUNCTION();

		if (worldCell.isLoaded)
		{
			VT_CORE_WARN("[SceneImporter]: World Cell is already loaded!");
			return;
		}

		if (worldCell.cellEntities.empty())
		{
			VT_CORE_WARN("[SceneImporter]: Unable to load World Cell which contains zero entities!");
			return;
		}

		const auto& metadata = AssetManager::GetMetadataFromHandle(scene->handle);
		const auto filePath = AssetManager::GetFilesystemPath(metadata.filePath);
		const std::filesystem::path sceneDirectory = filePath.parent_path();

		std::filesystem::path layersFolderPath = sceneDirectory / "Entities";
		if (!std::filesystem::exists(layersFolderPath))
		{
			return;
		}

		std::vector<std::filesystem::path> entityPaths;

		for (const auto& it : std::filesystem::directory_iterator(layersFolderPath))
		{
			if (!it.is_directory() && it.path().extension().string() == ".entity")
			{
				entityPaths.emplace_back(it.path());
			}
		}

		for (int32_t i = static_cast<int32_t>(entityPaths.size()) - 1; i >= 0; i--)
		{
			const auto& path = entityPaths.at(i);

			const std::string stem = path.stem().string();
			uint32_t entityId = std::stoul(stem);
			auto it = std::ranges::find(worldCell.cellEntities, EntityID(entityId));

			if (it == worldCell.cellEntities.end())
			{
				entityPaths.erase(entityPaths.begin() + i);
			}
		}

		if (entityPaths.empty())
		{
			return;
		}

		const uint32_t iterationCount = static_cast<uint32_t>(entityPaths.size());

		std::vector<Ref<Scene>> dummyScenes{};
		dummyScenes.resize(iterationCount);

		auto futures = Algo::ForEachParallelLockable([&dummyScenes, entityPaths, metadata, this](uint32_t threadIdx, uint32_t i)
		{
			Ref<Scene> dummyScene = CreateRef<Scene>();
			dummyScenes[threadIdx] = dummyScene;

			const auto& path = entityPaths.at(i);

			YAMLFileStreamReader streamReader{};
			if (!streamReader.OpenFile(path))
			{
				return;
			}

			DeserializeEntity(dummyScene, metadata, streamReader);
		},
		static_cast<uint32_t>(entityPaths.size()));

		for (auto& f : futures)
		{
			f.wait();
		}

		for (const auto& dummyScene : dummyScenes)
		{
			for (const auto& entity : dummyScene->GetAllEntities())
			{
				Entity realEntity = scene->CreateEntityWithUUID(entity.GetID());
				Entity::Copy(entity, realEntity, Volt::EntityCopyFlags::None);
			}
		}
	}
}
