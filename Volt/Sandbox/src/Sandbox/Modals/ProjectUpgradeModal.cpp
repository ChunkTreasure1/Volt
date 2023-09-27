#include "sbpch.h"
#include "ProjectUpgradeModal.h"

#include <Volt/Asset/Importers/SceneImporter.h>

#include <Volt/Utility/UIUtility.h>
#include <Volt/Utility/FileIO/YAMLStreamReader.h>

#include <imgui.h>

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

static std::unordered_map<PreV113PropertyType, std::type_index> s_preV113PropTypeToTypeIndexMap;
static bool s_initialize = false;

ProjectUpgradeModal::ProjectUpgradeModal(const std::string& strId)
	: Modal(strId)
{
	if (!s_initialize)
	{
		s_preV113PropTypeToTypeIndexMap[PreV113PropertyType::Bool] = std::type_index{ typeid(bool) };
		s_preV113PropTypeToTypeIndexMap[PreV113PropertyType::Int] = std::type_index{ typeid(int32_t) };
		s_preV113PropTypeToTypeIndexMap[PreV113PropertyType::UInt] = std::type_index{ typeid(uint32_t) };
		s_preV113PropTypeToTypeIndexMap[PreV113PropertyType::Short] = std::type_index{ typeid(int16_t) };
		s_preV113PropTypeToTypeIndexMap[PreV113PropertyType::UShort] = std::type_index{ typeid(uint16_t) };
		s_preV113PropTypeToTypeIndexMap[PreV113PropertyType::Char] = std::type_index{ typeid(int8_t) };
		s_preV113PropTypeToTypeIndexMap[PreV113PropertyType::UChar] = std::type_index{ typeid(uint8_t) };
		s_preV113PropTypeToTypeIndexMap[PreV113PropertyType::Float] = std::type_index{ typeid(float) };
		s_preV113PropTypeToTypeIndexMap[PreV113PropertyType::Double] = std::type_index{ typeid(double) };
		s_preV113PropTypeToTypeIndexMap[PreV113PropertyType::Vector2] = std::type_index{ typeid(glm::vec2) };
		s_preV113PropTypeToTypeIndexMap[PreV113PropertyType::Vector3] = std::type_index{ typeid(glm::vec3) };
		s_preV113PropTypeToTypeIndexMap[PreV113PropertyType::Vector4] = std::type_index{ typeid(glm::vec4) };
		s_preV113PropTypeToTypeIndexMap[PreV113PropertyType::String] = std::type_index{ typeid(std::string) };
		s_preV113PropTypeToTypeIndexMap[PreV113PropertyType::Int64] = std::type_index{ typeid(int64_t) };
		s_preV113PropTypeToTypeIndexMap[PreV113PropertyType::UInt64] = std::type_index{ typeid(uint64_t) };
		s_preV113PropTypeToTypeIndexMap[PreV113PropertyType::AssetHandle] = std::type_index{ typeid(Volt::AssetHandle) };
		s_preV113PropTypeToTypeIndexMap[PreV113PropertyType::Color3] = std::type_index{ typeid(glm::vec3) };
		s_preV113PropTypeToTypeIndexMap[PreV113PropertyType::Color4] = std::type_index{ typeid(glm::vec4) };
		s_preV113PropTypeToTypeIndexMap[PreV113PropertyType::Directory] = std::type_index{ typeid(std::filesystem::path) };
		s_preV113PropTypeToTypeIndexMap[PreV113PropertyType::Path] = std::type_index{ typeid(std::filesystem::path) };
		s_preV113PropTypeToTypeIndexMap[PreV113PropertyType::EntityId] = std::type_index{ typeid(entt::entity) };
		s_preV113PropTypeToTypeIndexMap[PreV113PropertyType::GUID] = std::type_index{ typeid(VoltGUID) };
		s_preV113PropTypeToTypeIndexMap[PreV113PropertyType::Quaternion] = std::type_index{ typeid(glm::quat) };

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
		ConvertScenesToV113();
		ConvertPrefabsToV113();
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

void ProjectUpgradeModal::ConvertPrefabsToV113()
{
}

void ProjectUpgradeModal::ConvertScenesToV113()
{
	// We only need to convert the layer files, and not the scene files

	// Find all scene directories
	auto& project = Volt::ProjectManager::GetProject();
	const std::filesystem::path assetsPath = project.projectDirectory / project.assetsDirectory;

	std::vector<std::filesystem::path> sceneDirectories;
	for (const auto& entry : std::filesystem::recursive_directory_iterator(assetsPath))
	{
		if (entry.path().extension().string() == ".vtscene")
		{
			sceneDirectories.emplace_back(entry.path().parent_path());
		}
	}

	// Convert all found scenes
	for (const auto& sceneDir : sceneDirectories)
	{
		if (!std::filesystem::exists(sceneDir / "Layers"))
		{
			continue;
		}

		Ref<Volt::Scene> scene = CreateRef<Volt::Scene>();
		std::vector<Volt::SceneLayer> sceneLayers;

		for (const auto& entry : std::filesystem::directory_iterator(sceneDir / "Layers"))
		{
			if (entry.path().string() != ".vtlayer")
			{
				continue;
			}

			DeserializePreV113SceneLayer(scene, sceneLayers.emplace_back(), entry.path());
		}
	}
}

void ProjectUpgradeModal::DeserializePreV113SceneLayer(Ref<Volt::Scene> scene, Volt::SceneLayer& sceneLayer, const std::filesystem::path& layerPath)
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
			DeserializePreV113Entity(scene, streamReader);
		});
	}
	streamReader.ExitScope();
}

void ProjectUpgradeModal::DeserializePreV113Entity(Ref<Volt::Scene> scene, Volt::YAMLStreamReader& streamReader)
{
	streamReader.EnterScope("Entity");

	entt::entity entityId = streamReader.ReadKey("id", (entt::entity)entt::null);

	if (IsPreV113EntityNull(entityId))
	{
		return;
	}

	entityId = scene->GetRegistry().create(entityId);

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
				Volt::ComponentRegistry::Helpers::AddComponentWithGUID(compGuid, scene->GetRegistry(), entityId);
				void* voidCompPtr = Volt::ComponentRegistry::Helpers::GetComponentWithGUID(compGuid, scene->GetRegistry(), entityId);
				uint8_t* componentData = reinterpret_cast<uint8_t*>(voidCompPtr);

				const Volt::IComponentTypeDesc* componentDesc = reinterpret_cast<const Volt::IComponentTypeDesc*>(typeDesc);
				DeserializePreV113Component(componentData, componentDesc, streamReader);
				break;
			}
		}
	});

	if (scene->GetRegistry().any_of<Volt::MonoScriptComponent>(entityId))
	{
		Volt::SceneImporter::Get().DeserializeMono(entityId, scene, streamReader);
	}

	streamReader.ExitScope();
}

void ProjectUpgradeModal::DeserializePreV113Component(uint8_t* componentData, const Volt::IComponentTypeDesc* componentDesc, Volt::YAMLStreamReader& streamReader)
{
	const auto& typeDeserializers = Volt::SceneImporter::GetTypeDeserializers();

	streamReader.ForEach("members", [&]() 
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
			const std::type_index vectorValueType = s_preV113PropTypeToTypeIndexMap.at(vectorType);
			void* arrayPtr = &componentData[componentMember->offset];

			if (arrayTypeDesc->GetElementTypeIndex() != vectorValueType)
			{
				VT_CORE_WARN("[Upgrade Project]: Component member vector type does not match file specified type!");
				return;
			}

			streamReader.ForEach("data", [&]() 
			{
				uint8_t* tempDataStorage = new uint8_t[arrayTypeDesc->GetElementTypeSize()];
				memset(tempDataStorage, 0, arrayTypeDesc->GetElementTypeSize());

				if (typeDeserializers.contains(vectorValueType))
				{
					typeDeserializers.at(vectorValueType)(streamReader, tempDataStorage, 0);
				}

				arrayTypeDesc->PushBack(arrayPtr, tempDataStorage);
				delete[] tempDataStorage;
			});

			return;
		}
		else if (oldType == PreV113PropertyType::Enum)
		{
			typeDeserializers.at(typeid(int32_t))(streamReader, componentData, componentMember->offset);
			return;
		}

		const std::type_index memberType = s_preV113PropTypeToTypeIndexMap.at(oldType);
		if (memberType != componentMember->typeIndex)
		{
			VT_CORE_WARN("[Upgrade Project]: Component member type does not match file specified type!");
			return;
		}

		if (typeDeserializers.contains(memberType))
		{
			typeDeserializers.at(memberType)(streamReader, componentData, componentMember->offset);
		}
	});
}

const bool ProjectUpgradeModal::IsPreV113EntityNull(entt::entity entityId)
{
	return entityId == entt::null || entityId == static_cast<entt::entity>(0);
}

const Volt::ComponentMember* ProjectUpgradeModal::TryGetComponentMemberFromName(const std::string& memberName, const Volt::IComponentTypeDesc* componentDesc)
{
	std::string demangledName = Utility::ToLower(memberName);
	demangledName.erase(std::remove_if(demangledName.begin(), demangledName.end(), ::isspace));

	for (uint32_t idx = 0; const auto& member : componentDesc->GetMembers())
	{
		const std::string memberNameLower = Utility::ToLower(std::string(member.name));

		if (demangledName.find(memberNameLower) != std::string::npos)
		{
			return &componentDesc->GetMembers().at(idx);
		}

		idx++;
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
