#include "vtpch.h"
#include "IndexBuffer.h"

#include "Volt/Core/Graphics/GraphicsContext.h"
#include "Volt/Utility/DirectXUtils.h"

namespace Volt
{
	IndexBuffer::IndexBuffer(const std::vector<uint32_t>& aIndices, uint32_t aCount)
		: myCount(aCount)
	{
		SetData(aIndices.data(), sizeof(uint32_t) * aCount);
	}

	IndexBuffer::IndexBuffer(uint32_t* aIndices, uint32_t aCount)
		: myCount(aCount)
	{
		SetData(aIndices, sizeof(uint32_t) * aCount);
	}

	IndexBuffer::~IndexBuffer()
	{
		myBuffer.Reset();
	}

	void IndexBuffer::Bind() const
	{
		auto context = GraphicsContext::GetImmediateContext();
		context->IASetIndexBuffer(myBuffer.Get(), DXGI_FORMAT_R32_UINT, 0);
	}

	void IndexBuffer::RT_Bind() const
	{
		auto context = GraphicsContext::GetDeferredContext();
		context->IASetIndexBuffer(myBuffer.Get(), DXGI_FORMAT_R32_UINT, 0);
	}

	Ref<IndexBuffer> IndexBuffer::Create(const std::vector<uint32_t>& aIndices, uint32_t aCount)
	{
		return CreateRef<IndexBuffer>(aIndices, aCount);
	}

	Ref<IndexBuffer> IndexBuffer::Create(uint32_t* aIndices, uint32_t aCount)
	{
		return CreateRef<IndexBuffer>(aIndices, aCount);
	}

	void IndexBuffer::SetData(const void* aData, uint32_t aSize)
	{
		D3D11_BUFFER_DESC bufferDesc{};
		bufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
		bufferDesc.Usage = D3D11_USAGE_IMMUTABLE;
		bufferDesc.CPUAccessFlags = 0;
		bufferDesc.MiscFlags = 0;
		bufferDesc.ByteWidth = aSize;
		bufferDesc.StructureByteStride = 0;

		D3D11_SUBRESOURCE_DATA data{};
		data.pSysMem = aData;

		auto device = GraphicsContext::GetDevice();
		VT_DX_CHECK(device->CreateBuffer(&bufferDesc, &data, myBuffer.GetAddressOf()));
	}
}