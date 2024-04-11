#pragma once

#include "VoltVulkan/Core.h"

#include <VoltRHI/Core/RHICommon.h>
#include <VoltRHI/Shader/ShaderCompiler.h>

struct IDxcCompiler3;
struct IDxcUtils;

namespace Volt::RHI
{
	class VulkanShaderCompiler final : public ShaderCompiler
	{
	public:
		VulkanShaderCompiler(const ShaderCompilerCreateInfo& createInfo);
		~VulkanShaderCompiler() override;

	protected:
		CompilationResult TryCompileImpl(const Specification& specification, Shader& shader) override;
		void AddMacroImpl(const std::string& macroName) override;
		void RemoveMacroImpl(std::string_view macroName) override;
		void* GetHandleImpl() const override;

	private:
		struct ShaderStageData
		{
			std::filesystem::path filepath;
			std::string source;
		};

		using ShaderSourceMap = std::unordered_map<ShaderStage, ShaderStageData>;

		CompilationResult CompileAll(const Specification& specification, Shader& shader);
		CompilationResult CompileSingle(const ShaderStage shaderStage, const std::string& source, const std::filesystem::path& filepath, const Specification& specification, Shader& shader);

		bool PreprocessSource(const ShaderStage shaderStage, const std::filesystem::path& filepath, std::string& outSource);

		IDxcCompiler3* m_hlslCompiler = nullptr;
		IDxcUtils* m_hlslUtils = nullptr;
	
		std::vector<std::filesystem::path> m_includeDirectories;
		std::vector<std::string> m_macros;

		ShaderCompilerFlags m_flags = ShaderCompilerFlags::None;
		std::filesystem::path m_cacheDirectory;
	};

	VTVK_API Ref<ShaderCompiler> CreateVulkanShaderCompiler(const ShaderCompilerCreateInfo& createInfo);
}
