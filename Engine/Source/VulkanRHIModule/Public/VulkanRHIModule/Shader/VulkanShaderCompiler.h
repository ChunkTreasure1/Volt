#pragma once

#include "VulkanRHIModule/Core.h"

#include <RHIModule/Core/RHICommon.h>
#include <RHIModule/Shader/ShaderCompiler.h>

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
		CompilationResultData TryCompileImpl(const Specification& specification) override;
		void AddMacroImpl(const std::string& macroName) override;
		void RemoveMacroImpl(std::string_view macroName) override;
		void* GetHandleImpl() const override;

	private:
		bool PreprocessSource(const ShaderStage shaderStage, const std::filesystem::path& filepath, std::string& outSource);

		CompilationResultData CompileAll(const Specification& specification);
		CompilationResult CompileSingle(const ShaderStage shaderStage, const std::string& source, const ShaderSourceEntry& sourceEntry, const Specification& specification, CompilationResultData& outData);

		void ReflectAllStages(const Specification& specification, CompilationResultData& inOutData);
		void ReflectStage(ShaderStage stage, const Specification& specification, CompilationResultData& inOutData);

		bool TryAddShaderBinding(const std::string& name, uint32_t set, uint32_t binding, CompilationResultData& outData);

		IDxcCompiler3* m_hlslCompiler = nullptr;
		IDxcUtils* m_hlslUtils = nullptr;
	
		Vector<std::filesystem::path> m_includeDirectories;
		Vector<std::string> m_macros;
		ShaderCompilerFlags m_flags = ShaderCompilerFlags::None;
		std::filesystem::path m_cacheDirectory;

		RefPtr<ShaderCache> m_shaderCache;
	};
}
