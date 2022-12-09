#pragma once

#include "Volt/Core/Base.h"
#include "Volt/Core/Graphics/GraphicsContext.h"
#include "Volt/Rendering/Shader/ShaderCommon.h"
#include "Volt/Utility/DirectXUtils.h"

#include <wrl.h>
#include <d3d11.h>
#include <cstdint>

struct ID3D11Buffer;

using namespace Microsoft::WRL;
namespace Volt
{
	class ConstantBuffer
	{
	public:
		ConstantBuffer(const void* aData, uint32_t aSize, ShaderStage aUsageStage);
		~ConstantBuffer();

		inline const ComPtr<ID3D11Buffer> GetHandle() const { return myBuffer; }
		inline const uint32_t GetSize() const { return mySize; }

		void SetData(const void* aData, uint32_t aSize, bool deferr = false);
		void RT_SetData(const void* aData, uint32_t aSize);

		void Bind(uint32_t aSlot);
		void RT_Bind(uint32_t aSlot);
		
		void AddStage(ShaderStage aStage);

		template<typename T>
		T* Map();

		template<typename T>
		T* RT_Map();

		void Unmap();
		void RT_Unmap();

		static Ref<ConstantBuffer> Create(const void* aData, uint32_t aSize, ShaderStage aUsageStage);

	private:
		uint32_t mySize;
		ComPtr<ID3D11Buffer> myBuffer = nullptr;

		ShaderStage myUsageStages;
	};

	template<typename T>
	inline T* ConstantBuffer::Map()
	{
		auto context = GraphicsContext::GetImmediateContext();
		
		D3D11_MAPPED_SUBRESOURCE subresource{};
		VT_DX_CHECK(context->Map(myBuffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &subresource));
		return reinterpret_cast<T*>(subresource.pData);
	}
	template<typename T>
	inline T* ConstantBuffer::RT_Map()
	{
		auto context = GraphicsContext::GetDeferredContext();

		D3D11_MAPPED_SUBRESOURCE subresource{};
		VT_DX_CHECK(context->Map(myBuffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &subresource));
		return reinterpret_cast<T*>(subresource.pData);
	}
}