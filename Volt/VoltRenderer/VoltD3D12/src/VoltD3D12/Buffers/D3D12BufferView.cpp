#include "dxpch.h"
#include "D3D12BufferView.h"

#include "VoltD3D12/Descriptors/DescriptorUtility.h"
#include "VoltD3D12/Buffers/D3D12StorageBuffer.h"

#include <VoltRHI/RHIProxy.h>

#include <CoreUtilities/EnumUtils.h>

namespace Volt::RHI
{
	D3D12BufferView::D3D12BufferView(const BufferViewSpecification& specification)
		: m_resource(specification.bufferResource)
	{
		if (specification.bufferResource->GetType() == ResourceType::StorageBuffer)
		{
			m_viewType = D3D12ViewType::SRV | D3D12ViewType::UAV;
			CreateSRV();

			const auto bufferMemoryUsage = specification.bufferResource->AsRef<D3D12StorageBuffer>().GetMemoryUsage();
			
			if (EnumValueContainsFlag(bufferMemoryUsage, MemoryUsage::GPU))
			{
				CreateUAV();
			}
		}
		else if (specification.bufferResource->GetType() == ResourceType::UniformBuffer)
		{
			m_viewType = D3D12ViewType::CBV;
			CreateCBV();
		}
	}

	D3D12BufferView::~D3D12BufferView()
	{
		RHIProxy::GetInstance().DestroyResource([srvDescriptor = m_srvDescriptor, uavDescriptor = m_uavDescriptor, cbvDescriptor = m_cbvDescriptor]() 
		{
			if (srvDescriptor.IsValid())
			{
				DescriptorUtility::FreeDescriptorPointer(srvDescriptor);
			}

			if (uavDescriptor.IsValid())
			{
				DescriptorUtility::FreeDescriptorPointer(uavDescriptor);
			}

			if (cbvDescriptor.IsValid())
			{
				DescriptorUtility::FreeDescriptorPointer(cbvDescriptor);
			}
		});
	}

	const uint64_t D3D12BufferView::GetDeviceAddress() const
	{
		return 0;
	}

	void* D3D12BufferView::GetHandleImpl() const
	{
		return nullptr;
	}
	
	void D3D12BufferView::CreateSRV()
	{
		size_t elementSize = 0;
		uint32_t elementCount = 0;
	
		D3D12_BUFFER_SRV_FLAGS flags = D3D12_BUFFER_SRV_FLAG_NONE;
		D3D12_SHADER_RESOURCE_VIEW_DESC viewDesc{};

		if (m_resource->GetType() == ResourceType::StorageBuffer)
		{
			auto& storageBuffer = m_resource->AsRef<StorageBuffer>();
			elementSize = 0; // If it's a raw buffer the element size is always 4 //storageBuffer.GetElementSize();
			elementCount = static_cast<uint32_t>(storageBuffer.GetByteSize() / 4);

			flags = D3D12_BUFFER_SRV_FLAG_RAW;
			viewDesc.Format = DXGI_FORMAT_R32_TYPELESS;
		}

		viewDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
		viewDesc.ViewDimension = D3D12_SRV_DIMENSION_BUFFER;
		viewDesc.Buffer.FirstElement = 0;
		viewDesc.Buffer.Flags = flags;
		viewDesc.Buffer.NumElements = elementCount;
		viewDesc.Buffer.StructureByteStride = static_cast<uint32_t>(elementSize);

		m_srvDescriptor = DescriptorUtility::AllocateDescriptorPointer(D3D12DescriptorType::CBV_SRV_UAV);

		auto* devicePtr = GraphicsContext::GetDevice()->GetHandle<ID3D12Device2*>();
		devicePtr->CreateShaderResourceView(m_resource->GetHandle<ID3D12Resource*>(), &viewDesc, D3D12_CPU_DESCRIPTOR_HANDLE(m_srvDescriptor.GetCPUPointer()));
	}
	
	void D3D12BufferView::CreateUAV()
	{
		size_t elementSize = 0;
		uint32_t elementCount = 0;

		D3D12_UNORDERED_ACCESS_VIEW_DESC viewDesc{};
		viewDesc.Buffer.Flags = D3D12_BUFFER_UAV_FLAG_NONE;

		if (m_resource->GetType() == ResourceType::StorageBuffer)
		{
			auto& storageBuffer = m_resource->AsRef<StorageBuffer>();
			elementSize = 0; // If it's a raw buffer the element size is always 4 //storageBuffer.GetElementSize();
			elementCount = static_cast<uint32_t>(storageBuffer.GetByteSize() / 4);

			viewDesc.Buffer.Flags = D3D12_BUFFER_UAV_FLAG_RAW;
			viewDesc.Format = DXGI_FORMAT_R32_TYPELESS;
		}

		viewDesc.ViewDimension = D3D12_UAV_DIMENSION_BUFFER;
		viewDesc.Buffer.CounterOffsetInBytes = 0;
		viewDesc.Buffer.FirstElement = 0;
		viewDesc.Buffer.NumElements = elementCount;
		viewDesc.Buffer.StructureByteStride = static_cast<uint32_t>(elementSize);

		m_uavDescriptor = DescriptorUtility::AllocateDescriptorPointer(D3D12DescriptorType::CBV_SRV_UAV);
		auto* devicePtr = GraphicsContext::GetDevice()->GetHandle<ID3D12Device2*>();
		devicePtr->CreateUnorderedAccessView(m_resource->GetHandle<ID3D12Resource*>(), nullptr, &viewDesc, D3D12_CPU_DESCRIPTOR_HANDLE(m_uavDescriptor.GetCPUPointer()));
	}

	void D3D12BufferView::CreateCBV()
	{
		// #TODO_Ivar: Implement
	}
}
