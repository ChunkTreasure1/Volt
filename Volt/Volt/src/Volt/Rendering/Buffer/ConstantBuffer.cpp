#include "vtpch.h"
#include "ConstantBuffer.h"

#include "Volt/Rendering/Renderer.h"
#include "Volt/Core/Buffer.h"

namespace Volt
{
	ConstantBuffer::ConstantBuffer(const void* aData, uint32_t aSize, ShaderStage aUsageStage)
		: mySize(aSize), myUsageStages(aUsageStage)
	{
		VT_CORE_ASSERT(aSize % 16 == 0, "Constant buffer must be 16 byte aligned!");

		D3D11_BUFFER_DESC constantBuffer{};
		constantBuffer.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
		constantBuffer.Usage = D3D11_USAGE_DYNAMIC;
		constantBuffer.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
		constantBuffer.MiscFlags = 0;
		constantBuffer.StructureByteStride = 0;
		constantBuffer.ByteWidth = aSize;

		D3D11_SUBRESOURCE_DATA data{};
		data.pSysMem = aData;

		auto device = GraphicsContext::GetDevice();
		VT_DX_CHECK(device->CreateBuffer(&constantBuffer, aData ? &data : nullptr, myBuffer.GetAddressOf()));
	}

	ConstantBuffer::~ConstantBuffer()
	{
		myBuffer.Reset();
	}

	void ConstantBuffer::SetData(const void* aData, uint32_t aSize, bool deferr)
	{
		if (deferr)
		{
			Buffer dataBuffer{ aSize };
			dataBuffer.Copy(aData, aSize);

			Renderer::SubmitResourceChange([data = dataBuffer, buffer = myBuffer]() 
				{
					auto context = GraphicsContext::GetContext();

					D3D11_MAPPED_SUBRESOURCE subresource;
					VT_DX_CHECK(context->Map(buffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &subresource));
					memcpy(subresource.pData, data.As<void>(), data.GetSize());
					context->Unmap(buffer.Get(), 0);
				});
		}
		else
		{
			auto context = GraphicsContext::GetContext();

			D3D11_MAPPED_SUBRESOURCE subresource;
			VT_DX_CHECK(context->Map(myBuffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &subresource));
			memcpy(subresource.pData, aData, aSize);
			context->Unmap(myBuffer.Get(), 0);
		}

	}

	void ConstantBuffer::Bind(uint32_t aSlot)
	{
		auto context = GraphicsContext::GetContext();

		if ((myUsageStages & ShaderStage::Vertex) != ShaderStage::None)
		{
			context->VSSetConstantBuffers(aSlot, 1, myBuffer.GetAddressOf());
		}

		if ((myUsageStages & ShaderStage::Pixel) != ShaderStage::None)
		{
			context->PSSetConstantBuffers(aSlot, 1, myBuffer.GetAddressOf());
		}

		if ((myUsageStages & ShaderStage::Compute) != ShaderStage::None)
		{
			context->CSSetConstantBuffers(aSlot, 1, myBuffer.GetAddressOf());
		}

		if ((myUsageStages & ShaderStage::Domain) != ShaderStage::None)
		{
			context->DSSetConstantBuffers(aSlot, 1, myBuffer.GetAddressOf());
		}

		if ((myUsageStages & ShaderStage::Hull) != ShaderStage::None)
		{
			context->HSSetConstantBuffers(aSlot, 1, myBuffer.GetAddressOf());
		}

		if ((myUsageStages & ShaderStage::Geometry) != ShaderStage::None)
		{
			context->GSSetConstantBuffers(aSlot, 1, myBuffer.GetAddressOf());
		}
	}

	void ConstantBuffer::AddStage(ShaderStage aStage)
	{
		myUsageStages = myUsageStages | aStage;
	}

	void ConstantBuffer::Unmap()
	{
		auto context = GraphicsContext::GetContext();
		context->Unmap(myBuffer.Get(), 0);
	}

	Ref<ConstantBuffer> ConstantBuffer::Create(const void* aData, uint32_t aSize, ShaderStage aUsageStage)
	{
		return CreateRef<ConstantBuffer>(aData, aSize, aUsageStage);
	}
}