#include "vtpch.h"
#include "VertexBuffer.h"

#include "Volt/Log/Log.h"
#include "Volt/Utility/DirectXUtils.h"

#include "Volt/Core/Graphics/GraphicsContext.h"

#include <d3d11.h>

namespace Volt
{
	VertexBuffer::VertexBuffer(const void* aData, uint32_t aSize, uint32_t aStride, BufferUsage usage)
		: myStride(aStride)
	{
		const D3D11_USAGE usageType = (D3D11_USAGE)usage;

		D3D11_BUFFER_DESC vertexBuffer{};
		vertexBuffer.BindFlags = D3D11_BIND_VERTEX_BUFFER;
		vertexBuffer.Usage = usageType;
		vertexBuffer.ByteWidth = aSize;
		vertexBuffer.CPUAccessFlags = usageType == D3D11_USAGE_DYNAMIC ? D3D11_CPU_ACCESS_WRITE : 0;
		vertexBuffer.MiscFlags = 0;
		vertexBuffer.StructureByteStride = aStride;

		D3D11_SUBRESOURCE_DATA data{};
		data.pSysMem = aData;

		auto device = GraphicsContext::GetDevice();
		VT_DX_CHECK(device->CreateBuffer(&vertexBuffer, &data, &myBuffer));
	}

	VertexBuffer::VertexBuffer(uint32_t aSize, uint32_t aStride)
		: myStride(aStride)
	{
		D3D11_BUFFER_DESC vertexBuffer{};
		vertexBuffer.BindFlags = D3D11_BIND_VERTEX_BUFFER;
		vertexBuffer.Usage = D3D11_USAGE_DYNAMIC;
		vertexBuffer.ByteWidth = aSize;
		vertexBuffer.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
		vertexBuffer.MiscFlags = 0;
		vertexBuffer.StructureByteStride = aStride;

		auto device = GraphicsContext::GetDevice();
		VT_DX_CHECK(device->CreateBuffer(&vertexBuffer, nullptr, myBuffer.GetAddressOf()));
	}

	VertexBuffer::~VertexBuffer()
	{
		myBuffer.Reset();
	}

	void VertexBuffer::SetName(const std::string& name)
	{
		myBuffer->SetPrivateData(WKPDID_D3DDebugObjectName, (uint32_t)name.size(), name.c_str());
	}

	void VertexBuffer::SetData(const void* aData, uint32_t aSize)
	{
		void* mappedData = RenderCommand::VertexBuffer_Map(this);
		memcpy(mappedData, aData, aSize);
		RenderCommand::VertexBuffer_Unmap(this);
	}

	void VertexBuffer::Bind(uint32_t aSlot) const
	{
		RenderCommand::VertexBuffer_Bind(this, aSlot, myStride);
	}

	void VertexBuffer::Unmap()
	{
		RenderCommand::VertexBuffer_Unmap(this);
	}

	Ref<VertexBuffer> VertexBuffer::Create(const void* data, uint32_t aSize, uint32_t aStride, BufferUsage usage)
	{
		return CreateRef<VertexBuffer>(data, aSize, aStride, usage);
	}

	Ref<VertexBuffer> VertexBuffer::Create(uint32_t aSize, uint32_t aStride)
	{
		return CreateRef<VertexBuffer>(aSize, aStride);
	}
}