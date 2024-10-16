#include "rcpch.h"

#include "RenderCore/Shader/ShaderSubSystem.h"

#include <Volt-Core/Project/ProjectManager.h>

#include <RHIModule/Shader/ShaderCompiler.h>
#include <RHIModule/Shader/ShaderCache.h>

namespace Volt
{
	VT_REGISTER_SUBSYSTEM(ShaderSubSystem, Engine, 0);

	void ShaderSubSystem::Initialize()
	{
		{
			RHI::ShaderCacheCreateInfo info{};
			info.cacheDirectory = "Engine/Shaders/Cache";

			m_shaderCache = RefPtr<RHI::ShaderCache>::Create(info);
		}

		{
			RHI::ShaderCompilerCreateInfo shaderCompilerInfo{};
			shaderCompilerInfo.flags = RHI::ShaderCompilerFlags::WarningsAsErrors;
			shaderCompilerInfo.shaderCache = m_shaderCache;

#ifdef VT_ENABLE_SHADER_RUNTIME_VALIDATION
			shaderCompilerInfo.flags |= RHI::ShaderCompilerFlags::EnableShaderValidator;
#endif

			shaderCompilerInfo.includeDirectories =
			{
				ProjectManager::GetEngineShaderIncludeDirectory(),
				ProjectManager::GetAssetsDirectory()
			};

			m_shaderCompiler = RHI::ShaderCompiler::Create(shaderCompilerInfo);
		}
	}

	void ShaderSubSystem::Shutdown()
	{
		m_shaderCompiler = nullptr;
		m_shaderCache = nullptr;
	}
}
