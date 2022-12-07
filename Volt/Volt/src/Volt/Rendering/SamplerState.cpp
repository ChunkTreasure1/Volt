#include "vtpch.h"
#include "SamplerState.h"

#include "Volt/Core/Graphics/GraphicsContext.h"

namespace Volt
{
	SamplerState::SamplerState(D3D11_FILTER aFilter, D3D11_TEXTURE_ADDRESS_MODE aAddressMode, D3D11_COMPARISON_FUNC aComparison)
	{
		auto device = GraphicsContext::GetDevice();
		
		D3D11_SAMPLER_DESC samplerDesc;
		samplerDesc.Filter = aFilter; 
		samplerDesc.AddressU = aAddressMode;
		samplerDesc.AddressV = aAddressMode;
		samplerDesc.AddressW = aAddressMode;
		samplerDesc.MipLODBias = 0.0f;
		samplerDesc.MaxAnisotropy = aFilter == D3D11_FILTER_ANISOTROPIC ? 16 : 1;
		samplerDesc.ComparisonFunc = aComparison;
		samplerDesc.BorderColor[0] = 1;
		samplerDesc.BorderColor[1] = 1;
		samplerDesc.BorderColor[2] = 1;
		samplerDesc.BorderColor[3] = 1;
		samplerDesc.MinLOD = 0.f;
		samplerDesc.MaxLOD = D3D11_FLOAT32_MAX;

		device->CreateSamplerState(&samplerDesc, mySampler.GetAddressOf());
	}

	SamplerState::~SamplerState()
	{
		mySampler = nullptr;
	}

	void SamplerState::Bind(uint32_t aSlot)
	{
		auto context = GraphicsContext::GetContext();
		context->CSSetSamplers(aSlot, 1, mySampler.GetAddressOf());
		context->PSSetSamplers(aSlot, 1, mySampler.GetAddressOf());
	}

	Ref<SamplerState> SamplerState::Create(D3D11_FILTER aFilter, D3D11_TEXTURE_ADDRESS_MODE aAddressMode, D3D11_COMPARISON_FUNC aComparison)
	{
		return CreateRef<SamplerState>(aFilter, aAddressMode, aComparison);
	}
}