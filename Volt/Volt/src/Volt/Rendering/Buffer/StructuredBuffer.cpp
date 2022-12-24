#include "vtpch.h"
#include "StructuredBuffer.h"

#include "Volt/Core/Graphics/GraphicsContext.h"
#include "Volt/Utility/DirectXUtils.h"

#include <d3d11.h>

namespace Volt
{
	StructuredBuffer::StructuredBuffer(uint32_t elementSize, uint32_t count, ShaderStage usageStage, bool shaderWriteable)
		: myElementSize(elementSize), myMaxCount(count), myUsageStages(usageStage), myShaderWriteable(shaderWriteable)
	{
		Invalidate(false);
	}

	StructuredBuffer::~StructuredBuffer()
	{
		myBuffer.Reset();
	}

	void StructuredBuffer::Resize(uint32_t elememtCount)
	{
		if (myMaxCount >= elememtCount)
		{
			return;
		}

		myMaxCount = elememtCount;
		Invalidate(true);
	}

	void StructuredBuffer::Bind(uint32_t slot) const
	{
		auto context = RenderCommand::GetCurrentContext();

		if ((myUsageStages & ShaderStage::Vertex) != ShaderStage::None)
		{
			context->VSSetShaderResources(slot, 1, mySRV.GetAddressOf());
		}

		if ((myUsageStages & ShaderStage::Pixel) != ShaderStage::None)
		{
			context->PSSetShaderResources(slot, 1, mySRV.GetAddressOf());
		}

		if ((myUsageStages & ShaderStage::Compute) != ShaderStage::None)
		{
			context->CSSetShaderResources(slot, 1, mySRV.GetAddressOf());
		}

		if ((myUsageStages & ShaderStage::Domain) != ShaderStage::None)
		{
			context->DSSetShaderResources(slot, 1, mySRV.GetAddressOf());
		}

		if ((myUsageStages & ShaderStage::Hull) != ShaderStage::None)
		{
			context->HSSetShaderResources(slot, 1, mySRV.GetAddressOf());
		}

		if ((myUsageStages & ShaderStage::Geometry) != ShaderStage::None)
		{
			context->GSSetShaderResources(slot, 1, mySRV.GetAddressOf());
		}
	}

	void StructuredBuffer::Unmap()
	{
		RenderCommand::StructuredBuffer_Unmap(this);
	}

	void StructuredBuffer::AddStage(ShaderStage stage)
	{
		myUsageStages = myUsageStages | stage;
	}

	Ref<StructuredBuffer> StructuredBuffer::Create(uint32_t elementSize, uint32_t count, ShaderStage usageStage, bool shaderWriteable)
	{
		return CreateRef<StructuredBuffer>(elementSize, count, usageStage, shaderWriteable);
	}

	void StructuredBuffer::Invalidate(bool resize)
	{
		VT_CORE_ASSERT(myElementSize % 16 == 0, "Structure must be 16 byte aligned!");

		auto device = GraphicsContext::GetDevice();

		ComPtr<ID3D11Buffer> tempBuffer;

		D3D11_BUFFER_DESC desc{};
		desc.ByteWidth = myElementSize * myMaxCount;
		desc.Usage = myShaderWriteable ? D3D11_USAGE_DEFAULT : D3D11_USAGE_DYNAMIC;
		desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
		desc.BindFlags = myShaderWriteable ? D3D11_BIND_UNORDERED_ACCESS : D3D11_BIND_SHADER_RESOURCE;
		desc.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;
		desc.StructureByteStride = myElementSize;

		VT_DX_CHECK(device->CreateBuffer(&desc, nullptr, tempBuffer.GetAddressOf()));

		if (resize)
		{
			auto context = GraphicsContext::GetImmediateContext();

			D3D11_BUFFER_DESC oldBufferDesc{};
			myBuffer->GetDesc(&oldBufferDesc);
			context->CopySubresourceRegion(tempBuffer.Get(), 0, 0, 0, 0, myBuffer.Get(), 0, nullptr);
		}

		myBuffer = tempBuffer;

		if (!myShaderWriteable)
		{
			D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc{};
			srvDesc.Format = DXGI_FORMAT_UNKNOWN;
			srvDesc.ViewDimension = D3D11_SRV_DIMENSION_BUFFER;
			srvDesc.Buffer.FirstElement = 0;
			srvDesc.Buffer.NumElements = myMaxCount;

			VT_DX_CHECK(device->CreateShaderResourceView(myBuffer.Get(), &srvDesc, mySRV.GetAddressOf()));
		}
		else
		{
			D3D11_UNORDERED_ACCESS_VIEW_DESC uavDesc{};
			uavDesc.Format = DXGI_FORMAT_UNKNOWN;
			uavDesc.ViewDimension = D3D11_UAV_DIMENSION_BUFFER;
			uavDesc.Buffer.FirstElement = 0;
			uavDesc.Buffer.NumElements = myMaxCount;

			VT_DX_CHECK(device->CreateUnorderedAccessView(myBuffer.Get(), &uavDesc, myUAV.GetAddressOf()));
		}
	}
}