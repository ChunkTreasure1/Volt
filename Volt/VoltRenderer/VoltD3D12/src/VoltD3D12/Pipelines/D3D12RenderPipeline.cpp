#include "dxpch.h"
#include "D3D12RenderPipeline.h"

#include "VoltD3D12/Shader/D3D12Shader.h"
#include "VoltRHI/Graphics/GraphicsDevice.h"

#include "VoltRHI/Images/ImageUtility.h"

#include <dxc/dxcapi.h>

namespace Volt::RHI
{
	D3D12RenderPipeline::D3D12RenderPipeline(const RenderPipelineCreateInfo& createInfo)
	{
		m_topology = createInfo.topology;
		Create(createInfo);
	}

	D3D12RenderPipeline::~D3D12RenderPipeline()
	{
	}

	void* D3D12RenderPipeline::GetHandleImpl() const
	{
		return nullptr;
	}

	void D3D12RenderPipeline::Invalidate()
	{
	}

	void D3D12RenderPipeline::Create(const RenderPipelineCreateInfo& createInfo)
	{
		auto shader = createInfo.shader->As<D3D12Shader>();

		auto d3d12Device = GraphicsContext::GetDevice()->GetHandle<ID3D12Device2*>();

		std::vector<D3D12_ROOT_PARAMETER> rootParams = {};

		CD3DX12_ROOT_SIGNATURE_DESC rootSignatureDesc = {};
		rootSignatureDesc.Init(static_cast<uint32_t>(rootParams.size()),
			rootParams.data(),
			0,
			nullptr,
			D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);

		ID3DBlob* signature = nullptr;
		ID3DBlob* errorBlob = nullptr;
		VT_D3D12_CHECK(D3D12SerializeRootSignature(&rootSignatureDesc, D3D_ROOT_SIGNATURE_VERSION_1_0, &signature, &errorBlob));
		VT_D3D12_CHECK(d3d12Device->CreateRootSignature(0, signature->GetBufferPointer(), signature->GetBufferSize(),VT_D3D12_ID(m_rootSignature)));

		D3D12_GRAPHICS_PIPELINE_STATE_DESC pipelineDesc{};

		auto stages = shader->GetStages();

		for (auto& stage : stages)
		{
			auto blob = shader->GetBlob(stage);
			D3D12_SHADER_BYTECODE byteCode = { blob->GetBufferPointer(), blob->GetBufferSize() };
			switch (stage)
			{
				case ShaderStage::Vertex:
					pipelineDesc.VS = byteCode;
					break;
				case ShaderStage::Pixel:
					pipelineDesc.PS = byteCode;
					break;
				case ShaderStage::Geometry:
					pipelineDesc.GS = byteCode;
					break;
			}
		}

		pipelineDesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
		pipelineDesc.BlendState.AlphaToCoverageEnable = TRUE;

		pipelineDesc.pRootSignature = m_rootSignature;

		pipelineDesc.NodeMask = 0;

		pipelineDesc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);

		switch (createInfo.cullMode)
		{
			case CullMode::Back: pipelineDesc.RasterizerState.CullMode = D3D12_CULL_MODE_BACK; break;
			case CullMode::Front: pipelineDesc.RasterizerState.CullMode = D3D12_CULL_MODE_FRONT; break;
			case CullMode::FrontAndBack: pipelineDesc.RasterizerState.CullMode = D3D12_CULL_MODE_NONE; break;
			case CullMode::None: pipelineDesc.RasterizerState.CullMode = D3D12_CULL_MODE_NONE; break;
		}

		switch (createInfo.fillMode)
		{
			case FillMode::Solid: pipelineDesc.RasterizerState.FillMode = D3D12_FILL_MODE_SOLID; break;
			case FillMode::Wireframe: pipelineDesc.RasterizerState.FillMode = D3D12_FILL_MODE_WIREFRAME; break;
		}

		size_t i = 0;
		for ( auto& format : shader->GetResources().outputFormats)
		{
			if (Utility::IsDepthFormat(format))
			{
				pipelineDesc.DSVFormat = ConvertFormatToD3D12Format(format);
				continue;
			}

			pipelineDesc.RTVFormats[i] = ConvertFormatToD3D12Format(format);
			i++;
		}

		pipelineDesc.NumRenderTargets = static_cast<UINT>(i);

		pipelineDesc.SampleMask = 0xffffffff;
		pipelineDesc.SampleDesc.Count = 1;

		switch (createInfo.topology)
		{
			case Topology::LineList: pipelineDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_LINE; break;
			case Topology::PatchList: pipelineDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_PATCH; break;
			case Topology::TriangleList: pipelineDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE; break;
			case Topology::PointList: pipelineDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_POINT; break;
			case Topology::TriangleStrip: pipelineDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE; break;
			default:
				break;
		}

		VT_D3D12_CHECK(d3d12Device->CreateGraphicsPipelineState(&pipelineDesc, VT_D3D12_ID(m_pipelineStateObject)));
	}

	
}
