#pragma once

#include "RHIModule/Shader/ShaderCompiler.h"

namespace Volt::RHI
{
	struct ShaderCacheCreateInfo
	{
		std::filesystem::path cacheDirectory;
	};

	struct CachedShaderResult
	{
		ShaderCompiler::CompilationResultData data;
		uint64_t timeSinceLastCompile = 0;
	};

	class VTRHI_API ShaderCache
	{
	public:
		ShaderCache(const ShaderCacheCreateInfo& cacheInfo);
		~ShaderCache();

		static CachedShaderResult TryGetCachedShader(const ShaderCompiler::Specification& shaderSpecification);
		static void CacheShader(const ShaderCompiler::Specification& shaderSpec, const ShaderCompiler::CompilationResultData& compilationResult);

	private:
		std::filesystem::path GetCachedFilePath(const ShaderCompiler::Specification& shaderSpec) const;

		inline static ShaderCache* s_instance = nullptr;
		ShaderCacheCreateInfo m_info;
	};
}
