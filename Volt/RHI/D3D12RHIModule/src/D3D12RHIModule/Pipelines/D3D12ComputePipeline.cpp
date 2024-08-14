#include "dxpch.h"
#include "D3D12ComputePipeline.h"

#include "D3D12RHIModule/Shader/D3D12Shader.h"

namespace Volt::RHI
{
	D3D12ComputePipeline::D3D12ComputePipeline(RefPtr<Shader> shader, bool useGlobalResources)
		: m_shader(shader), m_useGlobalResouces(useGlobalResources)
	{
		Invalidate();
	}

	D3D12ComputePipeline::~D3D12ComputePipeline()
	{
		Release();
	}

	void D3D12ComputePipeline::Invalidate()
	{
		Release();
		VT_ENSURE(m_shader);

		D3D12Shader& d3d12Shader = m_shader->AsRef<D3D12Shader>();
		const auto& stageData = d3d12Shader.GetShaderStageData().at(ShaderStage::Compute);

		D3D12_COMPUTE_PIPELINE_STATE_DESC pipelineDesc{};
		pipelineDesc.CachedPSO = { nullptr, 0 };
		pipelineDesc.NodeMask = 0;
		pipelineDesc.Flags = D3D12_PIPELINE_STATE_FLAG_NONE;
		pipelineDesc.CS = { stageData.data(), stageData.size() * sizeof(uint32_t) };
		pipelineDesc.pRootSignature = d3d12Shader.GetRootSignature().Get();

		auto d3d12Device = GraphicsContext::GetDevice()->GetHandle<ID3D12Device2*>();
		VT_D3D12_CHECK(d3d12Device->CreateComputePipelineState(&pipelineDesc, VT_D3D12_ID(m_pipeline)));
	}

	RefPtr<Shader> D3D12ComputePipeline::GetShader() const
	{
		return m_shader;
	}

	void* D3D12ComputePipeline::GetHandleImpl() const
	{
		return m_pipeline.Get();
	}

	void D3D12ComputePipeline::Release()
	{
		m_pipeline = nullptr;
	}
}
