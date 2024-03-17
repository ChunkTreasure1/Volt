#include "dxpch.h"
#include "D3D12Shader.h"

#include "VoltRHI/Shader/ShaderCompiler.h"
#include <dxc/dxcapi.h>

namespace Volt::RHI
{
	D3D12Shader::D3D12Shader(std::string_view name, const std::vector<std::filesystem::path>& sourceFiles, bool forceCompile) : m_name(name), m_sourceFiles(sourceFiles)
	{
		Reload(forceCompile);
	}

	D3D12Shader::~D3D12Shader()
	{
	}

	IDxcBlob* D3D12Shader::GetBlob(ShaderStage stage)
	{
		return m_blobMap[stage];
	}

	std::vector<ShaderStage> D3D12Shader::GetStages()
	{
		std::vector<ShaderStage> returnStage;

		for (auto& [stage, blob] : m_blobMap)
		{
			returnStage.emplace_back(stage);
		}

		return returnStage;
	}

	void* D3D12Shader::GetHandleImpl() const
	{
		return nullptr;
	}

	const bool D3D12Shader::Reload(bool forceCompile)
	{
		ShaderCompiler::Specification specs = {};
		specs.forceCompile = forceCompile;

		auto result = ShaderCompiler::TryCompile(specs, *this);
		return result == ShaderCompiler::CompilationResult::Success;
	}

	std::string_view D3D12Shader::GetName() const
	{
		return m_name;
	}
	
	const ShaderResources& D3D12Shader::GetResources() const
	{
		return m_resources;
	}

	const std::vector<std::filesystem::path>& D3D12Shader::GetSourceFiles() const
	{
		return m_sourceFiles;
	}

	ShaderDataBuffer D3D12Shader::GetConstantsBuffer() const
	{
		return ShaderDataBuffer();
	}

	const ShaderResourceBinding& D3D12Shader::GetResourceBindingFromName(std::string_view name) const
	{
		static ShaderResourceBinding invalidBinding{};
		return invalidBinding;
	}
}
