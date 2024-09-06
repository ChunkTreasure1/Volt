#include "dxpch.h"
#include "D3D12RHIModule/Pipelines/D3D12RenderPipeline.h"

#include "D3D12RHIModule/Shader/D3D12Shader.h"
#include "D3D12RHIModule/Common/D3D12Helpers.h"

#include <RHIModule/Images/ImageUtility.h>

#include <dxsc/dxcapi.h>

namespace Volt::RHI
{
	namespace Utility
	{
		static Vector<D3D12_INPUT_ELEMENT_DESC> CreateVertexLayout(const BufferLayout& vertexLayout, const BufferLayout& instanceLayout)
		{
			VT_ASSERT(!vertexLayout.GetElements().empty());
			
			Vector<D3D12_INPUT_ELEMENT_DESC> result{};
			result.reserve(vertexLayout.GetElements().size() + instanceLayout.GetElements().size());

			uint32_t semanticIndex = 0;
			for (const auto& element : vertexLayout.GetElements())
			{
				auto& d3d12Element = result.emplace_back();
				d3d12Element.SemanticName = element.name.c_str();
				d3d12Element.SemanticIndex = semanticIndex;
				d3d12Element.Format = VoltToD3D12ElementFormat(element.type);
				d3d12Element.InputSlot = 0;
				d3d12Element.AlignedByteOffset = D3D12_APPEND_ALIGNED_ELEMENT;
				d3d12Element.InputSlotClass = D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA;
				d3d12Element.InstanceDataStepRate = 0;

				semanticIndex++;
			}

			if (instanceLayout.IsValid())
			{
				for (const auto& element : instanceLayout.GetElements())
				{
					auto& d3d12Element = result.emplace_back();
					d3d12Element.SemanticName = element.name.c_str();
					d3d12Element.SemanticIndex = semanticIndex;
					d3d12Element.Format = VoltToD3D12ElementFormat(element.type);
					d3d12Element.InputSlot = 0;
					d3d12Element.AlignedByteOffset = D3D12_APPEND_ALIGNED_ELEMENT;
					d3d12Element.InputSlotClass = D3D12_INPUT_CLASSIFICATION_PER_INSTANCE_DATA;
					d3d12Element.InstanceDataStepRate = 1;

					semanticIndex++;
				}
			}

			return result;
		}
	}

	D3D12RenderPipeline::D3D12RenderPipeline(const RenderPipelineCreateInfo& createInfo)
		: m_createInfo(createInfo)
	{
		Invalidate();
	}

	D3D12RenderPipeline::~D3D12RenderPipeline()
	{
		Release();
	}

	void* D3D12RenderPipeline::GetHandleImpl() const
	{
		return m_pipeline.Get();
	}

	void D3D12RenderPipeline::Release()
	{
		m_pipeline = nullptr;
	}

	void D3D12RenderPipeline::Invalidate()
	{
		Release();

		if (m_createInfo.enablePrimitiveRestart)
		{
			VT_ENSURE(m_createInfo.topology != Topology::TriangleList && m_createInfo.topology != Topology::LineList && m_createInfo.topology != Topology::PatchList && m_createInfo.topology != Topology::PointList);
		}

		const auto& shaderResources = m_createInfo.shader->GetResources();
		D3D12Shader& d3d12Shader = m_createInfo.shader->AsRef<D3D12Shader>();

		D3D12_RASTERIZER_DESC rasterizerDesc{};
		rasterizerDesc.FillMode = Utility::VoltToD3D12Fill(m_createInfo.fillMode);              
		rasterizerDesc.CullMode = Utility::VoltToD3D12Cull(m_createInfo.cullMode);
		rasterizerDesc.FrontCounterClockwise = FALSE;
		rasterizerDesc.DepthBias = D3D12_DEFAULT_DEPTH_BIAS;
		rasterizerDesc.DepthBiasClamp = D3D12_DEFAULT_DEPTH_BIAS_CLAMP;
		rasterizerDesc.SlopeScaledDepthBias = D3D12_DEFAULT_SLOPE_SCALED_DEPTH_BIAS;
		rasterizerDesc.DepthClipEnable = TRUE;
		rasterizerDesc.MultisampleEnable = FALSE;
		rasterizerDesc.AntialiasedLineEnable = TRUE;
		rasterizerDesc.ForcedSampleCount = 0;
		rasterizerDesc.ConservativeRaster = D3D12_CONSERVATIVE_RASTERIZATION_MODE_OFF;

		D3D12_BLEND_DESC blendDesc{};
		blendDesc.AlphaToCoverageEnable = FALSE;
		blendDesc.IndependentBlendEnable = FALSE;

		for (uint32_t i = 0; const auto& outputFormat : shaderResources.outputFormats)
		{
			if (Utility::IsDepthFormat(outputFormat) || Utility::IsStencilFormat(outputFormat))
			{
				continue;
			}

			blendDesc.RenderTarget[i].BlendEnable = FALSE;
			blendDesc.RenderTarget[i].RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;
			i++;
		}

		D3D12_DEPTH_STENCIL_DESC depthStencilDesc{};
		depthStencilDesc.DepthFunc = Utility::VoltToD3D12CompareOp(m_createInfo.depthCompareOperator);
		depthStencilDesc.StencilEnable = FALSE;
		depthStencilDesc.StencilReadMask = D3D12_DEFAULT_STENCIL_READ_MASK;
		depthStencilDesc.StencilWriteMask = D3D12_DEFAULT_STENCIL_WRITE_MASK;

		const D3D12_DEPTH_STENCILOP_DESC defaultStencilOp = { D3D12_STENCIL_OP_KEEP, D3D12_STENCIL_OP_KEEP, D3D12_STENCIL_OP_KEEP, D3D12_COMPARISON_FUNC_ALWAYS };
		depthStencilDesc.FrontFace = defaultStencilOp;
		depthStencilDesc.BackFace = defaultStencilOp;

		switch (m_createInfo.depthMode)
		{
			case DepthMode::None:
			{
				depthStencilDesc.DepthEnable = FALSE;
				depthStencilDesc.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ZERO;
				break;
			}

			case DepthMode::Read:
			{
				depthStencilDesc.DepthEnable = TRUE;
				depthStencilDesc.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ZERO;
				break;
			}

			case DepthMode::Write:
			{
				depthStencilDesc.DepthEnable = FALSE;
				depthStencilDesc.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ALL;
				break;
			}

			case DepthMode::ReadWrite:
			{
				depthStencilDesc.DepthEnable = TRUE;
				depthStencilDesc.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ALL;
				break;
			}
		}

		Vector<D3D12_INPUT_ELEMENT_DESC> inputLayout;

		if (shaderResources.vertexLayout.IsValid())
		{
			inputLayout = Utility::CreateVertexLayout(shaderResources.vertexLayout, shaderResources.instanceLayout);
		}

		D3D12_GRAPHICS_PIPELINE_STATE_DESC pipelineStateDesc{};
		memset(&pipelineStateDesc, 0, sizeof(D3D12_GRAPHICS_PIPELINE_STATE_DESC));
		pipelineStateDesc.InputLayout = { inputLayout.data(), static_cast<uint32_t>(inputLayout.size()) };
		pipelineStateDesc.pRootSignature = d3d12Shader.GetRootSignature().Get();
		pipelineStateDesc.RasterizerState = rasterizerDesc;

		for (const auto& [stage, data] : d3d12Shader.GetShaderStageData())
		{
			if (stage == ShaderStage::Vertex)
			{
				pipelineStateDesc.VS = { data.data(), data.size() * sizeof(uint32_t) };
			}
			else if (stage == ShaderStage::Pixel)
			{
				pipelineStateDesc.PS = { data.data(), data.size() * sizeof(uint32_t) };
			}
			else if (stage == ShaderStage::Hull)
			{
				pipelineStateDesc.HS = { data.data(), data.size() * sizeof(uint32_t) };
			}
			else if (stage == ShaderStage::Domain)
			{
				pipelineStateDesc.DS = { data.data(), data.size() * sizeof(uint32_t) };
			}
			else if (stage == ShaderStage::Geometry)
			{
				pipelineStateDesc.GS = { data.data(), data.size() * sizeof(uint32_t) };
			}
		}

		pipelineStateDesc.BlendState = blendDesc;
		pipelineStateDesc.DepthStencilState = depthStencilDesc;
		pipelineStateDesc.SampleMask = UINT_MAX;
		pipelineStateDesc.PrimitiveTopologyType = Utility::VoltToD3D12Topology(m_createInfo.topology);
		pipelineStateDesc.SampleDesc.Count = 1;
		pipelineStateDesc.SampleDesc.Quality = 0;

		uint32_t i = 0;
		for (const auto& outputFormat : shaderResources.outputFormats)
		{
			if (Utility::IsDepthFormat(outputFormat) || Utility::IsStencilFormat(outputFormat))
			{
				const auto d3d12Format = ConvertFormatToD3D12Format(outputFormat);

				if (Utility::IsFormatTypeless(d3d12Format))
				{
					pipelineStateDesc.DSVFormat = Utility::GetDSVFormatFromTypeless(d3d12Format);
				}
				else
				{
					pipelineStateDesc.DSVFormat = d3d12Format;
				}
			}
			else
			{
				pipelineStateDesc.RTVFormats[i] = ConvertFormatToD3D12Format(outputFormat);
				i++;
			}
		}

		pipelineStateDesc.NumRenderTargets = i;

		auto d3d12Device = GraphicsContext::GetDevice()->GetHandle<ID3D12Device2*>();
		VT_D3D12_CHECK(d3d12Device->CreateGraphicsPipelineState(&pipelineStateDesc, VT_D3D12_ID(m_pipeline)));
	}

	RefPtr<Shader> D3D12RenderPipeline::GetShader() const
	{
		return m_createInfo.shader;
	}
}
