#pragma once

#include "VoltD3D12/Common/ComPtr.h"

#include <VoltRHI/Shader/Shader.h>
#include <VoltRHI/Shader/ShaderCompiler.h>

struct ID3D12RootSignature;

namespace Volt::RHI
{
	class D3D12Shader final : public Shader
	{
	public:
		D3D12Shader(const ShaderSpecification& createInfo);
		~D3D12Shader() override;

		const bool Reload(bool forceCompile) override;
		std::string_view GetName() const override;
		const std::vector<ShaderSourceEntry>& GetSourceEntries() const override;
		const ShaderResources& GetResources() const override;
		ShaderDataBuffer GetConstantsBuffer() const override;
		const ShaderResourceBinding& GetResourceBindingFromName(std::string_view name) const override;

		VT_NODISCARD VT_INLINE const std::unordered_map<ShaderStage, std::vector<uint32_t>>& GetShaderStageData() const { return m_shaderStageData; }
		VT_NODISCARD VT_INLINE ComPtr<ID3D12RootSignature> GetRootSignature() const { return m_rootSignature; }

	protected:
		void* GetHandleImpl() const override;

	private:
		void LoadShaderFromFiles();
		void Release();

		void CopyCompilationResults(const ShaderCompiler::CompilationResultData& compilationResult);
		void CreateRootSignature();

		ShaderCompiler::CompilationResultData CompileOrGetBinary(bool forceCompile);

		std::unordered_map<ShaderStage, std::vector<uint32_t>> m_shaderStageData;
		std::unordered_map<ShaderStage, ShaderSourceInfo> m_shaderSources;

		ComPtr<ID3D12RootSignature> m_rootSignature;

		ShaderSpecification m_specification;
		ShaderResources m_resources;
	};
}
