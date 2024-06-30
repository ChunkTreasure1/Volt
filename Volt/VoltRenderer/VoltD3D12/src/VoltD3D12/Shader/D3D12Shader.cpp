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
		m_rootSignature = nullptr;
		m_resources = {};
		m_shaderStageData.clear();
	}

	void D3D12Shader::CopyCompilationResults(const ShaderCompiler::CompilationResultData& compilationResult)
	{
		m_resources.outputFormats = compilationResult.outputFormats;
		m_resources.vertexLayout = compilationResult.vertexLayout;
		m_resources.instanceLayout = compilationResult.instanceLayout;

		m_resources.renderGraphConstantsData = compilationResult.renderGraphConstants;
		m_resources.constantsBuffer = compilationResult.constantsBuffer;
		m_resources.constants = compilationResult.constants;

		m_resources.bindings = compilationResult.bindings;

		m_resources.uniformBuffers = compilationResult.uniformBuffers;
		m_resources.storageBuffers = compilationResult.storageBuffers;
		m_resources.storageImages = compilationResult.storageImages;
		m_resources.images = compilationResult.images;
		m_resources.samplers = compilationResult.samplers;

		m_shaderStageData = compilationResult.shaderData;
	}

	inline void InitializeRange(D3D12_DESCRIPTOR_RANGE& range, D3D12_DESCRIPTOR_RANGE_TYPE type)
	{
		range.RangeType = type;
		range.RegisterSpace = 0;
		range.NumDescriptors = 0;
		range.BaseShaderRegister = 0;
		range.OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;
	}

	void D3D12Shader::CreateRootSignature()
	{
		std::vector<D3D12_ROOT_PARAMETER> rootParameters{};

		D3D12_DESCRIPTOR_RANGE cbvRange{};
		D3D12_DESCRIPTOR_RANGE srvRange{};
		D3D12_DESCRIPTOR_RANGE uavRange{};
		D3D12_DESCRIPTOR_RANGE samplerRange{};

		InitializeRange(cbvRange, D3D12_DESCRIPTOR_RANGE_TYPE_CBV);
		InitializeRange(srvRange, D3D12_DESCRIPTOR_RANGE_TYPE_SRV);
		InitializeRange(uavRange, D3D12_DESCRIPTOR_RANGE_TYPE_UAV);
		InitializeRange(samplerRange, D3D12_DESCRIPTOR_RANGE_TYPE_SAMPLER);

		constexpr uint32_t PUSH_CONSTANTS_BINDING = 999;

		for (const auto& [space, bindings] : m_resources.uniformBuffers)
		{
			for (const auto& [binding, buffer] : bindings)
			{
				if (binding == PUSH_CONSTANTS_BINDING)
				{
					auto& param = rootParameters.emplace_back();
					param.ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
					param.Descriptor.RegisterSpace = space;
					param.Descriptor.ShaderRegister = binding;
					param.ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;
				}
				else
				{
					cbvRange.NumDescriptors++;
					cbvRange.BaseShaderRegister = std::min(binding, cbvRange.BaseShaderRegister);
				}
			}
		}

		for (const auto& [space, bindings] : m_resources.storageBuffers)
		{
			for (const auto& [binding, buffer] : bindings)
			{
				if (buffer.isWrite)
				{
					uavRange.NumDescriptors++;
					uavRange.BaseShaderRegister = std::min(binding, uavRange.BaseShaderRegister);
				}
				else
				{
					srvRange.NumDescriptors++;
					srvRange.BaseShaderRegister = std::min(binding, srvRange.BaseShaderRegister);
				}
			}
		}

		for (const auto& [space, bindings] : m_resources.storageImages)
		{
			for (const auto& [binding, image] : bindings)
			{
				uavRange.NumDescriptors++;
				uavRange.BaseShaderRegister = std::min(binding, srvRange.BaseShaderRegister);
			}
		}

		for (const auto& [space, bindings] : m_resources.images)
		{
			for (const auto& [binding, image] : bindings)
			{
				srvRange.NumDescriptors++;
				srvRange.BaseShaderRegister = std::min(binding, srvRange.BaseShaderRegister);
			}
		}

		for (const auto& [space, bindings] : m_resources.samplers)
		{
			for (const auto& [binding, image] : bindings)
			{
				samplerRange.NumDescriptors++;
				samplerRange.BaseShaderRegister = std::min(binding, samplerRange.BaseShaderRegister);
			}
		}

		std::vector<D3D12_DESCRIPTOR_RANGE> descriptorRanges{};
		descriptorRanges.reserve(3);

		if (cbvRange.NumDescriptors > 0)
		{
			descriptorRanges.emplace_back(cbvRange);
		}

		if (srvRange.NumDescriptors > 0)
		{
			descriptorRanges.emplace_back(srvRange);
		}

		if (uavRange.NumDescriptors > 0)
		{
			descriptorRanges.emplace_back(uavRange);
		}

		if (!descriptorRanges.empty())
		{
			auto& tableParam = rootParameters.emplace_back();
			tableParam.ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
			tableParam.DescriptorTable = { static_cast<uint32_t>(descriptorRanges.size()), descriptorRanges.data() };
			tableParam.ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;
		}

		if (samplerRange.NumDescriptors > 0)
		{
			auto& tableParam = rootParameters.emplace_back();
			tableParam.ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
			tableParam.DescriptorTable = { 1, &samplerRange };
			tableParam.ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;
		}

		D3D12_ROOT_SIGNATURE_FLAGS flags =
			D3D12_ROOT_SIGNATURE_FLAG_CBV_SRV_UAV_HEAP_DIRECTLY_INDEXED |
			D3D12_ROOT_SIGNATURE_FLAG_SAMPLER_HEAP_DIRECTLY_INDEXED;

		if (!m_shaderStageData.contains(ShaderStage::Compute))
		{
			flags |= D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;
		}

		D3D12_ROOT_SIGNATURE_DESC rootSignatureDesc{};
		rootSignatureDesc.NumParameters = static_cast<uint32_t>(rootParameters.size());
		rootSignatureDesc.pParameters = rootParameters.data();
		rootSignatureDesc.NumStaticSamplers = 0;
		rootSignatureDesc.pStaticSamplers = nullptr;
		rootSignatureDesc.Flags = flags;

		ComPtr<ID3DBlob> signature = nullptr;
		ComPtr<ID3DBlob> errorBlob = nullptr;
		VT_D3D12_CHECK(D3D12SerializeRootSignature(&rootSignatureDesc, D3D_ROOT_SIGNATURE_VERSION_1_0, &signature, &errorBlob));

		auto d3d12Device = GraphicsContext::GetDevice()->GetHandle<ID3D12Device2*>();
		VT_D3D12_CHECK(d3d12Device->CreateRootSignature(0, signature->GetBufferPointer(), signature->GetBufferSize(), VT_D3D12_ID(m_rootSignature)))
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
		CopyCompilationResults(compilationResult);
		CreateRootSignature();

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
		return m_resources.constantsBuffer;
	}

	const ShaderResourceBinding& D3D12Shader::GetResourceBindingFromName(std::string_view name) const
	{
		static ShaderResourceBinding invalidBinding{};
		std::string nameStr = std::string(name);

		if (!m_resources.bindings.contains(nameStr))
		{
			return invalidBinding;
		}

		return m_resources.bindings.at(nameStr);
	}
}
