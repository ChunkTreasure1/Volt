#pragma once

#include "Volt/Core/Base.h"

#include <wrl.h>
#include <d3d11.h>

using namespace Microsoft::WRL;
namespace Volt
{
	class SamplerState
	{
	public:
		SamplerState(D3D11_FILTER aFilter, D3D11_TEXTURE_ADDRESS_MODE aAddressMode, D3D11_COMPARISON_FUNC aComparison = D3D11_COMPARISON_NEVER);
		~SamplerState();

		void Bind(uint32_t aSlot);

		inline ComPtr<ID3D11SamplerState> GetSampler() const { return mySampler; }
		static Ref<SamplerState> Create(D3D11_FILTER aFilter, D3D11_TEXTURE_ADDRESS_MODE aAddressMode, D3D11_COMPARISON_FUNC aComparison = D3D11_COMPARISON_NEVER);

	private:
		ComPtr<ID3D11SamplerState> mySampler;
	};
}