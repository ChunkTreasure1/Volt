#include "vtpch.h"
#include "ShaderRegistry.h"

#include "Volt/Asset/AssetManager.h"
#include "Volt/Log/Log.h"

#include "Volt/Rendering/Shader/Shader.h"

#include "Volt/Utility/StringUtility.h"
#include "Volt/Utility/FileSystem.h"

namespace Volt
{
	void ShaderRegistry::Initialize()
	{
		LoadAllShaders();
	}

	void ShaderRegistry::Shutdown()
	{
		myRegistry.clear();
	}

	Ref<Shader> ShaderRegistry::Get(const std::string& name)
	{
		std::string lowName = Utils::ToLower(name);
		auto it = myRegistry.find(lowName);
		if (it == myRegistry.end())
		{
			VT_CORE_ERROR("Unable to find shader {0}!", name.c_str());
			return nullptr;
		}

		return it->second;
	}

	void ShaderRegistry::Register(const std::string& name, Ref<Shader> shader)
	{
		auto it = myRegistry.find(name);
		if (it != myRegistry.end())
		{
			VT_CORE_ERROR("Shader with that name has already been registered!");
			return;
		}

		std::string lowName = Utils::ToLower(name);
		myRegistry[lowName] = shader;
	}

	void ShaderRegistry::ReloadShadersWithShader(const std::filesystem::path& shaderPath)
	{
		for (const auto& [name, shader] : myRegistry)
		{
			if (shader->ContainsShader(shaderPath))
			{
				shader->Reload(true);
			}
		}
	}

	const std::map<std::string, Ref<Shader>>& ShaderRegistry::GetAllShaders()
	{
		return myRegistry;
	}

	void ShaderRegistry::LoadAllShaders()
	{
		auto shaderSearchFolder = FileSystem::GetShadersPath();
		for (const auto& path : std::filesystem::recursive_directory_iterator(shaderSearchFolder))
		{
			AssetType type = AssetManager::GetAssetTypeFromPath(path.path());
			if (type == AssetType::Shader)
			{
				if (!AssetManager::Get().ExistsInRegistry(path.path()))
				{
					AssetManager::Get().AddToRegistry(path.path());
				}

				Ref<Shader> shader = AssetManager::GetAsset<Shader>(path.path());
				Register(shader->GetName(), shader);
			}
		}
	}
}