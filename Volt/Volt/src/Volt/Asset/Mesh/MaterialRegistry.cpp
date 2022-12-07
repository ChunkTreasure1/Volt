#include "vtpch.h"
#include "MaterialRegistry.h"

#include "Volt/Log/Log.h"

#include "Volt/Asset/AssetManager.h"
#include "Volt/Asset/Mesh/Material.h"

#include "Volt/Utility/FileSystem.h"
#include "Volt/Utility/StringUtility.h"
#include "Volt/Utility/SerializationMacros.h"

#include <yaml-cpp/yaml.h>

namespace Volt
{
	void MaterialRegistry::Initialize()
	{
		FindAllMaterials();
	}

	void MaterialRegistry::Shutdown()
	{
		s_registry.clear();
	}

	const std::filesystem::path MaterialRegistry::Get(const std::string& name)
	{
		std::string lowName = Utils::ToLower(name);
		auto it = s_registry.find(lowName);
		if (it == s_registry.end())
		{
			VT_CORE_ERROR("Unable to find material {0}!", name.c_str());
			return "";
		}

		return it->second;
	}

	void MaterialRegistry::Register(const std::string& name, const std::filesystem::path& path)
	{
		auto it = s_registry.find(name);
		if (it != s_registry.end())
		{
			VT_CORE_ERROR("Material with that name has already been registered!");
			return;
		}

		std::string lowName = Utils::ToLower(name);
		s_registry[lowName] = path;
	}

	void MaterialRegistry::FindAllMaterials()
	{
		for (const auto it : std::filesystem::recursive_directory_iterator(FileSystem::GetAssetsPath()))
		{
			if (!it.is_directory())
			{
				auto type = AssetManager::Get().GetAssetTypeFromPath(it.path());
				if (type == AssetType::Material)
				{
					std::ifstream file(it.path());
					if (!file.is_open()) [[unlikely]]
					{
						continue;
					}

					std::stringstream sstream;
					sstream << file.rdbuf();
					file.close();

					YAML::Node root = YAML::Load(sstream.str());
					YAML::Node multiMaterialNode = root["Material"];

					std::string nameString;
					VT_DESERIALIZE_PROPERTY(name, nameString, multiMaterialNode, std::string("Null"));

					Register(nameString, it.path());
				}
			}
		}
	}
}