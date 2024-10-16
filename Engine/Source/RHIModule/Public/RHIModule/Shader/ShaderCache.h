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

	class VTRHI_API ShaderCache : public RefCounted
	{
	public:
		ShaderCache(const ShaderCacheCreateInfo& cacheInfo);
		~ShaderCache();

		CachedShaderResult TryGetCachedShader(const ShaderCompiler::Specification& shaderSpecification);
		void CacheShader(const ShaderCompiler::Specification& shaderSpec, const ShaderCompiler::CompilationResultData& compilationResult);

	private:
		std::filesystem::path GetCachedFilePath(const ShaderCompiler::Specification& shaderSpec) const;
		ShaderCacheCreateInfo m_info;
	};
}
