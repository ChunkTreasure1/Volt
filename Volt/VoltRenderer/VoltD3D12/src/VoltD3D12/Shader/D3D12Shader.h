#pragma once

#include "VoltD3D12/Common/ComPtr.h"
#include "VoltD3D12/Descriptors/DescriptorCommon.h"

#include <VoltRHI/Shader/Shader.h>
#include <VoltRHI/Shader/ShaderCompiler.h>

#include <CoreUtilities/Containers/Map.h>

struct ID3D12RootSignature;

namespace Volt::RHI
{
	struct DescriptorRangeInfo
	{
		size_t rangeIndex;
		uint32_t descriptorIndex;
	};

	class D3D12Shader final : public Shader
	{
	public:
		D3D12Shader(const ShaderSpecification& createInfo);
		~D3D12Shader() override;

		const bool Reload(bool forceCompile) override;
		std::string_view GetName() const override;
		const Vector<ShaderSourceEntry>& GetSourceEntries() const override;
		const ShaderResources& GetResources() const override;
		ShaderDataBuffer GetConstantsBuffer() const override;
		VT_NODISCARD bool HasConstants() const override;
		const ShaderResourceBinding& GetResourceBindingFromName(std::string_view name) const override;

		VT_NODISCARD VT_INLINE const std::unordered_map<ShaderStage, Vector<uint32_t>>& GetShaderStageData() const { return m_shaderStageData; }
		VT_NODISCARD VT_INLINE ComPtr<ID3D12RootSignature> GetRootSignature() const { return m_rootSignature; }

		VT_NODISCARD uint32_t GetDescriptorIndexFromDescriptorHash(const size_t hash);
		VT_NODISCARD VT_INLINE uint32_t GetPushConstantsRootParameterIndex() const { return m_pushConstantsRootParamIndex; }
		VT_NODISCARD VT_INLINE uint32_t GetRenderGraphConstantsRootParameterIndex() const { return m_renderGraphConstantsRootParamIndex; }
		VT_NODISCARD VT_INLINE uint32_t GetDescriptorTableRootParameterIndex() const { return m_descriptorTableRootParamIndex; }

	protected:
		void* GetHandleImpl() const override;

	private:
		void LoadShaderFromFiles();
		void Release();

		void CopyCompilationResults(const ShaderCompiler::CompilationResultData& compilationResult);
		void CreateRootSignature();

		ShaderCompiler::CompilationResultData CompileOrGetBinary(bool forceCompile);

		std::unordered_map<ShaderStage, Vector<uint32_t>> m_shaderStageData;
		std::unordered_map<ShaderStage, ShaderSourceInfo> m_shaderSources;

		vt::map<size_t, DescriptorRangeInfo> m_bindingToDescriptorRangeInfo; // Hash is space + binding + type

		ComPtr<ID3D12RootSignature> m_rootSignature;

		ShaderSpecification m_specification;
		ShaderResources m_resources;

		uint32_t m_pushConstantsRootParamIndex = ~0u;
		uint32_t m_renderGraphConstantsRootParamIndex = ~0u;
		uint32_t m_descriptorTableRootParamIndex = ~0u;
	};
}
