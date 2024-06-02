#pragma once

#include "VoltRHI/Shader/ShaderCompiler.h"

namespace Volt::RHI
{
	class ShaderCache
	{
	public:
		static ShaderCompiler::CompilationResultData TryGetCachedShader(const ShaderCompiler::Specification& shaderSpecification);
		static void CacheShader(const ShaderCompiler::CompilationResultData& compilationResult);

	private:
		ShaderCache() = delete;
	};
}
