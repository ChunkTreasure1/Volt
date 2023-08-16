#pragma once
#include "VoltRHI/Shader/ShaderCompiler.h"

struct IDxcCompiler3;
struct IDxcUtils;
struct IDxcResult;


namespace Volt::RHI
{
	struct PreProcessorResult;
	class D3D12Shader;

	class D3D12ShaderCompiler final : public ShaderCompiler
	{
	public:
		D3D12ShaderCompiler(const ShaderCompilerCreateInfo& createInfo);
		~D3D12ShaderCompiler() override;
		// Inherited via ShaderCompiler
	protected:
		void* GetHandleImpl() override;
		CompilationResult TryCompileImpl(const Specification& specification, Shader& shader) override;
		void AddMacroImpl(const std::string& macroName) override;
		void RemoveMacroImpl(std::string_view macroName) override;

	private:
		IDxcCompiler3* m_hlslCompiler = nullptr;
		IDxcUtils* m_hlslUtils = nullptr;


		ShaderCompiler::CompilationResult CompileStage(const std::filesystem::path& path, const Specification& specification, D3D12Shader& shader);
		ShaderCompilerCreateInfo m_info;
	};
}
