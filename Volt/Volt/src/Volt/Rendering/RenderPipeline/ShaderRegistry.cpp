#include "vtpch.h"
#include "ShaderRegistry.h"

#include "Volt/Project/ProjectManager.h"

#include "Volt/Utility/FileSystem.h"

#include "Volt/Asset/AssetManager.h"

#include "Volt/Rendering/RenderPipeline/RenderPipeline.h"
#include "Volt/Rendering/Shader/Shader.h"
#include "Volt/Rendering/Shader/ShaderUtility.h"

namespace Volt
{
	void ShaderRegistry::Initialize()
	{
		LoadAllShaders();
	}

	void ShaderRegistry::Shutdown()
	{
		myShaderRegistry.clear();
	}

	void ShaderRegistry::Register(const std::string& name, Ref<Shader> shader)
	{
		std::string lowName = ::Utility::ToLower(name);
		if (myShaderRegistry.contains(lowName))
		{
			VT_CORE_ERROR("Shader with that name has already been registered!");
			return;
		}

		myShaderRegistry[lowName] = shader;

		for (const auto& p : shader->GetSourcePaths())
		{
			const auto sourceRelPath = Volt::AssetManager::GetRelativePath(p);

			if (!AssetManager::ExistsInRegistry(sourceRelPath))
			{
				AssetManager::Get().AddAssetToRegistry(sourceRelPath);
			}

			AssetManager::Get().AddDependency(shader->handle, sourceRelPath);
		}
	}

	void ShaderRegistry::Unregister(const std::string& name)
	{
		std::string lowName = ::Utility::ToLower(name);
		if (!myShaderRegistry.contains(lowName))
		{
			VT_CORE_ERROR("Shader with that name has not been registered!");
			return;
		}

		myShaderRegistry.erase(lowName);
	}

	bool ShaderRegistry::IsShaderRegistered(const std::string& name)
	{
		std::string lowName = ::Utility::ToLower(name);
		return myShaderRegistry.contains(lowName);
	}

	Ref<Shader> ShaderRegistry::GetShader(const std::string& name)
	{
		std::string lowName = ::Utility::ToLower(name);
		if (!myShaderRegistry.contains(lowName))
		{
			VT_CORE_ERROR("Unable to find shader {0}!", name.c_str());
			return nullptr;
		}

		return myShaderRegistry.at(lowName);
	}



	void ShaderRegistry::LoadAllShaders()
	{
		const std::vector<std::filesystem::path> searchPaths =
		{
			FileSystem::GetShadersPath(),
			ProjectManager::GetAssetsDirectory()
		};

		for (const auto& searchPath : searchPaths)
		{
			if (searchPath.empty())
			{
				continue;
			}

			for (const auto& path : std::filesystem::recursive_directory_iterator(searchPath))
			{
				const auto relPath = AssetManager::GetRelativePath(path.path());
				const auto type = AssetManager::GetAssetTypeFromPath(relPath);

				if (type == AssetType::Shader)
				{
					if (!AssetManager::ExistsInRegistry(relPath))
					{
						AssetManager::Get().AddAssetToRegistry(relPath);
					}

					Ref<Shader> shaderAsset = AssetManager::GetAsset<Shader>(relPath);
					Register(shaderAsset->GetName(), shaderAsset);
				}
			}
		}
	}
}
