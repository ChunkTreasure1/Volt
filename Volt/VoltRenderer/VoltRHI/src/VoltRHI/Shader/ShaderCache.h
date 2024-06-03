#pragma once

#include "VoltRHI/Shader/ShaderCompiler.h"

namespace Volt::RHI
{
	struct ShaderCacheCreateInfo
	{
		std::filesystem::path cacheDirectory;
	};

	class VTRHI_API ShaderCache
	{
	public:
		ShaderCache(const ShaderCacheCreateInfo& cacheInfo);
		~ShaderCache();

		static ShaderCompiler::CompilationResultData TryGetCachedShader(const ShaderCompiler::Specification& shaderSpecification);
		static void CacheShader(const ShaderCompiler::Specification& shaderSpec, const ShaderCompiler::CompilationResultData& compilationResult);

	private:
		std::filesystem::path GetCachedFilePath(const ShaderCompiler::Specification& shaderSpec) const;

		inline static ShaderCache* s_instance = nullptr;
		ShaderCacheCreateInfo m_info;
	};
}
