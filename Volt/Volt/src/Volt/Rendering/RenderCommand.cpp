#include "vtpch.h"
#include "RenderCommand.h"

#include "Volt/Core/Graphics/GraphicsContext.h"

#include "Volt/Rendering/SamplerState.h"
#include "Volt/Rendering/Buffer/ConstantBuffer.h"

#include "Volt/Utility/DirectXUtils.h"
#include "Volt/Utility/StringUtility.h"

#include <d3d11.h>
#include <d3d11_1.h>
#include <wrl.h>

using namespace Microsoft::WRL;

namespace Volt
{
	struct RenderContext
	{
		Topology topology = Topology::None;
		CullState cullState = CullState::CullBack;
		DepthState depthState = DepthState::ReadWrite;

		Context context = Context::Immidiate;
	};

	struct RenderCommandData
	{
		std::vector<ComPtr<ID3D11DepthStencilState>> depthStates;
		std::vector<ComPtr<ID3D11RasterizerState>> cullStates;

		std::vector<ComPtr<ID3D11DeviceContext>> deviceContexts;
		std::vector<ComPtr<ID3DUserDefinedAnnotation>> annotations;
	};

	inline static Scope<RenderContext> s_renderContext;
	inline static Scope<RenderCommandData> s_renderCommandData;

	inline static ComPtr<ID3D11DeviceContext> GetCurrentContext()
	{
		return s_renderCommandData->deviceContexts.at((uint32_t)s_renderContext->context);
	}

	inline static ComPtr<ID3DUserDefinedAnnotation> GetCurrentAnnotations()
	{
		return s_renderCommandData->annotations.at((uint32_t)s_renderContext->context);
	}

	void RenderCommand::Initialize()
	{
		s_renderCommandData = CreateScope<RenderCommandData>();
		s_renderContext = CreateScope<RenderContext>();

		s_renderCommandData->deviceContexts.emplace_back(GraphicsContext::GetImmediateContext());
		s_renderCommandData->deviceContexts.emplace_back(GraphicsContext::GetDeferredContext());

		s_renderCommandData->annotations.emplace_back(GraphicsContext::GetImmediateAnnotations());
		s_renderCommandData->annotations.emplace_back(GraphicsContext::GetDeferredAnnotations());

		// Create Depth states
		{
			auto device = GraphicsContext::GetDevice();

			{
				D3D11_DEPTH_STENCIL_DESC depthDesc = {};
				depthDesc.DepthEnable = true;
				depthDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
				depthDesc.DepthFunc = D3D11_COMPARISON_LESS;

				device->CreateDepthStencilState(&depthDesc, s_renderCommandData->depthStates.emplace_back().GetAddressOf());
			}

			{
				D3D11_DEPTH_STENCIL_DESC depthDesc = {};
				depthDesc.DepthEnable = true;
				depthDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ZERO;
				depthDesc.DepthFunc = D3D11_COMPARISON_LESS;

				device->CreateDepthStencilState(&depthDesc, s_renderCommandData->depthStates.emplace_back().GetAddressOf());
			}

			{
				D3D11_DEPTH_STENCIL_DESC depthDesc = {};
				depthDesc.DepthEnable = false;
				depthDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ZERO;
				depthDesc.DepthFunc = D3D11_COMPARISON_LESS;

				device->CreateDepthStencilState(&depthDesc, s_renderCommandData->depthStates.emplace_back().GetAddressOf());
			}
		}

		// Create Cull states
		{
			auto device = GraphicsContext::GetDevice();

			// Back
			{
				D3D11_RASTERIZER_DESC rasterizerDesc = {};
				rasterizerDesc.FillMode = D3D11_FILL_SOLID;
				rasterizerDesc.CullMode = D3D11_CULL_BACK;
				rasterizerDesc.FrontCounterClockwise = false;
				device->CreateRasterizerState(&rasterizerDesc, s_renderCommandData->cullStates.emplace_back().GetAddressOf());
			}

			// Front
			{
				D3D11_RASTERIZER_DESC rasterizerDesc = {};
				rasterizerDesc.FillMode = D3D11_FILL_SOLID;
				rasterizerDesc.CullMode = D3D11_CULL_FRONT;
				rasterizerDesc.FrontCounterClockwise = false;
				device->CreateRasterizerState(&rasterizerDesc, s_renderCommandData->cullStates.emplace_back().GetAddressOf());
			}

			// None
			{
				D3D11_RASTERIZER_DESC rasterizerDesc = {};
				rasterizerDesc.FillMode = D3D11_FILL_SOLID;
				rasterizerDesc.CullMode = D3D11_CULL_NONE;
				rasterizerDesc.FrontCounterClockwise = false;
				device->CreateRasterizerState(&rasterizerDesc, s_renderCommandData->cullStates.emplace_back().GetAddressOf());
			}
		}

		RestoreDefaultState();
	}

	void RenderCommand::Shutdown()
	{
		s_renderContext = nullptr;
		s_renderCommandData = nullptr;
	}

	void RenderCommand::SetTopology(Topology topology)
	{
		if (topology != s_renderContext->topology)
		{
			s_renderContext->topology = topology;
			GetCurrentContext()->IASetPrimitiveTopology((D3D11_PRIMITIVE_TOPOLOGY)topology);
		}
	}

	void RenderCommand::SetCullState(CullState cullState)
	{
		if (cullState != s_renderContext->cullState)
		{
			s_renderContext->cullState = cullState;
			GetCurrentContext()->RSSetState(s_renderCommandData->cullStates.at((uint32_t)cullState).Get());
		}
	}

	void RenderCommand::SetDepthState(DepthState depthState)
	{
		if (depthState != s_renderContext->depthState)
		{
			s_renderContext->depthState = depthState;
			GetCurrentContext()->OMSetDepthStencilState(s_renderCommandData->depthStates.at((uint32_t)depthState).Get(), 0);
		}
	}

	void RenderCommand::SetContext(Context context)
	{
		if (s_renderContext->context != context)
		{
			s_renderContext->context = context;
			RestoreDefaultState();
		}
	}

	void RenderCommand::Draw(uint32_t vertexCount, uint32_t startVertexLocation)
	{
		auto context = GetCurrentContext();
		context->Draw(vertexCount, startVertexLocation);
	}

	void RenderCommand::DrawIndexed(uint32_t indexCount, uint32_t startIndexLocation, uint32_t baseVertexLocation)
	{
		auto context = GetCurrentContext();
		context->DrawIndexed(indexCount, startIndexLocation, baseVertexLocation);
	}

	void RenderCommand::DrawIndexedInstanced(uint32_t indexCountPerInstance, uint32_t instanceCount, uint32_t startIndexLocation, uint32_t baseVertexLocation, uint32_t startInstanceLocation)
	{
		auto context = GetCurrentContext();
		context->DrawIndexedInstanced(indexCountPerInstance, instanceCount, startIndexLocation, baseVertexLocation, startInstanceLocation);
	}

	void RenderCommand::BeginAnnotation(std::string_view name)
	{
		GetCurrentAnnotations()->BeginEvent(Utils::ToWString(name).c_str());
	}

	void RenderCommand::EndAnnotation()
	{
		GetCurrentAnnotations()->EndEvent();
	}

	void RenderCommand::Sampler_Bind(SamplerState* samplerState, uint32_t slot)
	{
		auto context = GetCurrentContext();
		context->CSSetSamplers(slot, 1, samplerState->GetSampler().GetAddressOf());
		context->PSSetSamplers(slot, 1, samplerState->GetSampler().GetAddressOf());
	}

	void RenderCommand::Sampler_BindMultiple(const std::vector<SamplerState*>& samplerStates, uint32_t startSlot)
	{
		auto context = GetCurrentContext();

		std::vector<ID3D11SamplerState*> samplers{ samplerStates.size() };

		for (uint32_t i = 0; const auto & s : samplerStates)
		{
			samplers[i] = s->GetSampler().Get();
			i++;
		}

		context->CSSetSamplers(startSlot, (uint32_t)samplers.size(), samplers.data());
		context->PSSetSamplers(startSlot, (uint32_t)samplers.size(), samplers.data());
	}

	void* RenderCommand::ConstantBuffer_Map(ConstantBuffer* constantBuffer)
	{
		auto context = GetCurrentContext();

		D3D11_MAPPED_SUBRESOURCE subresource{};
		VT_DX_CHECK(context->Map(constantBuffer->GetHandle().Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &subresource));
		return subresource.pData;
	}

	void RenderCommand::ConstantBuffer_Unmap(ConstantBuffer* constantBuffer)
	{
		auto context = GetCurrentContext();
		context->Unmap(constantBuffer->GetHandle().Get(), 0);
	}

	void RenderCommand::RestoreDefaultState()
	{
		auto context = GetCurrentContext();

		s_renderContext->topology = Topology::TriangleList;
		context->IASetPrimitiveTopology((D3D11_PRIMITIVE_TOPOLOGY)Topology::TriangleList);

		s_renderContext->cullState = CullState::CullBack;
		context->RSSetState(s_renderCommandData->cullStates.at((uint32_t)CullState::CullBack).Get());

		s_renderContext->depthState = DepthState::ReadWrite;
		context->OMSetDepthStencilState(s_renderCommandData->depthStates.at((uint32_t)DepthState::ReadWrite).Get(), 0);
	}
}