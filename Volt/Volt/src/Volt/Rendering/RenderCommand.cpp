#include "vtpch.h"
#include "RenderCommand.h"

#include "Volt/Core/Graphics/GraphicsContext.h"
#include "Volt/Core/Profiling.h"

#include "Volt/Rendering/SamplerState.h"
#include "Volt/Rendering/Buffer/ConstantBuffer.h"
#include "Volt/Rendering/Buffer/StructuredBuffer.h"
#include "Volt/Rendering/Buffer/VertexBuffer.h"
#include "Volt/Rendering/Buffer/IndexBuffer.h"

#include "Volt/Rendering/Texture/Image2D.h"

#include "Volt/Utility/DirectXUtils.h"
#include "Volt/Utility/StringUtility.h"

#include <unordered_map>

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

		std::unordered_map<std::thread::id, RenderContext> threadContexts;
	};

	inline static Scope<RenderCommandData> s_renderCommandData;

	inline static ID3DUserDefinedAnnotation* GetCurrentAnnotations()
	{
		const std::thread::id threadId = std::this_thread::get_id();
		return s_renderCommandData->annotations.at((uint32_t)s_renderCommandData->threadContexts[threadId].context).Get();
	}

	inline static RenderContext& GetCurrentContextData()
	{
		const std::thread::id threadId = std::this_thread::get_id();
		return s_renderCommandData->threadContexts[threadId];
	}

	void RenderCommand::Initialize()
	{
		s_renderCommandData = CreateScope<RenderCommandData>();

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
		s_renderCommandData = nullptr;
	}

	void RenderCommand::SetTopology(Topology topology)
	{
		VT_PROFILE_FUNCTION();

		auto& context = GetCurrentContextData();

		if (topology != context.topology)
		{
			context.topology = topology;
			GetCurrentContext()->IASetPrimitiveTopology((D3D11_PRIMITIVE_TOPOLOGY)topology);
		}
	}

	void RenderCommand::SetCullState(CullState cullState)
	{
		VT_PROFILE_FUNCTION();

		auto& context = GetCurrentContextData();

		if (cullState != context.cullState)
		{
			context.cullState = cullState;
			GetCurrentContext()->RSSetState(s_renderCommandData->cullStates.at((uint32_t)cullState).Get());
		}
	}

	void RenderCommand::SetDepthState(DepthState depthState)
	{
		VT_PROFILE_FUNCTION();

		auto& context = GetCurrentContextData();

		if (depthState != context.depthState)
		{
			context.depthState = depthState;
			GetCurrentContext()->OMSetDepthStencilState(s_renderCommandData->depthStates.at((uint32_t)depthState).Get(), 0);
		}
	}

	void RenderCommand::SetContext(Context context)
	{
		VT_PROFILE_FUNCTION();

		auto& threadContext = GetCurrentContextData();

		if (threadContext.context != context)
		{
			threadContext.context = context;
			RestoreDefaultState();
		}
	}

	void RenderCommand::Draw(uint32_t vertexCount, uint32_t startVertexLocation)
	{
		VT_PROFILE_FUNCTION();

		auto context = GetCurrentContext();
		context->Draw(vertexCount, startVertexLocation);
	}

	void RenderCommand::DrawIndexed(uint32_t indexCount, uint32_t startIndexLocation, uint32_t baseVertexLocation)
	{
		VT_PROFILE_FUNCTION();

		auto context = GetCurrentContext();
		context->DrawIndexed(indexCount, startIndexLocation, baseVertexLocation);
	}

	void RenderCommand::DrawIndexedInstanced(uint32_t indexCountPerInstance, uint32_t instanceCount, uint32_t startIndexLocation, uint32_t baseVertexLocation, uint32_t startInstanceLocation)
	{
		VT_PROFILE_FUNCTION();

		auto context = GetCurrentContext();
		context->DrawIndexedInstanced(indexCountPerInstance, instanceCount, startIndexLocation, baseVertexLocation, startInstanceLocation);
	}

	void RenderCommand::BeginAnnotation(std::string_view name)
	{
		VT_PROFILE_FUNCTION();
		auto annotations = GetCurrentAnnotations();
		annotations->BeginEvent(Utils::ToWString(name).c_str());
	}

	void RenderCommand::EndAnnotation()
	{
		VT_PROFILE_FUNCTION();
		auto annotations = GetCurrentAnnotations();
		annotations->EndEvent();
	}

	void RenderCommand::Sampler_Bind(const SamplerState* samplerState, uint32_t slot)
	{
		VT_PROFILE_FUNCTION();

		auto context = GetCurrentContext();
		context->CSSetSamplers(slot, 1, samplerState->GetSampler().GetAddressOf());
		context->PSSetSamplers(slot, 1, samplerState->GetSampler().GetAddressOf());
	}

	void RenderCommand::Sampler_BindMultiple(const std::vector<SamplerState*>& samplerStates, uint32_t startSlot)
	{
		VT_PROFILE_FUNCTION();

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

	void* RenderCommand::ConstantBuffer_Map(const ConstantBuffer* constantBuffer)
	{
		VT_PROFILE_FUNCTION();

		auto context = GetCurrentContext();

		D3D11_MAPPED_SUBRESOURCE subresource{};
		VT_DX_CHECK(context->Map(constantBuffer->GetHandle().Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &subresource));
		return subresource.pData;
	}

	void RenderCommand::ConstantBuffer_Unmap(const ConstantBuffer* constantBuffer)
	{
		VT_PROFILE_FUNCTION();

		auto context = GetCurrentContext();
		context->Unmap(constantBuffer->GetHandle().Get(), 0);
	}

	void* RenderCommand::StructuredBuffer_Map(const StructuredBuffer* structuredBuffer)
	{
		VT_PROFILE_FUNCTION();

		auto context = GetCurrentContext();

		D3D11_MAPPED_SUBRESOURCE subresource{};
		VT_DX_CHECK(context->Map(structuredBuffer->GetHandle().Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &subresource));
		return subresource.pData;
	}

	void RenderCommand::StructuredBuffer_Unmap(const StructuredBuffer* structuredBuffer)
	{
		VT_PROFILE_FUNCTION();

		auto context = GetCurrentContext();
		context->Unmap(structuredBuffer->GetHandle().Get(), 0);
	}

	void RenderCommand::VertexBuffer_Bind(const VertexBuffer* vertexBuffer, uint32_t slot, uint32_t stride, uint32_t offset)
	{
		VT_PROFILE_FUNCTION();

		auto context = GetCurrentContext();

		if (!vertexBuffer)
		{
			context->IASetVertexBuffers(0, 0, nullptr, nullptr, nullptr);
		}
		else
		{
			context->IASetVertexBuffers(slot, 1, vertexBuffer->GetHandle().GetAddressOf(), &stride, &offset);
		}
	}

	void* RenderCommand::VertexBuffer_Map(const VertexBuffer* vertexBuffer)
	{
		VT_PROFILE_FUNCTION();

		auto context = GetCurrentContext();

		D3D11_MAPPED_SUBRESOURCE subresource{};
		VT_DX_CHECK(context->Map(vertexBuffer->GetHandle().Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &subresource));
		return subresource.pData;
	}

	void RenderCommand::VertexBuffer_Unmap(const VertexBuffer* vertexBuffer)
	{
		VT_PROFILE_FUNCTION();

		auto context = GetCurrentContext();
		context->Unmap(vertexBuffer->GetHandle().Get(), 0);
	}

	void RenderCommand::IndexBuffer_Bind(const IndexBuffer* indexBuffer)
	{
		VT_PROFILE_FUNCTION();

		auto context = GetCurrentContext();

		if (!indexBuffer)
		{
			context->IASetIndexBuffer(nullptr, DXGI_FORMAT_UNKNOWN, 0);
		}
		else
		{
			context->IASetIndexBuffer(indexBuffer->GetHandle().Get(), DXGI_FORMAT_R32_UINT, 0);
		}
	}

	void RenderCommand::BindTexturesToStage(const ShaderStage stage, const std::vector<Ref<Image2D>>& textures, const uint32_t startSlot)
	{
		VT_PROFILE_FUNCTION();

		auto context = GetCurrentContext();

		std::vector<ID3D11ShaderResourceView*> texturesToBind{ textures.size() };
		for (uint32_t i = 0; const auto & t : textures)
		{
			if (t)
			{
				texturesToBind[i] = t->GetSRV().Get();
			}
			else
			{
				texturesToBind[i] = nullptr;
			}
			i++;
		}

		switch (stage)
		{
			case ShaderStage::Vertex:
				context->VSSetShaderResources(startSlot, (uint32_t)texturesToBind.size(), texturesToBind.data());
				break;
			case ShaderStage::Pixel:
				context->PSSetShaderResources(startSlot, (uint32_t)texturesToBind.size(), texturesToBind.data());
				break;
			case ShaderStage::Hull:
				context->HSSetShaderResources(startSlot, (uint32_t)texturesToBind.size(), texturesToBind.data());
				break;
			case ShaderStage::Domain:
				context->DSSetShaderResources(startSlot, (uint32_t)texturesToBind.size(), texturesToBind.data());
				break;
			case ShaderStage::Geometry:
				context->GSSetShaderResources(startSlot, (uint32_t)texturesToBind.size(), texturesToBind.data());
				break;
			case ShaderStage::Compute:
				context->CSSetShaderResources(startSlot, (uint32_t)texturesToBind.size(), texturesToBind.data());
				break;
			default:
				break;
		}
	}

	void RenderCommand::ClearTexturesAtStage(const ShaderStage stage, const uint32_t startSlot, const uint32_t count)
	{
		VT_PROFILE_FUNCTION();

		auto context = GetCurrentContext();
		std::vector<ID3D11ShaderResourceView*> textures{ count, nullptr };

		switch (stage)
		{
			case ShaderStage::Vertex:
				context->VSSetShaderResources(startSlot, (uint32_t)textures.size(), textures.data());
				break;
			case ShaderStage::Pixel:
				context->PSSetShaderResources(startSlot, (uint32_t)textures.size(), textures.data());
				break;
			case ShaderStage::Hull:
				context->HSSetShaderResources(startSlot, (uint32_t)textures.size(), textures.data());
				break;
			case ShaderStage::Domain:
				context->DSSetShaderResources(startSlot, (uint32_t)textures.size(), textures.data());
				break;
			case ShaderStage::Geometry:
				context->GSSetShaderResources(startSlot, (uint32_t)textures.size(), textures.data());
				break;
			case ShaderStage::Compute:
				context->CSSetShaderResources(startSlot, (uint32_t)textures.size(), textures.data());
				break;
			default:
				break;
		}
	}

	ID3D11DeviceContext* RenderCommand::GetCurrentContext()
	{
		const auto& contextData = GetCurrentContextData();
		return s_renderCommandData->deviceContexts.at((uint32_t)contextData.context).Get();
	}

	void RenderCommand::RestoreDefaultState()
	{
		VT_PROFILE_FUNCTION();

		auto& contextData = GetCurrentContextData();
		auto context = GetCurrentContext();

		contextData.topology = Topology::TriangleList;
		context->IASetPrimitiveTopology((D3D11_PRIMITIVE_TOPOLOGY)Topology::TriangleList);

		contextData.cullState = CullState::CullBack;
		context->RSSetState(s_renderCommandData->cullStates.at((uint32_t)CullState::CullBack).Get());

		contextData.depthState = DepthState::ReadWrite;
		context->OMSetDepthStencilState(s_renderCommandData->depthStates.at((uint32_t)DepthState::ReadWrite).Get(), 0);
	}
}