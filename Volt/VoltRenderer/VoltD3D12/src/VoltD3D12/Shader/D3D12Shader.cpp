#include "dxpch.h"
#include "D3D12Shader.h"

#include <VoltRHI/Shader/ShaderCompiler.h>
#include <VoltRHI/Shader/ShaderUtility.h>
#include <VoltRHI/Utility/HashUtility.h>


#include <dxsc/dxcapi.h>

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

	uint32_t D3D12Shader::GetDescriptorIndexFromDescriptorHash(const size_t hash)
	{
		return m_bindingToDescriptorRangeInfo.at(hash).descriptorIndex;
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

	inline void InitializeRange(D3D12_DESCRIPTOR_RANGE& range, D3D12_DESCRIPTOR_RANGE_TYPE type, uint32_t registerSpace, uint32_t binding)
	{
		range.RangeType = type;
		range.RegisterSpace = registerSpace;
		range.BaseShaderRegister = binding;
		range.NumDescriptors = 0;
		range.OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;
	}

	inline DescriptorRangeInfo AddDescriptorToRange(vt::map<uint32_t, std::vector<size_t>>& rangesMap, std::vector<D3D12_DESCRIPTOR_RANGE>& ranges, uint32_t space, uint32_t binding, D3D12_DESCRIPTOR_RANGE_TYPE rangeType)
	{
		if (rangesMap.contains(space))
		{
			for (const auto& rangeIndex : rangesMap.at(space))
			{
				auto& range = ranges.at(rangeIndex);
				if (range.RangeType != rangeType)
				{
					continue;
				}

				if (range.BaseShaderRegister > 0 && range.BaseShaderRegister - 1 == binding)
				{
					range.BaseShaderRegister = binding;
					return { rangeIndex, range.NumDescriptors++ };
				}

				if (range.BaseShaderRegister + 1 == binding)
				{
					return { rangeIndex, range.NumDescriptors++ };
				}
			}
		}

		const size_t rangeIndex = ranges.size();
		auto& newRange = ranges.emplace_back();

		InitializeRange(newRange, rangeType, space, binding);
		newRange.NumDescriptors = 1;

		rangesMap[space].emplace_back(rangeIndex);

		return { rangeIndex, 0 };
	}

	inline void SetupDescriptorOffsets(vt::map<uint32_t, std::vector<size_t>>& rangesMap, std::vector<D3D12_DESCRIPTOR_RANGE>& ranges)
	{
		uint32_t descriptorCount = 0;
		for (const auto& [space, rangeIndices] : rangesMap)
		{
			for (const auto& rangeIndex : rangeIndices)
			{
				ranges.at(rangeIndex).OffsetInDescriptorsFromTableStart = descriptorCount;
				descriptorCount += ranges.at(rangeIndex).NumDescriptors;
			}
		}
	}

	inline size_t GetDescriptorBindingHash(uint32_t space, uint32_t binding, D3D12_DESCRIPTOR_RANGE_TYPE descriptorType)
	{
		size_t result = std::hash<uint32_t>()(space);
		result = Utility::HashCombine(result, std::hash<uint32_t>()(binding));
		result = Utility::HashCombine(result, std::hash<uint32_t>()(static_cast<uint32_t>(descriptorType)));

		return result;
	}

	void D3D12Shader::CreateRootSignature()
	{
		std::vector<D3D12_ROOT_PARAMETER> rootParameters{};

		std::vector<D3D12_DESCRIPTOR_RANGE> descriptorRanges;
		std::vector<D3D12_DESCRIPTOR_RANGE> samplerRanges;
		vt::map<uint32_t, std::vector<size_t>> bindingToDescriptorRanges;

		constexpr uint32_t PUSH_CONSTANTS_BINDING = 999;

		// Handle push constants / root constants
		if (m_resources.constantsBuffer.IsValid())
		{
			auto& param = rootParameters.emplace_back();
			param.ParameterType = D3D12_ROOT_PARAMETER_TYPE_32BIT_CONSTANTS;
			param.ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;
			param.Constants.RegisterSpace = 0;
			param.Constants.ShaderRegister = PUSH_CONSTANTS_BINDING;
			param.Constants.Num32BitValues = static_cast<uint32_t>(m_resources.constantsBuffer.GetSize() / sizeof(uint32_t));
		}

		for (const auto& [space, bindings] : m_resources.uniformBuffers)
		{
			for (const auto& [binding, buffer] : bindings)
			{
				auto rangeInfo = AddDescriptorToRange(bindingToDescriptorRanges, descriptorRanges, space, binding, D3D12_DESCRIPTOR_RANGE_TYPE_CBV);
				const size_t descriptorHash = GetDescriptorBindingHash(space, binding, D3D12_DESCRIPTOR_RANGE_TYPE_CBV);
				m_bindingToDescriptorRangeInfo[descriptorHash] = rangeInfo;
			}
		}

		for (const auto& [space, bindings] : m_resources.storageBuffers)
		{
			for (const auto& [binding, buffer] : bindings)
			{
				if (buffer.isWrite)
				{
					auto rangeInfo = AddDescriptorToRange(bindingToDescriptorRanges, descriptorRanges, space, binding, D3D12_DESCRIPTOR_RANGE_TYPE_UAV);
					const size_t descriptorHash = GetDescriptorBindingHash(space, binding, D3D12_DESCRIPTOR_RANGE_TYPE_UAV);
					m_bindingToDescriptorRangeInfo[descriptorHash] = rangeInfo;
				}
				else
				{
					auto rangeInfo = AddDescriptorToRange(bindingToDescriptorRanges, descriptorRanges, space, binding, D3D12_DESCRIPTOR_RANGE_TYPE_SRV);
					const size_t descriptorHash = GetDescriptorBindingHash(space, binding, D3D12_DESCRIPTOR_RANGE_TYPE_SRV);
					m_bindingToDescriptorRangeInfo[descriptorHash] = rangeInfo;
				}
			}
		}

		for (const auto& [space, bindings] : m_resources.storageImages)
		{
			for (const auto& [binding, image] : bindings)
			{
				auto rangeInfo = AddDescriptorToRange(bindingToDescriptorRanges, descriptorRanges, space, binding, D3D12_DESCRIPTOR_RANGE_TYPE_UAV);
				const size_t descriptorHash = GetDescriptorBindingHash(space, binding, D3D12_DESCRIPTOR_RANGE_TYPE_UAV);
				m_bindingToDescriptorRangeInfo[descriptorHash] = rangeInfo;
			}
		}

		for (const auto& [space, bindings] : m_resources.images)
		{
			for (const auto& [binding, image] : bindings)
			{
				auto rangeInfo = AddDescriptorToRange(bindingToDescriptorRanges, descriptorRanges, space, binding, D3D12_DESCRIPTOR_RANGE_TYPE_SRV);
				const size_t descriptorHash = GetDescriptorBindingHash(space, binding, D3D12_DESCRIPTOR_RANGE_TYPE_SRV);
				m_bindingToDescriptorRangeInfo[descriptorHash] = rangeInfo;
			}
		}

		for (const auto& [space, bindings] : m_resources.samplers)
		{
			for (const auto& [binding, image] : bindings)
			{
				auto rangeInfo = AddDescriptorToRange(bindingToDescriptorRanges, samplerRanges, space, binding, D3D12_DESCRIPTOR_RANGE_TYPE_SAMPLER);
				const size_t descriptorHash = GetDescriptorBindingHash(space, binding, D3D12_DESCRIPTOR_RANGE_TYPE_SAMPLER);
				m_bindingToDescriptorRangeInfo[descriptorHash] = rangeInfo;
			}
		}

		SetupDescriptorOffsets(bindingToDescriptorRanges, descriptorRanges);

		for (auto& [hash, rangeInfo] : m_bindingToDescriptorRangeInfo)
		{
			rangeInfo.descriptorIndex += descriptorRanges.at(rangeInfo.rangeIndex).OffsetInDescriptorsFromTableStart;
		}

		if (!descriptorRanges.empty())
		{
			auto& tableParam = rootParameters.emplace_back();
			tableParam.ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
			tableParam.DescriptorTable = { static_cast<uint32_t>(descriptorRanges.size()), descriptorRanges.data() };
			tableParam.ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;
		}

		if (!samplerRanges.empty())
		{
			auto& tableParam = rootParameters.emplace_back();
			tableParam.ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
			tableParam.DescriptorTable = { static_cast<uint32_t>(samplerRanges.size()), samplerRanges.data() };
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

	bool D3D12Shader::HasConstants() const
	{
		return m_resources.constantsBuffer.IsValid();
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
