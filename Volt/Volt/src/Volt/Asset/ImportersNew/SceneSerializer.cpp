#include "vtpch.h"
#include "SceneSerializer.h"

#include "Volt/Asset/AssetManager.h"

#include "Volt/Components/CoreComponents.h"
#include "Volt/Components/RenderingComponents.h"

#include "Volt/Scene/Scene.h"
#include "Volt/Scene/Entity.h"
#include "Volt/Scene/Reflection/ComponentReflection.h"
#include "Volt/Scene/Reflection/ComponentRegistry.h"

#include "Volt/Scripting/Mono/MonoScriptClass.h"
#include "Volt/Scripting/Mono/MonoScriptEngine.h"

#include "Volt/Core/BinarySerializer.h"

#include "Volt/Utility/Algorithms.h"
#include "Volt/Utility/FileSystem.h"

#include "Volt/Utility/FileIO/YAMLMemoryStreamWriter.h"
#include "Volt/Utility/FileIO/YAMLStreamReader.h"

namespace Volt
{
	inline static constexpr uint32_t CURRENT_ASSET_VERSION = 1;
	inline static constexpr uint32_t ENTITY_MAGIC_VAL = 1515;

	template<typename T>
	void RegisterSerializationFunction(std::unordered_map<std::type_index, std::function<void(YAMLMemoryStreamWriter&, const uint8_t*, const size_t)>>& outTypes)
	{
		VT_PROFILE_FUNCTION();

		outTypes[std::type_index{ typeid(T) }] = [](YAMLMemoryStreamWriter& streamWriter, const uint8_t* data, const size_t offset)
		{
			const T& var = *reinterpret_cast<const T*>(&data[offset]);
			streamWriter.SetKey("data", var);
		};
	}

	template<typename T>
	void RegisterDeserializationFunction(std::unordered_map<std::type_index, std::function<void(YAMLStreamReader&, uint8_t*, const size_t)>>& outTypes)
	{
		VT_PROFILE_FUNCTION();

		outTypes[std::type_index{ typeid(T) }] = [](YAMLStreamReader& streamReader, uint8_t* data, const size_t offset)
		{
			*reinterpret_cast<T*>(&data[offset]) = streamReader.ReadKey("data", T());
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
			const size_t compressedDataOffset = AssetSerializer::WriteMetadata(metadata, CURRENT_ASSET_VERSION, sceneFileWriter);

			auto buffer = streamWriter.WriteAndGetBuffer();
			sceneFileWriter.Write(buffer);
			buffer.Release();

			sceneFileWriter.WriteToDisk(scenePath, true, compressedDataOffset); // #TODO_Ivar: Fix compressed offset
		}
	}

	bool SceneSerializer::Deserialize(const AssetMetadata& metadata, Ref<Asset> destinationAsset) const
	{
		return false;
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
			std::filesystem::path vpPath = (ProjectManager::GetDirectory() / metadata.filePath.parent_path() / "Layers" / ("ent_" + std::to_string((uint32_t)id) + ".entVp"));
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

	void SceneSerializer::SerializeWorldEngine(const Ref<Scene>& scene, YAMLMemoryStreamWriter& streamWriter) const
	{
		auto& worldEngine = scene->m_worldEngine;

		streamWriter.BeginMapNamned("WorldEngine");
		streamWriter.SetKey("cellSize", worldEngine.GetSettings().cellSize);
		streamWriter.SetKey("worldSize", worldEngine.GetSettings().worldSize);

		streamWriter.EndMap();
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
				const auto entityPath = entitiesDirectoryPath / (entity.ToString() + ".vtasset");

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

	void SceneSerializer::SerializeMono(entt::entity id, const Ref<Scene>& scene, YAMLMemoryStreamWriter& streamWriter) const
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
}
