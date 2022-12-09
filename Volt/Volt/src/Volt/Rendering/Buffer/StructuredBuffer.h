#pragma once

#include "Volt/Core/Base.h"
#include "Volt/Core/Graphics/GraphicsContext.h"
#include "Volt/Utility/DirectXUtils.h"
#include "Volt/Rendering/Shader/ShaderUtility.h"

#include <wrl.h>
#include <d3d11.h>

using namespace Microsoft::WRL;
namespace Volt
{
	class StructuredBuffer
	{
	public:
		StructuredBuffer(uint32_t elementSize, uint32_t count, ShaderStage usageStage, bool shaderWriteable = false);
		~StructuredBuffer();

		inline const ComPtr<ID3D11Buffer> GetHandle() const { return myBuffer; }
		inline const uint32_t GetSize() const { return myElementSize; }

		inline const ComPtr<ID3D11ShaderResourceView> GetSRV() const { return mySRV; }
		inline const ComPtr<ID3D11UnorderedAccessView> GetUAV() const { return myUAV; }

		void Resize(uint32_t elememtCount);

		template<typename T>
		void SetData(const T* data, uint32_t count);
		
		void Bind(uint32_t slot) const;
		void RT_Bind(uint32_t slot) const;
		void AddStage(ShaderStage stage);
		
		template<typename T>
		T* Map();

		template<typename T>
		T* RT_Map();

		void Unmap();
		void RT_Unmap();

		static Ref<StructuredBuffer> Create(uint32_t elementSize, uint32_t count, ShaderStage usageStage, bool shaderWriteable = false);

	private:
		void Invalidate(bool resize);

		ComPtr<ID3D11Buffer> myBuffer = nullptr;
		ComPtr<ID3D11ShaderResourceView> mySRV = nullptr;
		ComPtr<ID3D11UnorderedAccessView> myUAV = nullptr;

		uint32_t myMaxCount;
		const uint32_t myElementSize;
		ShaderStage myUsageStages;

		bool myShaderWriteable = false;
	};

	template<typename T>
	inline void StructuredBuffer::SetData(const T* data, uint32_t count)
	{
		VT_CORE_ASSERT(count <= myMaxCount, "The data cannot be larger than the buffer!");
			
		auto context = GraphicsContext::GetImmediateContext();

		D3D11_MAPPED_SUBRESOURCE subresource;
		VT_DX_CHECK(context->Map(myBuffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &subresource));
		memcpy(subresource.pData, data, sizeof(T) * count);
		context->Unmap(myBuffer.Get(), 0);
	}

	template<typename T>
	inline T* StructuredBuffer::Map()
	{
		auto context = GraphicsContext::GetImmediateContext();

		D3D11_MAPPED_SUBRESOURCE subresource{};
		VT_DX_CHECK(context->Map(myBuffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &subresource));

		return reinterpret_cast<T*>(subresource.pData);
	}
	template<typename T>
	inline T* StructuredBuffer::RT_Map()
	{
		auto context = GraphicsContext::GetDeferredContext();

		D3D11_MAPPED_SUBRESOURCE subresource{};
		VT_DX_CHECK(context->Map(myBuffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &subresource));

		return reinterpret_cast<T*>(subresource.pData);
	}
}