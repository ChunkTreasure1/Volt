#include "sbpch.h"
#include "ProjectUpgradeModal.h"

#include <Volt/Utility/UIUtility.h>
#include <Volt/Utility/FileIO/YAMLStreamReader.h>

#include <Volt/Asset/Asset.h>
#include <Volt/Asset/Animation/AnimatedCharacter.h>
#include <Volt/Asset/Importers/AnimationGraphImporter.h>

#include <Volt/Asset/Animation/Skeleton.h>

#include <imgui.h>
#include <Volt/Utility/SerializationMacros.h>

#include "Wire/WireGUID.h"

ProjectUpgradeModal::ProjectUpgradeModal(const std::string& strId)
	: Modal(strId)
{
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
		ConvertMetaFilesFromV0();
	}
	if (projectVersion.GetPatch() < 2 && projectVersion.GetMinor() < 2 && projectVersion.GetMajor() == 0)
	{
		ConvertAnimationGraphsToV0_1_2();
	}

	Volt::ProjectManager::OnProjectUpgraded();
	Volt::ProjectManager::SerializeProject();
}

void ProjectUpgradeModal::ConvertMetaFilesFromV0()
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
		const auto [assetPath, handle] = DeserializeV0MetaFile(metaPath);
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
						WireGUID componentGUID;
						componentGUID.hiPart = compNode["guid"][0].as<uint64_t>();
						componentGUID.loPart = compNode["guid"][1].as<uint64_t>();
						if (componentGUID == WireGUID::Null())
						{
							continue;
						}

						// check if the component is an animation controller
						if (componentGUID.hiPart == 4626977537440075682 && componentGUID.loPart == 8666096866538760379)
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
						WireGUID componentGUID;
						componentGUID.hiPart = compNode["guid"][0].as<uint64_t>();
						componentGUID.loPart = compNode["guid"][1].as<uint64_t>();
						if (componentGUID == WireGUID::Null())
						{
							continue;
						}

						// check if the component is an animation controller
						if (componentGUID.hiPart == 4626977537440075682 && componentGUID.loPart == 8666096866538760379)
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
		resultAssetHandle = streamReader.ReadKey("assetHandle", uint64_t(0));

		streamReader.ExitScope();
	}
	else if (streamReader.HasKey("Handle"))
	{
		resultAssetHandle = streamReader.ReadKey("Handle", uint64_t(0));
		resultAssetFilePath = streamReader.ReadKey("Path", std::string(""));
	}

	return { resultAssetFilePath, resultAssetHandle };
}
