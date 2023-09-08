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
		m_shaderMap.clear();
	}

	Ref<RHI::Shader> ShaderMap::Get(const std::string& name)
	{
		if (!m_shaderMap.contains(name))
		{
			return nullptr;
		}

		return m_shaderMap.at(name);
	}

	void ShaderMap::LoadShaders()
	{
		const std::vector<std::filesystem::path> searchPaths =
		{
			ProjectManager::GetEngineDirectory() / "Engine" / "Shaders",
			ProjectManager::GetAssetsDirectory()
		};

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

				threadPool.SubmitTask([&, def = shaderDef]() 
				{
					Ref<RHI::Shader> shader = RHI::Shader::Create(def->GetName(), def->GetSourceFiles());
					m_shaderMap[std::string(def->GetName())] = shader;
				});
			}
		}
	}

	void ShaderMap::RegisterShader(const std::string& name, Ref<RHI::Shader> shader)
	{
		std::scoped_lock lock{ m_registerMutex };
		m_shaderMap[std::string(name)] = shader;
	}
}
