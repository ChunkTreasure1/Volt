#include "sbpch.h"
#include "ProjectUpgradeModal.h"

#include <Volt/Asset/Importers/SceneImporter.h>
#include <Volt/Asset/Importers/PrefabImporter.h>
#include <Volt/Asset/Prefab.h>

#include <Volt/Utility/UIUtility.h>
#include <Volt/Utility/FileIO/YAMLStreamReader.h>

#include <Volt/Asset/Asset.h>
#include <Volt/Asset/Animation/AnimatedCharacter.h>
#include <Volt/Asset/Importers/AnimationGraphImporter.h>

#include <Volt/Asset/Animation/Skeleton.h>
	
#include <Volt/Scripting/Mono/MonoScriptEngine.h>

#include <Volt/Components/RenderingComponents.h>

#include <imgui.h>
#include <Volt/Utility/SerializationMacros.h>

enum class PreV113PropertyType : uint32_t
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

static std::unordered_map<PreV113PropertyType, TypeIndexContainer> s_preV113PropTypeToTypeIndexMap;
static std::unordered_map<std::type_index, std::function<void(Volt::YAMLStreamReader&, uint8_t*, const size_t)>> s_arrayDeserializers;
static std::unordered_map<VoltGUID, std::unordered_map<std::string, std::string>> s_componentMemberRemap;

static bool s_initialize = false;

template<typename T>
void RegisterArrayDeserializationFunction()
{
	s_arrayDeserializers[std::type_index{ typeid(T) }] = [](Volt::YAMLStreamReader& streamReader, uint8_t* data, const size_t offset)
	{
		*reinterpret_cast<T*>(&data[offset]) = streamReader.ReadKey("value", T());
	};
}

ProjectUpgradeModal::ProjectUpgradeModal(const std::string& strId)
	: Modal(strId)
{
	if (!s_initialize)
	{
		s_preV113PropTypeToTypeIndexMap[PreV113PropertyType::Bool].typeIndex = std::type_index{ typeid(bool) };
		s_preV113PropTypeToTypeIndexMap[PreV113PropertyType::Int].typeIndex = std::type_index{ typeid(int32_t) };
		s_preV113PropTypeToTypeIndexMap[PreV113PropertyType::UInt].typeIndex = std::type_index{ typeid(uint32_t) };
		s_preV113PropTypeToTypeIndexMap[PreV113PropertyType::Short].typeIndex = std::type_index{ typeid(int16_t) };
		s_preV113PropTypeToTypeIndexMap[PreV113PropertyType::UShort].typeIndex = std::type_index{ typeid(uint16_t) };
		s_preV113PropTypeToTypeIndexMap[PreV113PropertyType::Char].typeIndex = std::type_index{ typeid(int8_t) };
		s_preV113PropTypeToTypeIndexMap[PreV113PropertyType::UChar].typeIndex = std::type_index{ typeid(uint8_t) };
		s_preV113PropTypeToTypeIndexMap[PreV113PropertyType::Float].typeIndex = std::type_index{ typeid(float) };
		s_preV113PropTypeToTypeIndexMap[PreV113PropertyType::Double].typeIndex = std::type_index{ typeid(double) };
		s_preV113PropTypeToTypeIndexMap[PreV113PropertyType::Vector2].typeIndex = std::type_index{ typeid(glm::vec2) };
		s_preV113PropTypeToTypeIndexMap[PreV113PropertyType::Vector3].typeIndex = std::type_index{ typeid(glm::vec3) };
		s_preV113PropTypeToTypeIndexMap[PreV113PropertyType::Vector4].typeIndex = std::type_index{ typeid(glm::vec4) };
		s_preV113PropTypeToTypeIndexMap[PreV113PropertyType::String].typeIndex = std::type_index{ typeid(std::string) };
		s_preV113PropTypeToTypeIndexMap[PreV113PropertyType::Int64].typeIndex = std::type_index{ typeid(int64_t) };
		s_preV113PropTypeToTypeIndexMap[PreV113PropertyType::UInt64].typeIndex = std::type_index{ typeid(uint64_t) };
		s_preV113PropTypeToTypeIndexMap[PreV113PropertyType::AssetHandle].typeIndex = std::type_index{ typeid(Volt::AssetHandle) };
		s_preV113PropTypeToTypeIndexMap[PreV113PropertyType::Color3].typeIndex = std::type_index{ typeid(glm::vec3) };
		s_preV113PropTypeToTypeIndexMap[PreV113PropertyType::Color4].typeIndex = std::type_index{ typeid(glm::vec4) };
		s_preV113PropTypeToTypeIndexMap[PreV113PropertyType::Directory].typeIndex = std::type_index{ typeid(std::filesystem::path) };
		s_preV113PropTypeToTypeIndexMap[PreV113PropertyType::Path].typeIndex = std::type_index{ typeid(std::filesystem::path) };
		s_preV113PropTypeToTypeIndexMap[PreV113PropertyType::EntityId].typeIndex = std::type_index{ typeid(entt::entity) };
		s_preV113PropTypeToTypeIndexMap[PreV113PropertyType::GUID].typeIndex = std::type_index{ typeid(VoltGUID) };
		s_preV113PropTypeToTypeIndexMap[PreV113PropertyType::Quaternion].typeIndex = std::type_index{ typeid(glm::quat) };

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

		RegisterArrayDeserializationFunction<entt::entity>();
		RegisterArrayDeserializationFunction<Volt::AssetHandle>();

		// Member remapping
		{
			s_componentMemberRemap[Volt::ComponentRegistry::GetGUIDFromTypeName(entt::type_name<Volt::MeshComponent>())]["Mesh"] = "handle";
			s_componentMemberRemap[Volt::ComponentRegistry::GetGUIDFromTypeName(entt::type_name<Volt::AnimatedCharacterComponent>())]["Character"] = "animatedCharacter";
			s_componentMemberRemap[Volt::ComponentRegistry::GetGUIDFromTypeName(entt::type_name<Volt::SpriteComponent>())]["Material"] = "materialHandle";
			s_componentMemberRemap[Volt::ComponentRegistry::GetGUIDFromTypeName(entt::type_name<Volt::AnimationControllerComponent>())]["Override Skin"] = "skin";
			s_componentMemberRemap[Volt::ComponentRegistry::GetGUIDFromTypeName(entt::type_name<Volt::DecalComponent>())]["Material"] = "decalMaterial";
		}

		s_initialize = true;
	}
}

void ProjectUpgradeModal::DrawModalContent()
{
	const std::string engineVersion = Volt::Application::Get().GetInfo().version.ToString();
	const std::string projectVersion = Volt::ProjectManager::GetProject().engineVersion.ToString();

	ImGui::TextUnformatted("The loaded project is not compatible with this version of the engine!");

	if (projectVersion.empty())
	{
		ImGui::Text("Would you like to upgrade the project to version %s?", engineVersion.c_str());
	}
	else
	{
		ImGui::Text("Would you like to upgrade the project from version %s to version %s?", projectVersion.c_str(), engineVersion.c_str());
	}

	if (ImGui::Button("Yes"))
	{
		UpgradeCurrentProject();

		ImGui::CloseCurrentPopup();
	}

	ImGui::SameLine();

	if (ImGui::Button("No"))
	{
		ImGui::CloseCurrentPopup();
	}
}

void ProjectUpgradeModal::OnOpen()
{
}

void ProjectUpgradeModal::OnClose()
{
}

void ProjectUpgradeModal::UpgradeCurrentProject()
{
	const Volt::Version engineVersion = Volt::Application::Get().GetInfo().version;
	const Volt::Version projectVersion = Volt::ProjectManager::GetProject().engineVersion;

	if (projectVersion.GetMinor() < 1 && projectVersion.GetMajor() == 0)
	{
		ConvertMetaFilesToV011();
	}

	if (projectVersion.GetMinor() < 3 && projectVersion.GetMajor() == 0)
	{
		ConvertPrefabsToV113();
		ConvertScenesToV113();
	}
	if (projectVersion.GetPatch() < 2 && projectVersion.GetMinor() < 2 && projectVersion.GetMajor() == 0)
	{
		ConvertAnimationGraphsToV0_1_2();
	}

	Volt::ProjectManager::OnProjectUpgraded();
	Volt::ProjectManager::SerializeProject();
}

void ProjectUpgradeModal::ConvertMetaFilesToV011()
{
	auto& project = Volt::ProjectManager::GetProject();

	const std::filesystem::path assetsPath = project.projectDirectory / project.assetsDirectory;

	std::vector<std::filesystem::path> metaFilesToConvert;

	for (const auto& entry : std::filesystem::recursive_directory_iterator(assetsPath))
	{
		if (entry.path().extension().string() != ".vtmeta")
		{
			continue;
		}

		metaFilesToConvert.emplace_back(entry.path());
	}

	struct AssetDescriptor
	{
		Volt::AssetHandle handle;
		std::filesystem::path assetFilePath;
	};

	std::vector<AssetDescriptor> assetDescriptors;

	for (const auto& metaPath : metaFilesToConvert)
	{
		auto [assetPath, handle] = DeserializeV0MetaFile(metaPath);
		assetDescriptors.emplace_back(handle, assetPath);

		if (std::filesystem::exists(metaPath))
		{
			std::filesystem::remove(metaPath);
		}
	}

	for (const auto& descriptor : assetDescriptors)
	{
		Volt::AssetManager::Get().AddAssetToRegistry(descriptor.assetFilePath, descriptor.handle);
	}
}

void ProjectUpgradeModal::ConvertAnimationGraphsToV0_1_2()
{
	auto& project = Volt::ProjectManager::GetProject();

	const std::filesystem::path assetsPath = project.projectDirectory / project.assetsDirectory;

	std::vector<std::filesystem::path> animGraphsToConvert;
	std::vector<std::filesystem::path> characterAssets;
	std::vector<std::filesystem::path> layerAssets;
	std::vector<std::filesystem::path> prefabAssets;
	for (const auto& entry : std::filesystem::recursive_directory_iterator(assetsPath))
	{
		if (entry.path().extension().string() == ".vtanimgraph")
		{
			animGraphsToConvert.emplace_back(entry.path());
		}

		if (entry.path().extension().string() == ".vtchr")
		{
			characterAssets.emplace_back(entry.path());
		}

		if (entry.path().extension().string() == ".vtlayer")
		{
			layerAssets.emplace_back(entry.path());
		}

		if (entry.path().extension().string() == ".vtprefab")
		{
			prefabAssets.emplace_back(entry.path());
		}
	}

	auto parseCharacterWithHandle = [characterAssets](Volt::AssetHandle handle, Volt::AssetHandle& outSkinHandle, Volt::AssetHandle& outSkeletonHandle)
	{
		for (const auto& characterPath : characterAssets)
		{
			Volt::YAMLStreamReader metaStreamReader{};

			if (!metaStreamReader.OpenFile(characterPath.string() + ".vtmeta"))
			{
				return false;
			}

			Volt::AssetHandle characterAssetHandle = 0;

			metaStreamReader.EnterScope("Metadata");

			characterAssetHandle = metaStreamReader.ReadKey("assetHandle", uint64_t(0));

			if (characterAssetHandle == handle)
			{
				Volt::YAMLStreamReader charStreamReader{};

				if (!charStreamReader.OpenFile(characterPath))
				{
					return false;
				}

				charStreamReader.EnterScope("AnimatedCharacter");
				outSkeletonHandle = charStreamReader.ReadKey("skeleton", uint64_t(0));
				outSkinHandle = charStreamReader.ReadKey("skin", uint64_t(0));
				return true;
			}
		}
		return false;
	};

	struct AnimGraphDescriptor
	{
		Volt::AssetHandle handle = 0;
		Volt::AssetHandle skeletonHandle = 0;
		Volt::AssetHandle skinHandle = 0;
	};

	std::vector<AnimGraphDescriptor> animGraphDescriptors;
	animGraphDescriptors.resize(animGraphsToConvert.size());

	for (const auto& animGraphPath : animGraphsToConvert)
	{
		AnimGraphDescriptor& descriptor = animGraphDescriptors.emplace_back();
		{ // convert the animgraph
			{// parse meta
				Volt::YAMLStreamReader metaStreamReader{};

				if (!metaStreamReader.OpenFile(animGraphPath.string() + ".vtmeta"))
				{
					VT_CORE_ASSERT(false);
				}

				Volt::AssetHandle characterAssetHandle = 0;

				metaStreamReader.EnterScope("Metadata");

				descriptor.handle = metaStreamReader.ReadKey("assetHandle", uint64_t(0));
			}

			std::ifstream input(animGraphPath);
			if (!input.is_open())
			{
				VT_CORE_ERROR("Failed to convert: File {0} not found!", animGraphPath);
				continue;
			}

			std::stringstream sstream;
			sstream << input.rdbuf();
			input.close();

			YAML::Node root;

			root = YAML::Load(sstream.str());

			YAML::Node rootNode = root["AnimationGraph"];
			Volt::AssetHandle characterHandle = rootNode["character"].as<uint64_t>();
			rootNode.remove("character");

			parseCharacterWithHandle(characterHandle, descriptor.skinHandle, descriptor.skeletonHandle);

			uint64_t apa2 = descriptor.skinHandle;
			apa2;

			uint64_t skeletonHandleInt = static_cast<uint64_t>(descriptor.skeletonHandle);
			rootNode.force_insert("skeleton", skeletonHandleInt);


			YAML::Node graphNode = rootNode["Graph"];

			for (int nodeIndex = 0; graphNode["Nodes"][nodeIndex]; nodeIndex++)
			{
				auto node = graphNode["Nodes"][nodeIndex];
				if (node["type"].as<std::string>() == "StateMachineNode")
				{
					auto nodeSpecific = node["nodeSpecific"];
					auto stateMachineNode = nodeSpecific["StateMachine"];
					stateMachineNode.remove("characterHandle");
					stateMachineNode.force_insert("skeletonHandle", skeletonHandleInt);

					//loop through all states
					for (int i = 0; stateMachineNode["States"][i]; i++)
					{
						auto state = stateMachineNode["States"][i];

						if (state["characterHandle"])
						{
							state.remove("characterHandle");
							state.force_insert("skeletonHandle", skeletonHandleInt);
						}
						state.force_insert("topPinId", state["pinId"].as<uint64_t>());
						state.force_insert("bottomPinId", state["pinId2"].as<uint64_t>());
						state.remove("pinId");
						state.remove("pinId2");

						bool isAny = state["isAny"].as<bool>();
						bool isEntry = state["isEntry"].as<bool>();
						state.force_insert("stateType", isAny ? "AliasState" : isEntry ? "EntryState" : "AnimationState");

						if (isAny)
						{
							YAML::Node arrayNode(YAML::NodeType::Sequence);  // Create a new Node of Sequence type.
							for (auto stateLoopTwo : stateMachineNode["States"])
							{
								if (!(stateLoopTwo["isAny"].as<bool>()) && !(stateLoopTwo["isEntry"].as<bool>()))
								{
									arrayNode.push_back(stateLoopTwo["id"].as<uint64_t>());  // Add elements to the Sequence Node.
								}
							}

							state.force_insert("TransitionFromStates", arrayNode);
						}
					}
					//loop through them again and remove the bools
					for (int i = 0; stateMachineNode["States"][i]; i++)
					{
						auto state = stateMachineNode["States"][i];
						state.remove("isAny");
						state.remove("isEntry");
					}
				}
			}

			std::ofstream output(animGraphPath);
			if (!output.is_open())
			{
				VT_CORE_ERROR("Failed to convert: File {0} not found!", animGraphPath);
				continue;
			}

			output << root;
			output.close();
		}
	}

	auto createMeshComponentNode = [](Volt::AssetHandle skinHandle)
	{
		YAML::Node meshComponentNode;

		YAML::Node guidNode(YAML::NodeType::Sequence);
		guidNode.push_back(5579790378469427390);
		guidNode.push_back(5138106327942809248);
		meshComponentNode.force_insert("guid", guidNode); // guid for mesh component

		YAML::Node  propertiesNode(YAML::NodeType::Sequence);

		YAML::Node propertyNodeMesh;
		propertyNodeMesh["type"] = 16;
		propertyNodeMesh["vectorType"] = 13;
		propertyNodeMesh["name"] = "Mesh";
		propertyNodeMesh["data"] = static_cast<uint64_t>(skinHandle);
		propertiesNode.push_back(propertyNodeMesh);

		YAML::Node propertyNodeMaterial;
		propertyNodeMaterial["type"] = 16;
		propertyNodeMaterial["vectorType"] = 13;
		propertyNodeMaterial["name"] = "Material";
		propertyNodeMaterial["data"] = 0;
		propertiesNode.push_back(propertyNodeMaterial);

		meshComponentNode.force_insert("properties", propertiesNode);
		return meshComponentNode;
	};

	for (auto& layer : layerAssets)
	{
		bool modified = false;
		std::ifstream input(layer);
		if (!input.is_open())
		{
			VT_CORE_ERROR("Failed to convert: File {0} not found!", layer);
			continue;
		}

		std::stringstream sstream;
		sstream << input.rdbuf();
		input.close();

		YAML::Node root;

		root = YAML::Load(sstream.str());

		YAML::Node rootNode = root["Layer"];

		if (rootNode["Entities"])
		{
			for (int entityIndex = 0; rootNode["Entities"][entityIndex]; entityIndex++)
			{
				auto entity = rootNode["Entities"][entityIndex]["Entity"];
				if (entity["components"])
				{

					for (int componentIndex = 0; entity["components"][componentIndex]; componentIndex++)
					{
						auto compNode = entity["components"][componentIndex];
						std::pair<uint64_t, uint64_t> componentGUID;
						componentGUID.first = compNode["guid"][0].as<uint64_t>();
						componentGUID.second = compNode["guid"][1].as<uint64_t>();
						if (componentGUID.first != 0 && componentGUID.second != 0)
						{
							continue;
						}

						// check if the component is an animation controller
						if (componentGUID.first == 4626977537440075682 && componentGUID.second == 8666096866538760379)
						{
							if (compNode["properties"].size() > 0)
							{
								uint64_t compGraphAssetId = compNode["properties"][0]["data"].as<uint64_t>();

								for (auto& descriptor : animGraphDescriptors)
								{
									if (descriptor.handle == compGraphAssetId)
									{
										entity["components"].push_back(createMeshComponentNode(descriptor.skinHandle));
										modified = true;
										break;
									}
								}
							}
						}
					}
				}
			}
		}

		if (modified)
		{
			std::ofstream output(layer);
			if (!output.is_open())
			{
				VT_CORE_ERROR("Failed to convert: File {0} not found!", layer);
				continue;
			}

			output << root;
			output.close();
		}
	}

	for (auto& prefab : prefabAssets)
	{
		bool modified = false;
		std::ifstream input(prefab);
		if (!input.is_open())
		{
			VT_CORE_ERROR("Failed to convert: File {0} not found!", prefab);
			continue;
		}

		std::stringstream sstream;
		sstream << input.rdbuf();
		input.close();

		YAML::Node root;

		root = YAML::Load(sstream.str());

		YAML::Node rootNode = root["Prefab"];

		if (rootNode["entities"])
		{
			for (int entityIndex = 0; rootNode["entities"][entityIndex]; entityIndex++)
			{
				auto entity = rootNode["entities"][entityIndex];
				if (entity["components"])
				{

					for (int componentIndex = 0; entity["components"][componentIndex]; componentIndex++)
					{
						auto compNode = entity["components"][componentIndex];
						std::pair<uint64_t, uint64_t> componentGUID;
						componentGUID.first = compNode["guid"][0].as<uint64_t>();
						componentGUID.second= compNode["guid"][1].as<uint64_t>();
						if (componentGUID.first != 0 && componentGUID.second != 0)
						{
							continue;
						}

						// check if the component is an animation controller
						if (componentGUID.first == 4626977537440075682 && componentGUID.second == 8666096866538760379)
						{
							if (compNode["properties"].size() > 0)
							{
								uint64_t compGraphAssetId = compNode["properties"][0]["data"].as<uint64_t>();
								for (auto& descriptor : animGraphDescriptors)
								{
									if (descriptor.handle == compGraphAssetId)
									{
										modified = true;
										entity["components"].push_back(createMeshComponentNode(descriptor.skinHandle));
										break;
									}
								}
							}
						}
					}
				}
			}
		}
		if (modified)
		{
			std::ofstream output(prefab);
			if (!output.is_open())
			{
				VT_CORE_ERROR("Failed to convert: File {0} not found!", prefab);
				continue;
			}

			output << root;
			output.close();
		}
	}
}

void ProjectUpgradeModal::ConvertPrefabsToV113()
{
	auto& project = Volt::ProjectManager::GetProject();
	const std::filesystem::path assetsPath = project.projectDirectory / project.assetsDirectory;

	std::vector<std::filesystem::path> prefabFilePaths;
	for (const auto& entry : std::filesystem::recursive_directory_iterator(assetsPath))
	{
		if (entry.path().extension().string() == ".vtprefab")
		{
			prefabFilePaths.emplace_back(entry.path());
		}
	}

	for (const auto& prefabFilePath : prefabFilePaths)
	{
		ConvertPreV113Prefab(prefabFilePath);
	}
}

void ProjectUpgradeModal::ConvertScenesToV113()
{
	// We only need to convert the layer files, and not the scene files

	// Find all scene directories
	auto& project = Volt::ProjectManager::GetProject();
	const std::filesystem::path assetsPath = project.projectDirectory / project.assetsDirectory;

	std::vector<std::filesystem::path> sceneFilePaths;
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
			Volt::YAMLStreamReader streamReader{};
			if (!streamReader.OpenFile(sceneFilePath))
			{
				VT_CORE_ERROR("[Project Upgrade]: Unable to open scene file! Skipping!");
				continue;
			}

			streamReader.EnterScope("Scene");
			sceneName = streamReader.ReadKey("name", std::string("New Scene"));
			streamReader.ExitScope();
		}

		Ref<Volt::Scene> scene = CreateRef<Volt::Scene>(sceneName);
		std::vector<Volt::SceneLayer> sceneLayers;

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

void ProjectUpgradeModal::ConvertPreV113Prefab(const std::filesystem::path& filePath)
{
	Volt::YAMLStreamReader streamReader{};
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
		version = streamReader.ReadKey("version", 0u);

		streamReader.ForEach("entities", [&]()
		{
			Volt::EntityID entityId = streamReader.ReadKey("id", Volt::Entity::NullID());
			if (IsPreV113EntityNull(entityId))
			{
				return;
			}

			DeserializePreV113Entity(prefabScene, streamReader, entityRemapping, true);
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

void ProjectUpgradeModal::DeserializePreV113SceneLayer(Ref<Volt::Scene> scene, Volt::SceneLayer& sceneLayer, const std::filesystem::path& layerPath, std::map<Volt::EntityID, Volt::EntityID>& entityRemapping)
{
	Volt::YAMLStreamReader streamReader{};

	if (!streamReader.OpenFile(layerPath))
	{
		return;
	}

	streamReader.EnterScope("Layer");
	{
		sceneLayer.name = streamReader.ReadKey("name", std::string("Null"));
		sceneLayer.id = streamReader.ReadKey("id", 0u);
		sceneLayer.visible = streamReader.ReadKey("visible", true);
		sceneLayer.locked = streamReader.ReadKey("locked", false);

		streamReader.ForEach("Entities", [&]()
		{
			DeserializePreV113Entity(scene, streamReader, entityRemapping, false);
		});
	}
	streamReader.ExitScope();
}

void ProjectUpgradeModal::DeserializePreV113Entity(Ref<Volt::Scene> scene, Volt::YAMLStreamReader& streamReader, std::map<Volt::EntityID, Volt::EntityID>& entityRemapping, bool isPrefabEntity)
{
	if (!isPrefabEntity)
	{
		streamReader.EnterScope("Entity");
	}

	Volt::EntityID originalEntityId = streamReader.ReadKey("id", Volt::Entity::NullID());

	if (IsPreV113EntityNull(originalEntityId))
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
		const VoltGUID compGuid = streamReader.ReadKey("guid", VoltGUID::Null());
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

void ProjectUpgradeModal::DeserializePreV113Component(uint8_t* componentData, const Volt::IComponentTypeDesc* componentDesc, Volt::YAMLStreamReader& streamReader)
{
	const auto& typeDeserializers = Volt::SceneImporter::GetTypeDeserializers();

	streamReader.ForEach("properties", [&]()
	{
		const std::string memberName = streamReader.ReadKey("name", std::string(""));

		if (memberName.empty())
		{
			return;
		}

		const Volt::ComponentMember* componentMember = TryGetComponentMemberFromName(memberName, componentDesc);
		if (!componentMember)
		{
			return;
		}

		const PreV113PropertyType oldType = static_cast<PreV113PropertyType>(streamReader.ReadKey("type", 0u));

		if (oldType == PreV113PropertyType::Vector && componentMember->typeDesc != nullptr)
		{
			const Volt::IArrayTypeDesc* arrayTypeDesc = reinterpret_cast<const Volt::IArrayTypeDesc*>(componentMember->typeDesc);
			const PreV113PropertyType vectorType = static_cast<PreV113PropertyType>(streamReader.ReadKey("vectorType", 0u));
			const std::type_index vectorValueType = s_preV113PropTypeToTypeIndexMap.at(vectorType).typeIndex;
			void* arrayPtr = &componentData[componentMember->offset];

			if (arrayTypeDesc->GetElementTypeIndex() != vectorValueType)
			{
				VT_CORE_WARN("[Upgrade Project]: Component member vector type does not match file specified type!");
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
		else if (oldType == PreV113PropertyType::Enum)
		{
			typeDeserializers.at(typeid(int32_t))(streamReader, componentData, componentMember->offset);
			return;
		}

		const std::type_index memberType = s_preV113PropTypeToTypeIndexMap.at(oldType).typeIndex;
		if (memberType != componentMember->typeIndex)
		{
			VT_CORE_WARN("[Upgrade Project]: Component member type does not match file specified type!");
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

void ProjectUpgradeModal::DeserializePreV113MonoScripts(Ref<Volt::Scene> scene, const Volt::EntityID entityId, Volt::YAMLStreamReader& streamReader)
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

		const std::string scriptName = streamReader.ReadKey("name", std::string(""));
		Volt::UUID scriptId = streamReader.ReadKey("id", Volt::UUID(0));

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
			const std::string memberName = streamReader.ReadKey("name", std::string(""));
			if (!classFields.contains(memberName))
			{
				return;
			}

			Ref<Volt::MonoScriptFieldInstance> fieldInstance = CreateRef<Volt::MonoScriptFieldInstance>();
			fieldCache[memberName] = fieldInstance;

			fieldInstance->field = classFields.at(memberName);
			if (fieldInstance->field.type.IsString())
			{
				const std::string str = streamReader.ReadKey("data", std::string(""));
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

void ProjectUpgradeModal::HandleEntityRemapping(Ref<Volt::Scene> scene, const std::map<Volt::EntityID, Volt::EntityID>& entityRemapping)
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

void ProjectUpgradeModal::HandleEntityArrayRemapping(Ref<Volt::Scene> scene, const std::map<Volt::EntityID, Volt::EntityID>& entityRemapping, const Volt::ComponentMember& componentMember, uint8_t* componentData)
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

void ProjectUpgradeModal::ValidateSceneConversion(Ref<Volt::Scene> scene)
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

void ProjectUpgradeModal::ValidateSceneConversionArray(Ref<Volt::Scene> scene, const Volt::ComponentMember& componentMember, uint8_t* componentData)
{
	const Volt::IArrayTypeDesc* memberArrayTypeDesc = reinterpret_cast<const Volt::IArrayTypeDesc*>(componentMember.typeDesc);
	void* arrayPtr = reinterpret_cast<void*>(&componentData[componentMember.offset]);

	std::vector<size_t> indicesToRemove{};

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

const bool ProjectUpgradeModal::IsPreV113EntityNull(Volt::EntityID entityId)
{
	return entityId == Volt::Entity::NullID();
}

void ProjectUpgradeModal::ValidateEntityValidity(Volt::EntityID* entityId)
{
	if (*entityId == (Volt::EntityID)0)
	{
		*entityId = Volt::Entity::NullID();
	}
}

const Volt::ComponentMember* ProjectUpgradeModal::TryGetComponentMemberFromName(const std::string& memberName, const Volt::IComponentTypeDesc* componentDesc)
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

std::pair<std::filesystem::path, Volt::AssetHandle> ProjectUpgradeModal::DeserializeV0MetaFile(const std::filesystem::path& metaPath)
{
	Volt::YAMLStreamReader streamReader{};

	if (!streamReader.OpenFile(metaPath))
	{
		return {};
	}

	std::filesystem::path resultAssetFilePath;
	Volt::AssetHandle resultAssetHandle = 0;

	// Is new
	if (streamReader.HasKey("Metadata"))
	{
		streamReader.EnterScope("Metadata");

		resultAssetFilePath = streamReader.ReadKey("filePath", std::string(""));
		resultAssetHandle = streamReader.ReadKey("assetHandle", uint64_t(0u));

		streamReader.ExitScope();
	}
	else if (streamReader.HasKey("Handle"))
	{
		resultAssetHandle = streamReader.ReadKey("Handle", uint64_t(0));
		resultAssetFilePath = streamReader.ReadKey("Path", std::string(""));
	}

	return { resultAssetFilePath, resultAssetHandle };
}
