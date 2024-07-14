#pragma once
#include "VoltRHI/Shader/ShaderCompiler.h"

struct IDxcCompiler3;
struct IDxcUtils;
struct IDxcResult;
struct ID3D12ShaderReflection;

namespace Volt::RHI
{
	struct PreProcessorResult;
	class D3D12Shader;

	class D3D12ShaderCompiler final : public ShaderCompiler
	{
	public:
		D3D12ShaderCompiler(const ShaderCompilerCreateInfo& createInfo);
		~D3D12ShaderCompiler() override;
	
	protected:
		CompilationResultData TryCompileImpl(const Specification& specification) override;
		void AddMacroImpl(const std::string& macroName) override;
		void RemoveMacroImpl(std::string_view macroName) override;
		void* GetHandleImpl() const override;

	private:
		bool PreprocessSource(const ShaderStage shaderStage, const std::filesystem::path& filepath, std::string& outSource);

		CompilationResultData CompileAll(const Specification& specification, std::unordered_map<ShaderStage, ID3D12ShaderReflection*>& outReflectionData);
		CompilationResult CompileSingle(const ShaderStage shaderStage, const std::string& source, const ShaderSourceEntry& sourceEntry, const Specification& specification, CompilationResultData& outData, ID3D12ShaderReflection** outReflectionData);

		void ReflectAllStages(const Specification& specification, CompilationResultData& inOutData, const std::unordered_map<ShaderStage, ID3D12ShaderReflection*>& reflectionData);
		void ReflectStage(ShaderStage stage, const Specification& specification, CompilationResultData& inOutData, ID3D12ShaderReflection* reflectionData);

		bool TryAddShaderBinding(const std::string& name, uint32_t set, uint32_t binding, ShaderRegisterType registerType, CompilationResultData& outData);

		IDxcCompiler3* m_hlslCompiler = nullptr;
		IDxcUtils* m_hlslUtils = nullptr;

		std::vector<std::filesystem::path> m_includeDirectories;
		std::vector<std::string> m_macros;
		ShaderCompilerFlags m_flags = ShaderCompilerFlags::None;
		std::filesystem::path m_cacheDirectory;
	};
}
