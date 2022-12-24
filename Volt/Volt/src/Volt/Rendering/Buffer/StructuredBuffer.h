#pragma once

#include "Volt/Core/Base.h"
#include "Volt/Rendering/Shader/ShaderUtility.h"
#include "Volt/Rendering/RenderCommand.h"

#include <wrl.h>

using namespace Microsoft::WRL;

struct ID3D11Buffer;
struct ID3D11ShaderResourceView;
struct ID3D11UnorderedAccessView;

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
		void AddStage(ShaderStage stage);
		
		template<typename T>
		T* Map();
		void Unmap();

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
		void* mappedData = RenderCommand::StructuredBuffer_Map(this);
		memcpy(mappedData, data, sizeof(T) * count);
		RenderCommand::StructuredBuffer_Unmap(this);
	}

	template<typename T>
	inline T* StructuredBuffer::Map()
	{
		return reinterpret_cast<T*>(RenderCommand::StructuredBuffer_Map(this));
	}
}