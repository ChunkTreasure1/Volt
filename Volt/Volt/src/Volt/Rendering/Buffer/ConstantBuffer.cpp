#include "vtpch.h"
#include "ConstantBuffer.h"

#include "Volt/Core/Buffer.h"
#include "Volt/Core/Graphics/GraphicsContext.h"

#include "Volt/Rendering/Renderer.h"

#include "Volt/Utility/DirectXUtils.h"

#include <d3d11.h>

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

			Renderer::SubmitResourceChange([this, data = dataBuffer]() 
				{
					void* mappedData = RenderCommand::ConstantBuffer_Map(this);
					memcpy(mappedData, data.As<void>(), data.GetSize());
					RenderCommand::ConstantBuffer_Unmap(this);
				});
		}
		else
		{
			void* mappedData = RenderCommand::ConstantBuffer_Map(this);
			memcpy(mappedData, aData, aSize);
			RenderCommand::ConstantBuffer_Unmap(this);
		}

	}

	void ConstantBuffer::Bind(uint32_t aSlot)
	{
		auto context = RenderCommand::GetCurrentContext();

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
		RenderCommand::ConstantBuffer_Unmap(this);
	}

	Ref<ConstantBuffer> ConstantBuffer::Create(const void* aData, uint32_t aSize, ShaderStage aUsageStage)
	{
		return CreateRef<ConstantBuffer>(aData, aSize, aUsageStage);
	}
}