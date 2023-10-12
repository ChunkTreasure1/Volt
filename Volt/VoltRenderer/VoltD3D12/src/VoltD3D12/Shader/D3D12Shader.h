#pragma once
#include "VoltRHI/Shader/Shader.h"

struct IDxcBlob;

namespace Volt::RHI
{
	class D3D12Shader final : public Shader
	{
		
	public:
		D3D12Shader(std::string_view name, const std::vector<std::filesystem::path>& sourceFiles, bool forceCompile);
		~D3D12Shader() override;

		IDxcBlob* GetBlob(ShaderStage stage);

		std::vector<ShaderStage> GetStages();

		void* GetHandleImpl() const override;
		const bool Reload(bool forceCompile) override;
		std::string_view GetName() const override;
		const ShaderResources& GetResources() const override;
		const std::vector<std::filesystem::path>& GetSourceFiles() const override;
		ShaderDataBuffer GetConstantsBuffer() const override;
		const ShaderResourceBinding& GetResourceBindingFromName(std::string_view name) const override;

	private:
		friend class D3D12ShaderCompiler;
		std::string m_name;

		std::unordered_map<ShaderStage, IDxcBlob*> m_blobMap;

		std::vector<std::filesystem::path> m_sourceFiles;
		ShaderResources m_resources;
	};
}
