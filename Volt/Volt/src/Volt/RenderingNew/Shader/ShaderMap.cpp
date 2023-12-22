#include "vtpch.h"
#include "ShaderMap.h"

#include "Volt/Asset/AssetManager.h"
#include "Volt/Asset/Rendering/ShaderDefinition.h"

#include "Volt/Core/Application.h"

#include "Volt/Project/ProjectManager.h"

#include <VoltRHI/Shader/Shader.h>

namespace Volt
{
	void ShaderMap::Initialize()
	{
		LoadShaders();
	}

	void ShaderMap::Shutdown()
	{
		s_shaderMap.clear();
	}

	void ShaderMap::ReloadAll()
	{
		for (const auto [name, shader] : s_shaderMap)
		{
			shader->Reload(true);
		}
	}

	Ref<RHI::Shader> ShaderMap::Get(const std::string& name)
	{
		if (!s_shaderMap.contains(name))
		{
			return nullptr;
		}

		return s_shaderMap.at(name);
	}

	void ShaderMap::LoadShaders()
	{
		const std::vector<std::filesystem::path> searchPaths =
		{
			ProjectManager::GetEngineDirectory() / "Engine" / "Shaders",
			ProjectManager::GetAssetsDirectory()
		};

		std::vector<std::future<void>> shaderFutures;

		for (const auto& searchPath : searchPaths)
		{
			for (const auto& path : std::filesystem::recursive_directory_iterator(searchPath))
			{
				const auto relPath = AssetManager::GetRelativePath(path.path());
				const auto type = AssetManager::GetAssetTypeFromPath(relPath);
			
				if (type != AssetType::ShaderDefinition)
				{
					continue;
				}

				if (!AssetManager::ExistsInRegistry(relPath))
				{
					AssetManager::Get().AddAssetToRegistry(relPath);
				}

				Ref<ShaderDefinition> shaderDef = AssetManager::GetAsset<ShaderDefinition>(relPath);

				auto& threadPool = Application::GetThreadPool();

				shaderFutures.emplace_back(threadPool.SubmitTask([&, def = shaderDef]() 
				{
					// #TODO: Fix force compile!
					// We need to do this because the formats / input layouts are not created correctly otherwise
					Ref<RHI::Shader> shader = RHI::Shader::Create(def->GetName(), def->GetSourceFiles(), true);
					s_shaderMap[std::string(def->GetName())] = shader;
				}));
			}
		}

		for (const auto& future : shaderFutures)
		{
			future.wait();
		}
	}

	void ShaderMap::RegisterShader(const std::string& name, Ref<RHI::Shader> shader)
	{
		std::scoped_lock lock{ s_registerMutex };
		s_shaderMap[std::string(name)] = shader;
	}
}
