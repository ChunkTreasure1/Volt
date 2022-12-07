#pragma once

#include "Volt/Core/Base.h"

#include <vector>
#include <wrl.h>

struct ID3D11Buffer;

using namespace Microsoft::WRL;
namespace Volt
{
	class IndexBuffer
	{
	public:
		IndexBuffer(const std::vector<uint32_t>& aIndices, uint32_t aCount);
		IndexBuffer(uint32_t* aIndices, uint32_t aCount);
		~IndexBuffer();

		void Bind();

		static Ref<IndexBuffer> Create(const std::vector<uint32_t>& aIndices, uint32_t aCount);
		static Ref<IndexBuffer> Create(uint32_t* aIndices, uint32_t aCount);

	private:
		void SetData(const void* aData, uint32_t aSize);

		ComPtr<ID3D11Buffer> myBuffer = nullptr;
		uint32_t myCount = 0;
	};
}