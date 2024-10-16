#include "vtpch.h"
#include "Volt/Asset/Serializers/SceneSerializer.h"

#include <AssetSystem/AssetManager.h>

#include "Volt/Components/CoreComponents.h"
#include "Volt/Components/RenderingComponents.h"

#include "Volt/Scene/Scene.h"
#include "Volt/Scene/Entity.h"

#include "Volt/Utility/Algorithms.h"
#include "Volt/Utility/YAMLSerializationHelpers.h"

#include <Volt-Core/Project/ProjectManager.h>

#include <EntitySystem/ComponentRegistry.h>

#include <CoreUtilities/FileIO/YAMLMemoryStreamWriter.h>
#include <CoreUtilities/FileIO/YAMLMemoryStreamReader.h>
#include <CoreUtilities/FileSystem.h>

namespace Volt
{
	static constexpr const char* ENTITY_FILE_EXTENSION = ".vtent";

	template<typename T>
	void RegisterSerializationFunction(std::unordered_map<TypeTraits::TypeIndex, std::function<void(YAMLMemoryStreamWriter&, const uint8_t*, const size_t)>>& outTypes)
	{
		VT_PROFILE_FUNCTION();

		outTypes[TypeTraits::TypeIndex::FromType<T>()] = [](YAMLMemoryStreamWriter& streamWriter, const uint8_t* data, const size_t offset)
		{
			const T& var = *reinterpret_cast<const T*>(&data[offset]);
			streamWriter.SetKey("data", var);
		};
	}

	template<typename T>
	void RegisterDeserializationFunction(std::unordered_map<TypeTraits::TypeIndex, std::function<void(YAMLMemoryStreamReader&, uint8_t*, const size_t)>>& outTypes)
	{
		VT_PROFILE_FUNCTION();

		outTypes[TypeTraits::TypeIndex::FromType<T>()] = [](YAMLMemoryStreamReader& streamReader, uint8_t* data, const size_t offset)
		{
			*reinterpret_cast<T*>(&data[offset]) = streamReader.ReadAtKey("data", T());
		};
	}

	SceneSerializer::SceneSerializer()
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

	SceneSerializer::~SceneSerializer()
	{
		s_instance = nullptr;
	}

	void SceneSerializer::Serialize(const AssetMetadata& metadata, const Ref<Asset>& asset) const
	{
		const Ref<Scene> scene = std::reinterpret_pointer_cast<Scene>(asset);

		std::filesystem::path directoryPath = AssetManager::GetFilesystemPath(metadata.filePath);
		if (!std::filesystem::is_directory(directoryPath))
		{
			directoryPath = directoryPath.parent_path();
		}

		if (!std::filesystem::exists(directoryPath))
		{
			std::filesystem::create_directories(directoryPath);
		}

		std::filesystem::path scenePath = directoryPath / (metadata.filePath.stem().string() + ".vtasset");

		// Serialize scene file
		{
			YAMLMemoryStreamWriter streamWriter{};
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
		
			BinaryStreamWriter sceneFileWriter{};
			const size_t compressedDataOffset = AssetSerializer::WriteMetadata(metadata, asset->GetVersion(), sceneFileWriter);

			auto buffer = streamWriter.WriteAndGetBuffer();
			sceneFileWriter.Write(buffer);
			buffer.Release();

			sceneFileWriter.WriteToDisk(scenePath, true, compressedDataOffset);
		}

		SerializeEntities(metadata, scene, directoryPath);
	}

	bool SceneSerializer::Deserialize(const AssetMetadata& metadata, Ref<Asset> destinationAsset) const
	{
		Ref<Scene> scene = reinterpret_pointer_cast<Scene>(destinationAsset);

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
			VT_LOG(Error, "Failed to open file {0}!", metadata.filePath);
			destinationAsset->SetFlag(AssetFlag::Invalid, true);
			return false;
		}

		SerializedAssetMetadata serializedMetadata = AssetSerializer::ReadMetadata(streamReader);
		VT_ASSERT_MSG(serializedMetadata.version == destinationAsset->GetVersion(), "Incompatible version!");

		// Scene File
		{
			Buffer buffer{};
			streamReader.Read(buffer);

			YAMLMemoryStreamReader yamlStreamReader{};
			if (!yamlStreamReader.ConsumeBuffer(buffer))
			{
				destinationAsset->SetFlag(AssetFlag::Invalid, true);
				return false;
			}

			yamlStreamReader.EnterScope("Scene");
			scene->m_name = yamlStreamReader.ReadAtKey("name", std::string("New Scene"));

			yamlStreamReader.EnterScope("Settings");
			{
				scene->m_sceneSettings.useWorldEngine = yamlStreamReader.ReadAtKey("useWorldEngine", true);
			}
			yamlStreamReader.ExitScope();

			if (scene->m_sceneSettings.useWorldEngine)
			{
				DeserializeWorldEngine(scene, yamlStreamReader);
			}

			yamlStreamReader.ExitScope();
		}

		const std::filesystem::path& scenePath = filePath;
		std::filesystem::path directoryPath = scenePath.parent_path();

		LoadCellEntities(metadata, scene, directoryPath);

		scene->SortScene();
		return true;
	}

	void SceneSerializer::SerializeEntity(entt::entity id, const AssetMetadata& metadata, const Ref<Scene>& scene, YAMLMemoryStreamWriter& streamWriter) const
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

				const IComponentTypeDesc* componentDesc = reinterpret_cast<const IComponentTypeDesc*>(GetComponentRegistry().GetTypeDescFromName(storage.type().name()));
				if (!componentDesc)
				{
					continue;
				}

				const uint8_t* componentPtr = reinterpret_cast<const uint8_t*>(storage.get(id));
				SerializeClass(componentPtr, 0, componentDesc, streamWriter, false);
			}
		}
		streamWriter.EndSequence();

		if (registry.any_of<VertexPaintedComponent>(id))
		{
			std::filesystem::path vpPath = (ProjectManager::GetRootDirectory() / metadata.filePath.parent_path() / "Layers" / ("ent_" + std::to_string((uint32_t)id) + ".entVp"));
			auto& vpComp = registry.get<VertexPaintedComponent>(id);

			// #TODO_Ivar: This is kind of questionable after TGA
			if (std::filesystem::exists(vpPath))
			{
				using std::filesystem::perms;
				std::filesystem::permissions(vpPath, perms::_All_write);
			}

			BinaryStreamWriter vpStreamWriter;

			vpStreamWriter.Write(vpComp.vertexColors.data(), sizeof(uint32_t) * vpComp.vertexColors.size());
			vpStreamWriter.Write(vpComp.meshHandle);

			vpStreamWriter.WriteToDisk(vpPath, false, 0);
		}

		streamWriter.EndMap();
		streamWriter.EndMap();
	}

	void SceneSerializer::DeserializeEntity(const Ref<Scene>& scene, const AssetMetadata& metadata, YAMLMemoryStreamReader& streamReader) const
	{
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

			const ICommonTypeDesc* typeDesc = GetComponentRegistry().GetTypeDescFromGUID(compGuid);
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
					DeserializeClass(componentData, 0, componentDesc, entity, streamReader);
					break;
				}
			}

		});

		if (scene->GetRegistry().any_of<VertexPaintedComponent>(entity))
		{
			std::filesystem::path vpPath = metadata.filePath.parent_path();
			vpPath = ProjectManager::GetRootDirectory() / vpPath / "Layers" / ("ent_" + std::to_string((uint32_t)entityId) + ".entVp");

			if (std::filesystem::exists(vpPath))
			{
				auto& vpComp = scene->GetRegistry().get<VertexPaintedComponent>(entity);

				std::ifstream vpFile(vpPath, std::ios::in | std::ios::binary);
				if (!vpFile.is_open())
				{
					VT_LOG(Error, "Could not open entVp file!");
				}

				Vector<uint8_t> totalData;
				const size_t srcSize = vpFile.seekg(0, std::ios::end).tellg();
				totalData.resize_uninitialized(srcSize);
				vpFile.seekg(0, std::ios::beg);
				vpFile.read(reinterpret_cast<char*>(totalData.data()), totalData.size());
				vpFile.close();

				memcpy_s(&vpComp.meshHandle, sizeof(vpComp.meshHandle), totalData.data() + totalData.size() - sizeof(vpComp.meshHandle), sizeof(vpComp.meshHandle));
				totalData.resize_uninitialized(totalData.size() - sizeof(vpComp.meshHandle));

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

	void SceneSerializer::LoadWorldCell(const Ref<Scene>& scene, const WorldCell& worldCell) const
	{
		VT_PROFILE_FUNCTION();

		if (worldCell.isLoaded)
		{
			VT_LOG(Warning, "[SceneImporter]: World Cell is already loaded!");
			return;
		}

		if (worldCell.cellEntities.empty())
		{
			VT_LOG(Warning, "[SceneImporter]: Unable to load World Cell which contains zero entities!");
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

		Vector<std::filesystem::path> entityPaths;

		for (const auto& it : std::filesystem::directory_iterator(layersFolderPath))
		{
			if (!it.is_directory() && it.path().extension().string() == ENTITY_FILE_EXTENSION)
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

		const uint32_t threadCount = Algo::GetThreadCountFromIterationCount(static_cast<uint32_t>(entityPaths.size()));

		Vector<Ref<Scene>> dummyScenes{};
		dummyScenes.resize(threadCount);

		Algo::ForEachParallelLocking([&dummyScenes, entityPaths, metadata, this](uint32_t threadIdx, uint32_t i)
		{
			if (!dummyScenes[threadIdx])
			{
				dummyScenes[threadIdx] = CreateRef<Scene>();;	
			}

			const auto& path = entityPaths.at(i);

			BinaryStreamReader streamReader{ path };
			if (!streamReader.IsStreamValid())
			{
				return;
			}

			uint32_t magicVal = 0;
			streamReader.Read(magicVal);

			if (magicVal != ENTITY_MAGIC_VAL)
			{
				VT_LOG(Error, "[SceneSerializer]: File is not a valid entity!");
				return;
			}

			Buffer buffer{};
			streamReader.Read(buffer);

			YAMLMemoryStreamReader yamlStreamReader{};
			if (!yamlStreamReader.ConsumeBuffer(buffer))
			{
				return;
			}

			DeserializeEntity(dummyScenes[threadIdx], metadata, yamlStreamReader);
		},
		static_cast<uint32_t>(entityPaths.size()));

		for (const auto& dummyScene : dummyScenes)
		{
			if (!dummyScene)
			{
				continue;
			}

			for (const auto& entity : dummyScene->GetAllEntities())
			{
				Entity realEntity = scene->CreateEntityWithID(entity.GetID());
				Entity::Copy(entity, realEntity, Volt::EntityCopyFlags::None);

				scene->InvalidateEntityTransform(realEntity.GetID());
			}
		}
	}

	void SceneSerializer::SerializeWorldEngine(const Ref<Scene>& scene, YAMLMemoryStreamWriter& streamWriter) const
	{
		auto& worldEngine = scene->m_worldEngine;

		streamWriter.BeginMapNamned("WorldEngine");
		streamWriter.SetKey("cellSize", worldEngine.GetSettings().cellSize);
		streamWriter.SetKey("worldSize", worldEngine.GetSettings().worldSize);

		streamWriter.EndMap();
	}

	void SceneSerializer::DeserializeWorldEngine(const Ref<Scene>& scene, YAMLMemoryStreamReader& streamReader) const
	{
		auto& worldEngine = scene->m_worldEngine;

		streamReader.EnterScope("WorldEngine");
		worldEngine.GetSettingsMutable().cellSize = streamReader.ReadAtKey("cellSize", 25'600);
		worldEngine.GetSettingsMutable().worldSize = streamReader.ReadAtKey("worldSize", glm::uvec2{ 128'000 });
		worldEngine.GenerateCells();
		streamReader.ExitScope();
	}

	void SceneSerializer::SerializeEntities(const AssetMetadata& metadata, const Ref<Scene>& scene, const std::filesystem::path& sceneDirectory) const
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
				const auto entityPath = entitiesDirectoryPath / (entity.ToString() + ENTITY_FILE_EXTENSION);

				YAMLMemoryStreamWriter streamWriter{};
				SerializeEntity(entity, metadata, scene, streamWriter);

				auto buffer = streamWriter.WriteAndGetBuffer();

				BinaryStreamWriter binaryStreamWriter{};
				binaryStreamWriter.Write(ENTITY_MAGIC_VAL);
				binaryStreamWriter.Write(buffer);

				buffer.Release();
			
				binaryStreamWriter.WriteToDisk(entityPath, true, 0);
			}, 
			static_cast<uint32_t>(entities.size()));
		}

		const auto removedEntities = scene->GetAllRemovedEntities();
		if (!removedEntities.empty())
		{
			Algo::ForEachParallel([removedEntities, entitiesDirectoryPath](uint32_t threadIdx, uint32_t i)
			{
				EntityID id = removedEntities.at(i);

				const std::filesystem::path entityFilePath = entitiesDirectoryPath / (std::to_string(id) + ENTITY_FILE_EXTENSION);
				if (std::filesystem::exists(entityFilePath))
				{
					FileSystem::MoveToRecycleBin(entityFilePath);
				}

			}, static_cast<uint32_t>(removedEntities.size()));
		}

		VT_LOG(Info, "[SceneImporter]: Saved {0} entities!", entities.size());
		VT_LOG(Info, "[SceneImporter]: Removed {0} entities!", removedEntities.size());

		scene->ClearEditedEntities();
	}

	void SceneSerializer::LoadCellEntities(const AssetMetadata& metadata, const Ref<Scene>& scene, const std::filesystem::path& sceneDirectory) const
	{
		std::filesystem::path layersFolderPath = sceneDirectory / "Entities";
		if (!std::filesystem::exists(layersFolderPath))
		{
			return;
		}

		Vector<std::filesystem::path> entityPaths;

		for (const auto& it : std::filesystem::directory_iterator(layersFolderPath))
		{
			if (!it.is_directory() && it.path().extension().string() == ENTITY_FILE_EXTENSION)
			{
				entityPaths.emplace_back(it.path());
			}
		}

		if (entityPaths.empty())
		{
			return;
		}

		const uint32_t iterationCount = static_cast<uint32_t>(entityPaths.size());

		Vector<std::unordered_map<WorldCellID, Vector<EntityID>>> threadCellEntities{};
		threadCellEntities.resize(Algo::GetThreadCountFromIterationCount(iterationCount));

		Algo::ForEachParallelLocking([&threadCellEntities, entityPaths, metadata, this](uint32_t threadIdx, uint32_t i)
		{
			const auto& path = entityPaths.at(i);

			BinaryStreamReader streamReader{ path };
			if (!streamReader.IsStreamValid())
			{
				return;
			}

			uint32_t magicVal = 0;
			streamReader.Read(magicVal);

			if (magicVal != ENTITY_MAGIC_VAL)
			{
				VT_LOG(Error, "[SceneSerializer]: File is not a valid entity!");
				return;
			}

			Buffer buffer{};
			streamReader.Read(buffer);

			YAMLMemoryStreamReader yamlStreamReader{};
			if (!yamlStreamReader.ConsumeBuffer(buffer))
			{
				return;
			}

			yamlStreamReader.EnterScope("Entity");

			EntityID entityId = yamlStreamReader.ReadAtKey("id", Entity::NullID());
			WorldCellID cellId = yamlStreamReader.ReadAtKey("cellId", INVALID_WORLD_CELL_ID);

			if (entityId == Entity::NullID())
			{
				return;
			}

			if (cellId == INVALID_WORLD_CELL_ID)
			{
				cellId = 0;
			}

			yamlStreamReader.ExitScope();

			threadCellEntities[threadIdx][cellId].push_back(entityId);
		},
		iterationCount);

		auto& worldEngine = scene->m_worldEngine;
		for (const auto& tCellEntities : threadCellEntities)
		{
			for (const auto& [cellId, entities] : tCellEntities)
			{
				worldEngine.AddEntitiesToCell(cellId, entities);
			}
		}
	}

	Entity SceneSerializer::CreateEntityFromUUIDThreadSafe(EntityID entityId, const Ref<Scene>& scene) const
	{
		static std::mutex createEntityMutex;
		std::scoped_lock lock{ createEntityMutex };

		return scene->CreateEntityWithID(entityId);
	}
	
	void SceneSerializer::SerializeClass(const uint8_t* data, const size_t offset, const IComponentTypeDesc* compDesc, YAMLMemoryStreamWriter& streamWriter, bool isSubComponent) const
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

	void SceneSerializer::SerializeArray(const uint8_t* data, const size_t offset, const IArrayTypeDesc* arrayDesc, YAMLMemoryStreamWriter& streamWriter) const
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

	void SceneSerializer::DeserializeClass(uint8_t* data, const size_t offset, const IComponentTypeDesc* compDesc, Entity dstEntity, YAMLMemoryStreamReader& streamReader) const
	{
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
						DeserializeClass(data, offset + componentMember->offset, memberCompType, dstEntity, streamReader);
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
						DeserializeArray(data, offset + componentMember->offset, arrayTypeDesc, dstEntity, streamReader);
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

		compDesc->OnComponentDeserialized(dstEntity.GetScene()->GetEntityHelperFromEntityID(dstEntity.GetID()));
	}

	void SceneSerializer::DeserializeArray(uint8_t* data, const size_t offset, const IArrayTypeDesc* arrayDesc, Entity dstEntity, YAMLMemoryStreamReader& streamReader) const
	{
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
						DeserializeClass(tempBytePtr, 0, compType, dstEntity, streamReader);
						break;
					}

					case ValueType::Enum:
						*reinterpret_cast<int32_t*>(&tempDataStorage) = streamReader.ReadAtKey("enumValue", int32_t(0));
						break;

					case ValueType::Array:
					{
						const IArrayTypeDesc* arrayTypeDesc = reinterpret_cast<const IArrayTypeDesc*>(arrayDesc->GetElementTypeDesc());
						DeserializeArray(tempBytePtr, 0, arrayTypeDesc, dstEntity, streamReader);
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
}
