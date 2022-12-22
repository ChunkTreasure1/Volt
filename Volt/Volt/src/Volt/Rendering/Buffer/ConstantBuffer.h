#pragma once

#include "Volt/Core/Base.h"
#include "Volt/Rendering/Shader/ShaderCommon.h"

#include "Volt/Rendering/RenderCommand.h"

#include <wrl.h>

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
		void Bind(uint32_t aSlot);
		void AddStage(ShaderStage aStage);

		template<typename T>
		T* Map();
		void Unmap();

		static Ref<ConstantBuffer> Create(const void* aData, uint32_t aSize, ShaderStage aUsageStage);

	private:
		uint32_t mySize;
		ComPtr<ID3D11Buffer> myBuffer = nullptr;

		ShaderStage myUsageStages;
	};

	template<typename T>
	inline T* ConstantBuffer::Map()
	{
		return reinterpret_cast<T*>(RenderCommand::ConstantBuffer_Map(this));
	}
}