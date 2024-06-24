#pragma once

#include <VoltRHI/Shader/Shader.h>
#include <VoltRHI/Shader/ShaderCompiler.h>

struct IDxcBlob;

namespace Volt::RHI
{
	class D3D12Shader final : public Shader
	{
		
	public:
		D3D12Shader(const ShaderSpecification& createInfo);
		~D3D12Shader() override;

		IDxcBlob* GetBlob(ShaderStage stage);

		std::vector<ShaderStage> GetStages();

		const bool Reload(bool forceCompile) override;
		std::string_view GetName() const override;
		const std::vector<ShaderSourceEntry>& GetSourceEntries() const override;
		const ShaderResources& GetResources() const override;
		ShaderDataBuffer GetConstantsBuffer() const override;
		const ShaderResourceBinding& GetResourceBindingFromName(std::string_view name) const override;

	protected:
		void* GetHandleImpl() const override;

	private:
		void LoadShaderFromFiles();
		void Release();

		ShaderCompiler::CompilationResultData CompileOrGetBinary(bool forceCompile);

		std::unordered_map<ShaderStage, ShaderSourceInfo> m_shaderSources;

		ShaderSpecification m_specification;
		ShaderResources m_resources;
	};
}
