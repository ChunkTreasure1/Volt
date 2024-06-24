#include "dxpch.h"
#include "D3D12Shader.h"

#include <VoltRHI/Shader/ShaderCompiler.h>
#include <VoltRHI/Shader/ShaderUtility.h>

#include <dxc/dxcapi.h>

namespace Volt::RHI
{
	D3D12Shader::D3D12Shader(const ShaderSpecification& specification) 
		: m_specification(specification)
	{
		if (m_specification.sourceEntries.empty())
		{
			RHILog::LogTagged(LogSeverity::Error, "[D3D12Shader]", "Trying to create a shader {0} without any sources!", m_specification.name);
			return;
		}

		Reload(specification.forceCompile);
	}

	D3D12Shader::~D3D12Shader()
	{
		Release();
	}

	IDxcBlob* D3D12Shader::GetBlob(ShaderStage stage)
	{
		return nullptr;
	}

	std::vector<ShaderStage> D3D12Shader::GetStages()
	{
		std::vector<ShaderStage> returnStage;

		//for (auto& [stage, blob] : m_blobMap)
		//{
		//	returnStage.emplace_back(stage);
		//}

		return returnStage;
	}

	void* D3D12Shader::GetHandleImpl() const
	{
		return nullptr;
	}

	void D3D12Shader::LoadShaderFromFiles()
	{
		for (const auto& entry : m_specification.sourceEntries)
		{
			const ShaderStage stage = entry.shaderStage;
			std::string source = Utility::ReadStringFromFile(entry.filePath);

			if (source.empty())
			{
				continue;
			}

			if (m_shaderSources.contains(stage))
			{
				RHILog::LogTagged(LogSeverity::Error, "[D3D12Shader]", "Multiple shaders of same stage defined in file {0}!", entry.filePath.string().c_str());
				continue;
			}

			m_shaderSources[stage].source = source;
			m_shaderSources[stage].sourceEntry = entry;
		}
	}

	void D3D12Shader::Release()
	{
	}

	ShaderCompiler::CompilationResultData D3D12Shader::CompileOrGetBinary(bool forceCompile)
	{
		ShaderCompiler::Specification compileSpec{};
		compileSpec.forceCompile = forceCompile;
		compileSpec.shaderSourceInfo = m_shaderSources;

		return ShaderCompiler::TryCompile(compileSpec);
	}

	const bool D3D12Shader::Reload(bool forceCompile)
	{
		m_shaderSources.clear();
		LoadShaderFromFiles();

		const ShaderCompiler::CompilationResultData compilationResult = CompileOrGetBinary(forceCompile);
		if (compilationResult.result != ShaderCompiler::CompilationResult::Success)
		{
			return false;
		}

		Release();

		return true;
	}

	std::string_view D3D12Shader::GetName() const
	{
		return m_specification.name;
	}
	
	const ShaderResources& D3D12Shader::GetResources() const
	{
		return m_resources;
	}

	const std::vector<ShaderSourceEntry>& D3D12Shader::GetSourceEntries() const
	{
		return m_specification.sourceEntries;
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
