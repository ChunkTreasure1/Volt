#pragma once

#include "Volt/Core/Base.h"
#include "Volt/Rendering/Vertex.h"
#include "Volt/Core/Graphics/GraphicsContext.h"
#include "Volt/Utility/DirectXUtils.h"

#include <vector>
#include <d3d11.h>
#include <wrl.h>

using namespace Microsoft::WRL;
namespace Volt
{
	class VertexBuffer
	{
	public:
		VertexBuffer(const void* data, uint32_t aSize, uint32_t aStride, D3D11_USAGE usage);
		VertexBuffer(uint32_t aSize, uint32_t aStride);
		~VertexBuffer();

		void SetName(const std::string& name);
		void SetData(const void* aData, uint32_t aSize);
		void Bind(uint32_t aSlot = 0) const;
		void RT_Bind(uint32_t aSlot = 0) const;

		template<typename T>
		T* Map();

		template<typename T>
		T* RT_Map();

		void Unmap();
		void RT_Unmap();

		static Ref<VertexBuffer> Create(const void* data, uint32_t aSize, uint32_t aStride, D3D11_USAGE usage = D3D11_USAGE_IMMUTABLE);
		static Ref<VertexBuffer> Create(uint32_t aSize, uint32_t aStride);
	
	private:
		ComPtr<ID3D11Buffer> myBuffer = nullptr;
		uint32_t myStride = 0;
	};
	template<typename T>
	inline T* VertexBuffer::Map()
	{
		auto context = GraphicsContext::GetImmediateContext();

		D3D11_MAPPED_SUBRESOURCE subresource{};
		VT_DX_CHECK(context->Map(myBuffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &subresource));
		return reinterpret_cast<T*>(subresource.pData);
	}

	template<typename T>
	inline T* VertexBuffer::RT_Map()
	{
		auto context = GraphicsContext::GetDeferredContext();

		D3D11_MAPPED_SUBRESOURCE subresource{};
		VT_DX_CHECK(context->Map(myBuffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &subresource));
		return reinterpret_cast<T*>(subresource.pData);
	}
}