#pragma once

#include "Volt/Rendering/Shader/Shader.h"

#include <vulkan/vulkan.h>

namespace shaderc
{
	class Compiler;
	class CompileOptions;
}

namespace Volt
{
	class ShaderCompiler
	{
	public:
		struct ShaderStageData
		{
			Shader::Language language;
			std::filesystem::path filepath;
			std::string source;
		};

		using ShaderSourceMap = std::unordered_map<VkShaderStageFlagBits, ShaderStageData>;
		using ShaderDataMap = std::unordered_map<VkShaderStageFlagBits, std::vector<uint32_t>>;

		static bool TryCompile(ShaderDataMap& outShaderData, const std::vector<std::filesystem::path>& shaderFiles);

	private:		
		static void LoadShaderFromFiles(ShaderSourceMap& shaderSources, const std::vector<std::filesystem::path>& shaderFiles);
		static const std::vector<Shader::Language> GetLanguages(const std::vector<std::filesystem::path>& paths);

		static bool CompileAll(const ShaderSourceMap& shaderSources, const std::vector<std::filesystem::path>& shaderFiles, ShaderDataMap& outShaderData);
		static bool CompileGLSL(const VkShaderStageFlagBits stage, const std::string& source, const std::filesystem::path filepath, std::vector<uint32_t>& outShaderData);
		static bool CompileHLSL(const VkShaderStageFlagBits stage, const std::string& source, const std::filesystem::path filepath, std::vector<uint32_t>& outShaderData);

		static bool PreprocessGLSL(const VkShaderStageFlagBits stage, const std::filesystem::path& filepath, std::string& outSource, shaderc::Compiler& compiler, const shaderc::CompileOptions& compileOptions);
		static bool PreprocessHLSL(const VkShaderStageFlagBits stage, const std::filesystem::path& filepath, std::string& outSource);
	};
}
