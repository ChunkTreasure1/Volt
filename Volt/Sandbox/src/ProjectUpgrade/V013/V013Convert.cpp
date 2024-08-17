#include "sbpch.h"
#include "V013Convert.h"
#include <Volt/Project/ProjectManager.h>

#include <Volt/Scene/Scene.h>
#include <Volt/Scene/Entity.h>
#include <Volt/Scene/Reflection/ComponentRegistry.h>
#include <Volt/Components/CoreComponents.h>
#include <Volt/Asset/Prefab.h>
#include <AssetSystem/AssetManager.h>
#include <Volt/Asset/Importers/SceneImporter.h>
#include <Volt/Asset/Importers/PrefabImporter.h>
#include <Volt/Scripting/Mono/MonoScriptEngine.h>

#include <Volt/Components/AudioComponents.h>
#include <Volt/Components/RenderingComponents.h>
#include <Volt/Vision/VisionComponents.h>
#include <Volt/Net/SceneInteraction/NetActorComponent.h>
#include <Volt/Net/SceneInteraction/GameModeComponent.h>
#include <Volt/Utility/YAMLSerializationHelpers.h>

#include <CoreUtilities/FileIO/YAMLFileStreamReader.h>

namespace V013
{
	enum class PreV013PropertyType : uint32_t
	{
		Bool = 0,
		Int = 1,
		UInt = 2,
		Short = 3,
		UShort = 4,
		Char = 5,
		UChar = 6,
		Float = 7,
		Double = 8,
		Vector2 = 9,
		Vector3 = 10,
		Vector4 = 11,
		String = 12,
		Unknown = 13,

		Int64 = 14,
		UInt64 = 15,
		AssetHandle = 16,
		Color3 = 17,
		Color4 = 18,
		Directory = 19,
		Path = 20,
		Vector = 21,
		EntityId = 22,
		GUID = 23,
		Enum = 24,
		Quaternion = 25
	};

	struct TypeIndexContainer
	{
		std::type_index typeIndex = typeid(void);
	};

	static std::unordered_map<PreV013PropertyType, TypeIndexContainer> s_preV113PropTypeToTypeIndexMap;
	static std::unordered_map<std::type_index, std::function<void(YAMLFileStreamReader&, uint8_t*, const size_t)>> s_arrayDeserializers;
	static std::unordered_map<VoltGUID, std::unordered_map<std::string, std::string>> s_componentMemberRemap;

	inline const bool IsPreV013EntityNull(Volt::EntityID entityId)
	{
		return entityId == Volt::Entity::NullID();
	}

	inline void ValidateEntityValidity(Volt::EntityID* entityId)
	{
		if (*entityId == (Volt::EntityID)0)
		{
			*entityId = Volt::Entity::NullID();
		}
	}

	inline const Volt::ComponentMember* TryGetComponentMemberFromName(const std::string& memberName, const Volt::IComponentTypeDesc* componentDesc)
	{
		std::string demangledName = Utility::ToLower(memberName);
		demangledName.erase(std::remove_if(demangledName.begin(), demangledName.end(), ::isspace));

		for (uint32_t idx = 0; const auto & member : componentDesc->GetMembers())
		{
			const std::string memberNameLower = Utility::ToLower(std::string(member.name));

			if (demangledName.find(memberNameLower) != std::string::npos)
			{
				return &componentDesc->GetMembers().at(idx);
			}

			idx++;
		}

		if (s_componentMemberRemap.contains(componentDesc->GetGUID()))
		{
			for (const auto& [oldMemberName, newMemberName] : s_componentMemberRemap.at(componentDesc->GetGUID()))
			{
				if (memberName == oldMemberName)
				{
					return componentDesc->FindMemberByName(newMemberName);
				}
			}
		}

		return nullptr;
	}

	inline void DeserializePreV113Component(uint8_t* componentData, const Volt::IComponentTypeDesc* componentDesc, YAMLFileStreamReader& streamReader)
	{
		const auto& typeDeserializers = Volt::SceneImporter::GetTypeDeserializers();

		streamReader.ForEach("properties", [&]()
		{
			const std::string memberName = streamReader.ReadAtKey("name", std::string(""));

			if (memberName.empty())
			{
				return;
			}

			const Volt::ComponentMember* componentMember = TryGetComponentMemberFromName(memberName, componentDesc);
			if (!componentMember)
			{
				return;
			}

			const PreV013PropertyType oldType = static_cast<PreV013PropertyType>(streamReader.ReadAtKey("type", 0u));

			if (oldType == PreV013PropertyType::Vector && componentMember->typeDesc != nullptr)
			{
				const Volt::IArrayTypeDesc* arrayTypeDesc = reinterpret_cast<const Volt::IArrayTypeDesc*>(componentMember->typeDesc);
				const PreV013PropertyType vectorType = static_cast<PreV013PropertyType>(streamReader.ReadAtKey("vectorType", 0u));
				const std::type_index vectorValueType = s_preV113PropTypeToTypeIndexMap.at(vectorType).typeIndex;
				void* arrayPtr = &componentData[componentMember->offset];

				if (arrayTypeDesc->GetElementTypeIndex() != vectorValueType)
				{
					VT_LOG(Warning, "[Upgrade Project]: Component member vector type does not match file specified type!");
					return;
				}

				streamReader.ForEach("data", [&]()
				{
					void* tempDataStorage = nullptr;
					arrayTypeDesc->DefaultConstructElement(tempDataStorage);

					uint8_t* tempBytePtr = reinterpret_cast<uint8_t*>(tempDataStorage);

					if (s_arrayDeserializers.contains(vectorValueType))
					{
						s_arrayDeserializers.at(vectorValueType)(streamReader, tempBytePtr, 0);
					}

					if (arrayTypeDesc->GetElementTypeIndex() == std::type_index{ typeid(Volt::EntityID) })
					{
						ValidateEntityValidity(reinterpret_cast<Volt::EntityID*>(tempDataStorage));
					}

					arrayTypeDesc->PushBack(arrayPtr, tempDataStorage);
					arrayTypeDesc->DestroyElement(tempDataStorage);
				});

				return;
			}
			else if (oldType == PreV013PropertyType::Enum)
			{
				typeDeserializers.at(typeid(int32_t))(streamReader, componentData, componentMember->offset);
				return;
			}

			const std::type_index memberType = s_preV113PropTypeToTypeIndexMap.at(oldType).typeIndex;
			if (memberType != componentMember->typeIndex)
			{
				VT_LOG(Warning, "[Upgrade Project]: Component member type does not match file specified type!");
				return;
			}

			if (typeDeserializers.contains(memberType))
			{
				typeDeserializers.at(memberType)(streamReader, componentData, componentMember->offset);
				if (memberType == std::type_index{ typeid(Volt::EntityID) })
				{
					Volt::EntityID* entDataPtr = reinterpret_cast<Volt::EntityID*>(&componentData[componentMember->offset]);
					ValidateEntityValidity(entDataPtr);
				}
			}
		});
	}

	void DeserializePreV113MonoScripts(Ref<Volt::Scene> scene, const Volt::EntityID entityId, YAMLFileStreamReader& streamReader)
	{
		Volt::Entity entity = scene->GetEntityFromUUID(entityId);
		const auto& typeDeserializers = Volt::SceneImporter::GetTypeDeserializers();

		streamReader.ForEach("MonoScripts", [&]()
		{
			streamReader.EnterScope("ScriptEntry");

			if (!entity.HasComponent<Volt::MonoScriptComponent>())
			{
				entity.AddComponent<Volt::MonoScriptComponent>();
			}

			const std::string scriptName = streamReader.ReadAtKey("name", std::string(""));
			UUID64 scriptId = streamReader.ReadAtKey("id", UUID64(0));

			auto scriptClass = Volt::MonoScriptEngine::GetScriptClass(scriptName);
			if (!scriptClass)
			{
				streamReader.ExitScope();
				return;
			}

			auto& scriptComp = entity.GetComponent<Volt::MonoScriptComponent>();

			if (auto it = std::find(scriptComp.scriptNames.begin(), scriptComp.scriptNames.end(), scriptName); it != scriptComp.scriptNames.end())
			{
				const size_t index = std::distance(scriptComp.scriptNames.begin(), it);
				scriptId = scriptComp.scriptIds.at(index);
			}
			else
			{
				scriptComp.scriptIds.emplace_back(scriptId);
				scriptComp.scriptNames.emplace_back(scriptName);
			}

			const auto& classFields = scriptClass->GetFields();
			auto& fieldCache = scene->GetScriptFieldCache().GetCache()[scriptId];

			streamReader.ForEach("properties", [&]()
			{
				const std::string memberName = streamReader.ReadAtKey("name", std::string(""));
				if (!classFields.contains(memberName))
				{
					return;
				}

				Ref<Volt::MonoScriptFieldInstance> fieldInstance = CreateRef<Volt::MonoScriptFieldInstance>();
				fieldCache[memberName] = fieldInstance;

				fieldInstance->field = classFields.at(memberName);
				if (fieldInstance->field.type.IsString())
				{
					const std::string str = streamReader.ReadAtKey("data", std::string(""));
					fieldInstance->SetValue(str, str.size());
				}
				else if (typeDeserializers.contains(fieldInstance->field.type.typeIndex))
				{
					fieldInstance->data.Allocate(fieldInstance->field.type.typeSize);
					typeDeserializers.at(fieldInstance->field.type.typeIndex)(streamReader, fieldInstance->data.As<uint8_t>(), 0);
				}
			});

			streamReader.ExitScope();
		});
	}

	inline void DeserializePreV013Entity(Ref<Volt::Scene> scene, YAMLFileStreamReader& streamReader, std::map<Volt::EntityID, Volt::EntityID>& entityRemapping, bool isPrefabEntity)
	{
		if (!isPrefabEntity)
		{
			streamReader.EnterScope("Entity");
		}

		Volt::EntityID originalEntityId = streamReader.ReadAtKey("id", Volt::Entity::NullID());

		if (IsPreV013EntityNull(originalEntityId))
		{
			return;
		}

		Volt::Entity newEntity = scene->CreateEntityWithUUID(originalEntityId);
		const Volt::EntityID entityId = newEntity.GetID();

		const auto handle = scene->GetHandleFromUUID(entityId);

		if (entityId != originalEntityId)
		{
			entityRemapping[originalEntityId] = entityId;
		}

		bool skipTransformComp = false;
		bool skipPrefabComp = false;
		bool skipRelComp = false;
		bool skipCommonComp = false;

		streamReader.ForEach("components", [&]()
		{
			const VoltGUID compGuid = streamReader.ReadAtKey("guid", VoltGUID::Null());
			if (compGuid == VoltGUID::Null())
			{
				return;
			}

			const Volt::ICommonTypeDesc* typeDesc = Volt::ComponentRegistry::GetTypeDescFromGUID(compGuid);
			if (!typeDesc)
			{
				return;
			}

			switch (typeDesc->GetValueType())
			{
				case Volt::ValueType::Component:
				{
					if (!Volt::ComponentRegistry::Helpers::HasComponentWithGUID(compGuid, scene->GetRegistry(), handle))
					{
						Volt::ComponentRegistry::Helpers::AddComponentWithGUID(compGuid, scene->GetRegistry(), handle);
					}
					void* voidCompPtr = Volt::ComponentRegistry::Helpers::GetComponentWithGUID(compGuid, scene->GetRegistry(), handle);
					uint8_t* componentData = reinterpret_cast<uint8_t*>(voidCompPtr);

					const Volt::IComponentTypeDesc* componentDesc = reinterpret_cast<const Volt::IComponentTypeDesc*>(typeDesc);

					if (componentDesc->GetGUID() == Volt::GetTypeGUID<Volt::TransformComponent>())
					{
						if (!streamReader.IsSequenceEmpty("properties"))
						{
							skipTransformComp = true;
						}
					}
					else if (componentDesc->GetGUID() == Volt::GetTypeGUID<Volt::PrefabComponent>())
					{
						if (!streamReader.IsSequenceEmpty("properties"))
						{
							skipPrefabComp = true;
						}
					}
					else if (componentDesc->GetGUID() == Volt::GetTypeGUID<Volt::RelationshipComponent>())
					{
						if (!streamReader.IsSequenceEmpty("properties"))
						{
							skipRelComp = true;
						}
					}
					else if (componentDesc->GetGUID() == Volt::GetTypeGUID<Volt::CommonComponent>())
					{
						if (!streamReader.IsSequenceEmpty("properties"))
						{
							skipCommonComp = true;
						}
					}

					DeserializePreV113Component(componentData, componentDesc, streamReader);
					break;
				}
			}
		});

		if (scene->GetRegistry().any_of<Volt::PrefabComponent>(handle) && !isPrefabEntity)
		{
			auto& prefabComp = scene->GetRegistry().get<Volt::PrefabComponent>(handle);
			Ref<Volt::Prefab> prefabAsset = Volt::AssetManager::GetAsset<Volt::Prefab>(prefabComp.prefabAsset);

			if (prefabAsset && prefabAsset->IsValid())
			{
				Volt::EntityCopyFlags copyFlags = Volt::EntityCopyFlags::None;
				if (skipTransformComp)
				{
					copyFlags = copyFlags | Volt::EntityCopyFlags::SkipTransform;
				}
				if (skipPrefabComp)
				{
					copyFlags = copyFlags | Volt::EntityCopyFlags::SkipPrefab;
				}
				if (skipRelComp)
				{
					copyFlags = copyFlags | Volt::EntityCopyFlags::SkipRelationships;
				}
				if (skipCommonComp)
				{
					copyFlags = copyFlags | Volt::EntityCopyFlags::SkipCommonData;
				}

				prefabAsset->CopyPrefabEntity(newEntity, prefabComp.prefabEntity, copyFlags);
			}
		}

		if (scene->GetRegistry().any_of<Volt::MonoScriptComponent>(handle))
		{
			DeserializePreV113MonoScripts(scene, entityId, streamReader);
		}

		// Make sure entity has relationship component
		if (!scene->GetRegistry().any_of<Volt::RelationshipComponent>(handle))
		{
			scene->GetRegistry().emplace<Volt::RelationshipComponent>(handle);
		}

		if (!isPrefabEntity)
		{
			streamReader.ExitScope();
		}
	}

	inline void ValidateSceneConversionArray(Ref<Volt::Scene> scene, const Volt::ComponentMember& componentMember, uint8_t* componentData)
	{
		const Volt::IArrayTypeDesc* memberArrayTypeDesc = reinterpret_cast<const Volt::IArrayTypeDesc*>(componentMember.typeDesc);
		void* arrayPtr = reinterpret_cast<void*>(&componentData[componentMember.offset]);

		Vector<size_t> indicesToRemove{};

		for (size_t i = 0; i < memberArrayTypeDesc->Size(arrayPtr); i++)
		{
			Volt::EntityID* value = reinterpret_cast<Volt::EntityID*>(memberArrayTypeDesc->At(arrayPtr, i));
			if (!scene->GetRegistry().valid(scene->GetHandleFromUUID(*value)))
			{
				indicesToRemove.emplace_back(i);
				*value = Volt::Entity::NullID();
			}
		}

		for (const auto& i : indicesToRemove)
		{
			memberArrayTypeDesc->Erase(arrayPtr, i);
		}
	}

	inline void ValidateSceneConversion(Ref<Volt::Scene> scene)
	{
		for (auto&& curr : scene->GetRegistry().storage())
		{
			auto& storage = curr.second;
			std::string_view typeName = storage.type().name();

			const Volt::ICommonTypeDesc* typeDesc = Volt::ComponentRegistry::GetTypeDescFromName(typeName);
			if (!typeDesc)
			{
				continue;
			}

			if (typeDesc->GetGUID() == Volt::GetTypeGUID<Volt::PrefabComponent>())
			{
				continue;
			}

			if (typeDesc->GetValueType() != Volt::ValueType::Component)
			{
				continue;
			}

			const Volt::IComponentTypeDesc* componentTypeDesc = reinterpret_cast<const Volt::IComponentTypeDesc*>(typeDesc);

			for (const auto& member : componentTypeDesc->GetMembers())
			{
				if (member.typeDesc)
				{
					if (member.typeDesc->GetValueType() != Volt::ValueType::Array)
					{
						continue;
					}

					const Volt::IArrayTypeDesc* memberArrayTypeDesc = reinterpret_cast<const Volt::IArrayTypeDesc*>(member.typeDesc);
					if (memberArrayTypeDesc->GetElementTypeIndex() != std::type_index{ typeid(Volt::EntityID) })
					{
						continue;
					}

					for (const auto& id : storage)
					{
						uint8_t* componentDataPtr = reinterpret_cast<uint8_t*>(storage.get(id));
						ValidateSceneConversionArray(scene, member, componentDataPtr);
					}
				}
				else if (member.typeIndex == std::type_index{ typeid(Volt::EntityID) })
				{
					for (const auto& id : storage)
					{
						uint8_t* componentDataPtr = reinterpret_cast<uint8_t*>(storage.get(id));
						Volt::EntityID& value = *reinterpret_cast<Volt::EntityID*>(&componentDataPtr[member.offset]);

						if (!scene->GetRegistry().valid(scene->GetHandleFromUUID(value)))
						{
							value = Volt::Entity::NullID();
						}
					}
				}
			}
		}
	}

	inline void HandleEntityArrayRemapping(Ref<Volt::Scene> scene, const std::map<Volt::EntityID, Volt::EntityID>& entityRemapping, const Volt::ComponentMember& componentMember, uint8_t* componentData)
	{
		const Volt::IArrayTypeDesc* memberArrayTypeDesc = reinterpret_cast<const Volt::IArrayTypeDesc*>(componentMember.typeDesc);
		void* arrayPtr = reinterpret_cast<void*>(&componentData[componentMember.offset]);

		for (size_t i = 0; i < memberArrayTypeDesc->Size(arrayPtr); i++)
		{
			Volt::EntityID* value = reinterpret_cast<Volt::EntityID*>(memberArrayTypeDesc->At(arrayPtr, i));
			for (const auto& [originalEntityId, newEntityId] : entityRemapping)
			{
				if (*value == originalEntityId)
				{
					*value = newEntityId;
				}
			}
		}
	}

	inline void HandleEntityRemapping(Ref<Volt::Scene> scene, const std::map<Volt::EntityID, Volt::EntityID>& entityRemapping)
	{
		for (auto&& curr : scene->GetRegistry().storage())
		{
			auto& storage = curr.second;
			std::string_view typeName = storage.type().name();

			const Volt::ICommonTypeDesc* typeDesc = Volt::ComponentRegistry::GetTypeDescFromName(typeName);
			if (!typeDesc)
			{
				continue;
			}

			if (typeDesc->GetValueType() != Volt::ValueType::Component)
			{
				continue;
			}

			const Volt::IComponentTypeDesc* componentTypeDesc = reinterpret_cast<const Volt::IComponentTypeDesc*>(typeDesc);

			for (const auto& member : componentTypeDesc->GetMembers())
			{
				if (member.typeDesc)
				{
					if (member.typeDesc->GetValueType() != Volt::ValueType::Array)
					{
						continue;
					}

					const Volt::IArrayTypeDesc* memberArrayTypeDesc = reinterpret_cast<const Volt::IArrayTypeDesc*>(member.typeDesc);
					if (memberArrayTypeDesc->GetElementTypeIndex() != std::type_index{ typeid(Volt::EntityID) })
					{
						continue;
					}

					for (const auto& id : storage)
					{
						uint8_t* componentDataPtr = reinterpret_cast<uint8_t*>(storage.get(id));
						HandleEntityArrayRemapping(scene, entityRemapping, member, componentDataPtr);
					}
				}
				else if (member.typeIndex == std::type_index{ typeid(Volt::EntityID) })
				{
					for (const auto& id : storage)
					{
						uint8_t* componentDataPtr = reinterpret_cast<uint8_t*>(storage.get(id));

						for (const auto& [originalEntityId, newEntityId] : entityRemapping)
						{
							Volt::EntityID& value = *reinterpret_cast<Volt::EntityID*>(&componentDataPtr[member.offset]);
							if (value == originalEntityId)
							{
								value = newEntityId;
							}
						}
					}
				}
			}
		}
	}

	inline void ConvertPreV013Prefab(const std::filesystem::path& filePath)
	{
		YAMLFileStreamReader streamReader{};
		if (!streamReader.OpenFile(filePath))
		{
			return;
		}

		uint32_t version = 0;
		Volt::EntityID rootEntityId = Volt::Entity::NullID();
		Ref<Volt::Scene> prefabScene = CreateRef<Volt::Scene>();

		std::map<Volt::EntityID, Volt::EntityID> entityRemapping;

		streamReader.EnterScope("Prefab");
		{
			version = streamReader.ReadAtKey("version", 0u);

			streamReader.ForEach("entities", [&]()
			{
				Volt::EntityID entityId = streamReader.ReadAtKey("id", Volt::Entity::NullID());
				if (IsPreV013EntityNull(entityId))
				{
					return;
				}

				DeserializePreV013Entity(prefabScene, streamReader, entityRemapping, true);
			});
		}
		streamReader.ExitScope();

		HandleEntityRemapping(prefabScene, entityRemapping);
		ValidateSceneConversion(prefabScene);

		// In old prefabs, the root is the only entity with no parent
		prefabScene->ForEachWithComponents<const Volt::RelationshipComponent, const Volt::IDComponent>([&](const entt::entity id, const Volt::RelationshipComponent& comp, const Volt::IDComponent& idComponent)
		{
			if (comp.parent == Volt::Entity::NullID())
			{
				rootEntityId = idComponent.id;
			}
		});

		Ref<Volt::Prefab> newPrefab = CreateRef<Volt::Prefab>(prefabScene, rootEntityId, version);

		Volt::AssetMetadata imposterMeta{};
		imposterMeta.filePath = filePath;

		Volt::PrefabImporter::Get().Save(imposterMeta, newPrefab);
	}

	inline void ConvertPrefabsToV013()
	{
		auto& project = Volt::ProjectManager::GetProject();
		const std::filesystem::path assetsPath = project.rootDirectory / project.assetsDirectory;

		Vector<std::filesystem::path> prefabFilePaths;
		for (const auto& entry : std::filesystem::recursive_directory_iterator(assetsPath))
		{
			if (entry.path().extension().string() == ".vtprefab")
			{
				prefabFilePaths.emplace_back(entry.path());
			}
		}

		for (const auto& prefabFilePath : prefabFilePaths)
		{
			ConvertPreV013Prefab(prefabFilePath);
		}
	}

	inline void DeserializePreV113SceneLayer(Ref<Volt::Scene> scene, Volt::SceneLayer& sceneLayer, const std::filesystem::path& layerPath, std::map<Volt::EntityID, Volt::EntityID>& entityRemapping)
	{
		YAMLFileStreamReader streamReader{};

		if (!streamReader.OpenFile(layerPath))
		{
			return;
		}

		streamReader.EnterScope("Layer");
		{
			sceneLayer.name = streamReader.ReadAtKey("name", std::string("Null"));
			sceneLayer.id = streamReader.ReadAtKey("id", 0u);
			sceneLayer.visible = streamReader.ReadAtKey("visible", true);
			sceneLayer.locked = streamReader.ReadAtKey("locked", false);

			streamReader.ForEach("Entities", [&]()
			{
				DeserializePreV013Entity(scene, streamReader, entityRemapping, false);
			});
		}
		streamReader.ExitScope();
	}

	inline void ConvertScenesToV013()
	{
		// We only need to convert the layer files, and not the scene files

		// Find all scene directories
		auto& project = Volt::ProjectManager::GetProject();
		const std::filesystem::path assetsPath = project.rootDirectory / project.assetsDirectory;

		Vector<std::filesystem::path> sceneFilePaths;
		for (const auto& entry : std::filesystem::recursive_directory_iterator(assetsPath))
		{
			if (entry.path().extension().string() == ".vtscene")
			{
				sceneFilePaths.emplace_back(entry.path());
			}
		}

		// Convert all found scenes
		for (const auto& sceneFilePath : sceneFilePaths)
		{
			const auto sceneDir = sceneFilePath.parent_path();

			if (!std::filesystem::exists(sceneDir / "Layers"))
			{
				continue;
			}

			std::string sceneName;

			// Load scene name
			{
				YAMLFileStreamReader streamReader{};
				if (!streamReader.OpenFile(sceneFilePath))
				{
					VT_LOG(Error, "[Project Upgrade]: Unable to open scene file! Skipping!");
					continue;
				}

				streamReader.EnterScope("Scene");
				sceneName = streamReader.ReadAtKey("name", std::string("New Scene"));
				streamReader.ExitScope();
			}

			Ref<Volt::Scene> scene = CreateRef<Volt::Scene>(sceneName);
			Vector<Volt::SceneLayer> sceneLayers;

			std::map<Volt::EntityID, Volt::EntityID> entityRemapping;

			for (const auto& entry : std::filesystem::directory_iterator(sceneDir / "Layers"))
			{
				if (entry.path().extension().string() != ".vtlayer")
				{
					continue;
				}

				DeserializePreV113SceneLayer(scene, sceneLayers.emplace_back(), entry.path(), entityRemapping);
			}

			scene->SetLayers(sceneLayers);

			HandleEntityRemapping(scene, entityRemapping);
			//ValidateSceneConversion(scene);

			Volt::AssetMetadata imposterMeta;
			imposterMeta.filePath = Volt::AssetManager::GetRelativePath(sceneFilePath);
			Volt::SceneImporter::Get().Save(imposterMeta, scene);
		}
	}

	template<typename T>
	inline void RegisterArrayDeserializationFunction()
	{
		s_arrayDeserializers[std::type_index{ typeid(T) }] = [](YAMLFileStreamReader& streamReader, uint8_t* data, const size_t offset)
		{
			*reinterpret_cast<T*>(&data[offset]) = streamReader.ReadAtKey("value", T());
		};
	}

	template<typename T>
	inline void AddComponentMemberRemap(const std::string& oldName, const std::string& newName)
	{
		s_componentMemberRemap[Volt::GetTypeGUID<T>()][oldName] = newName;
	}

	inline void Initialize()
	{
		s_preV113PropTypeToTypeIndexMap[PreV013PropertyType::Bool].typeIndex = std::type_index{ typeid(bool) };
		s_preV113PropTypeToTypeIndexMap[PreV013PropertyType::Int].typeIndex = std::type_index{ typeid(int32_t) };
		s_preV113PropTypeToTypeIndexMap[PreV013PropertyType::UInt].typeIndex = std::type_index{ typeid(uint32_t) };
		s_preV113PropTypeToTypeIndexMap[PreV013PropertyType::Short].typeIndex = std::type_index{ typeid(int16_t) };
		s_preV113PropTypeToTypeIndexMap[PreV013PropertyType::UShort].typeIndex = std::type_index{ typeid(uint16_t) };
		s_preV113PropTypeToTypeIndexMap[PreV013PropertyType::Char].typeIndex = std::type_index{ typeid(int8_t) };
		s_preV113PropTypeToTypeIndexMap[PreV013PropertyType::UChar].typeIndex = std::type_index{ typeid(uint8_t) };
		s_preV113PropTypeToTypeIndexMap[PreV013PropertyType::Float].typeIndex = std::type_index{ typeid(float) };
		s_preV113PropTypeToTypeIndexMap[PreV013PropertyType::Double].typeIndex = std::type_index{ typeid(double) };
		s_preV113PropTypeToTypeIndexMap[PreV013PropertyType::Vector2].typeIndex = std::type_index{ typeid(glm::vec2) };
		s_preV113PropTypeToTypeIndexMap[PreV013PropertyType::Vector3].typeIndex = std::type_index{ typeid(glm::vec3) };
		s_preV113PropTypeToTypeIndexMap[PreV013PropertyType::Vector4].typeIndex = std::type_index{ typeid(glm::vec4) };
		s_preV113PropTypeToTypeIndexMap[PreV013PropertyType::String].typeIndex = std::type_index{ typeid(std::string) };
		s_preV113PropTypeToTypeIndexMap[PreV013PropertyType::Int64].typeIndex = std::type_index{ typeid(int64_t) };
		s_preV113PropTypeToTypeIndexMap[PreV013PropertyType::UInt64].typeIndex = std::type_index{ typeid(uint64_t) };
		s_preV113PropTypeToTypeIndexMap[PreV013PropertyType::AssetHandle].typeIndex = std::type_index{ typeid(Volt::AssetHandle) };
		s_preV113PropTypeToTypeIndexMap[PreV013PropertyType::Color3].typeIndex = std::type_index{ typeid(glm::vec3) };
		s_preV113PropTypeToTypeIndexMap[PreV013PropertyType::Color4].typeIndex = std::type_index{ typeid(glm::vec4) };
		s_preV113PropTypeToTypeIndexMap[PreV013PropertyType::Directory].typeIndex = std::type_index{ typeid(std::filesystem::path) };
		s_preV113PropTypeToTypeIndexMap[PreV013PropertyType::Path].typeIndex = std::type_index{ typeid(std::filesystem::path) };
		s_preV113PropTypeToTypeIndexMap[PreV013PropertyType::EntityId].typeIndex = std::type_index{ typeid(Volt::EntityID) };
		s_preV113PropTypeToTypeIndexMap[PreV013PropertyType::GUID].typeIndex = std::type_index{ typeid(VoltGUID) };
		s_preV113PropTypeToTypeIndexMap[PreV013PropertyType::Quaternion].typeIndex = std::type_index{ typeid(glm::quat) };

		RegisterArrayDeserializationFunction<int8_t>();
		RegisterArrayDeserializationFunction<uint8_t>();
		RegisterArrayDeserializationFunction<int16_t>();
		RegisterArrayDeserializationFunction<uint16_t>();
		RegisterArrayDeserializationFunction<int32_t>();
		RegisterArrayDeserializationFunction<uint32_t>();

		RegisterArrayDeserializationFunction<float>();
		RegisterArrayDeserializationFunction<double>();
		RegisterArrayDeserializationFunction<bool>();

		RegisterArrayDeserializationFunction<glm::vec2>();
		RegisterArrayDeserializationFunction<glm::vec3>();
		RegisterArrayDeserializationFunction<glm::vec4>();

		RegisterArrayDeserializationFunction<glm::uvec2>();
		RegisterArrayDeserializationFunction<glm::uvec3>();
		RegisterArrayDeserializationFunction<glm::uvec4>();

		RegisterArrayDeserializationFunction<glm::ivec2>();
		RegisterArrayDeserializationFunction<glm::ivec3>();
		RegisterArrayDeserializationFunction<glm::ivec4>();

		RegisterArrayDeserializationFunction<glm::quat>();
		RegisterArrayDeserializationFunction<glm::mat4>();
		RegisterArrayDeserializationFunction<VoltGUID>();

		RegisterArrayDeserializationFunction<std::string>();
		RegisterArrayDeserializationFunction<std::filesystem::path>();

		RegisterArrayDeserializationFunction<Volt::EntityID>();
		RegisterArrayDeserializationFunction<Volt::AssetHandle>();

		// Member remapping
		{
			AddComponentMemberRemap<Volt::MeshComponent>("Mesh", "handle");
			AddComponentMemberRemap<Volt::AnimatedCharacterComponent>("Character", "animatedCharacter");
			AddComponentMemberRemap<Volt::SpriteComponent>("Material", "materialHandle");
			AddComponentMemberRemap<Volt::AnimationControllerComponent>("Override Skin", "skin");
			AddComponentMemberRemap<Volt::DecalComponent>("Material", "decalMaterial");

			// Vision components
			{
				AddComponentMemberRemap<Volt::VisionTriggerComponent>("Camera", "triggerCam");
				AddComponentMemberRemap<Volt::VisionTriggerComponent>("Force Camera", "forceActiveCam");

				AddComponentMemberRemap<Volt::VisionCameraComponent>("Blend Time", "blendTime");
				AddComponentMemberRemap<Volt::VisionCameraComponent>("Field Of View", "fieldOfView");
				AddComponentMemberRemap<Volt::VisionCameraComponent>("Ignored Layers", "layerMasks");
				AddComponentMemberRemap<Volt::VisionCameraComponent>("Camera Type", "cameraType");
				AddComponentMemberRemap<Volt::VisionCameraComponent>("Blend Type", "blendType");
				AddComponentMemberRemap<Volt::VisionCameraComponent>("Damping", "damping");
				AddComponentMemberRemap<Volt::VisionCameraComponent>("Offset", "offset");
				AddComponentMemberRemap<Volt::VisionCameraComponent>("Follow", "followId");
				AddComponentMemberRemap<Volt::VisionCameraComponent>("LookAt", "lookAtId");
				AddComponentMemberRemap<Volt::VisionCameraComponent>("Collision Focus Point", "collisionRayPoint");
				AddComponentMemberRemap<Volt::VisionCameraComponent>("Focal Distance", "focalDistance");
				AddComponentMemberRemap<Volt::VisionCameraComponent>("Mouse Sensitivity", "mouseSensitivity");
				AddComponentMemberRemap<Volt::VisionCameraComponent>("Collision Sphere Radius", "collisionRadius");
				AddComponentMemberRemap<Volt::VisionCameraComponent>("Collision", "isColliding");
				AddComponentMemberRemap<Volt::VisionCameraComponent>("Is Default", "isDefault");
				AddComponentMemberRemap<Volt::VisionCameraComponent>("X FollowLock", "xFollowLock");
				AddComponentMemberRemap<Volt::VisionCameraComponent>("Y FollowLock", "yFollowLock");
				AddComponentMemberRemap<Volt::VisionCameraComponent>("Z FollowLock", "zFollowLock");
				AddComponentMemberRemap<Volt::VisionCameraComponent>("X ShouldDamp", "xShouldDamp");
				AddComponentMemberRemap<Volt::VisionCameraComponent>("Y ShouldDamp", "yShouldDamp");
				AddComponentMemberRemap<Volt::VisionCameraComponent>("Z ShouldDamp", "zShouldDamp");
				AddComponentMemberRemap<Volt::VisionCameraComponent>("Additive Blend", "additiveBlend");
			}

			// NetActorComponent
			{
				AddComponentMemberRemap<Volt::NetActorComponent>("Condition", "condition");
				AddComponentMemberRemap<Volt::NetActorComponent>("Update Position", "updateTransformPos");
				AddComponentMemberRemap<Volt::NetActorComponent>("Update Rotation", "updateTransformRot");
				AddComponentMemberRemap<Volt::NetActorComponent>("Update Scale", "updateTransformScale");
				AddComponentMemberRemap<Volt::NetActorComponent>("RepId", "repId");
				AddComponentMemberRemap<Volt::NetActorComponent>("cID", "clientId");
			}

			// GameModeComponent
			{
				AddComponentMemberRemap<Volt::GameModeComponent>("PlayerPrefab", "prefabHandle");
				AddComponentMemberRemap<Volt::GameModeComponent>("EnemyPrefab", "enemy");
			}

			// AudioListenerComponent
			{
				AddComponentMemberRemap<Volt::AudioListenerComponent>("Default", "default");
			}
		}
	}

	inline void Shutdown()
	{
		s_preV113PropTypeToTypeIndexMap.clear();
		s_arrayDeserializers.clear();
		s_componentMemberRemap.clear();
	}

	void Convert()
	{
		Initialize();

		ConvertPrefabsToV013();
		ConvertScenesToV013();

		Shutdown();
	}
}
