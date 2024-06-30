#include "dxpch.h"
#include "D3D12DefaultAllocator.h"

#include "VoltD3D12/Common/D3D12MemAlloc.h"
#include "VoltD3D12/Common/D3D12Helpers.h"
#include "VoltD3D12/Memory/D3D12Allocation.h"

#include <VoltRHI/Utility/HashUtility.h>
#include <VoltRHI/Images/ImageUtility.h>

namespace Volt::RHI
{
	D3D12DefaultAllocator::D3D12DefaultAllocator()
	{
		auto device = GraphicsContext::GetDevice()->GetHandle<ID3D12Device2*>();
		auto adapter = GraphicsContext::GetPhysicalDevice()->GetHandle<IDXGIAdapter4*>();

		D3D12MA::ALLOCATOR_DESC desc{};
		desc.pAdapter = adapter;
		desc.pDevice = device;
		desc.Flags = D3D12MA::ALLOCATOR_FLAG_NONE;

		VT_D3D12_CHECK(D3D12MA::CreateAllocator(&desc, &m_allocator));
	}

	D3D12DefaultAllocator::~D3D12DefaultAllocator()
	{
		VT_D3D12_DELETE(m_allocator);
	}

	RefPtr<Allocation> D3D12DefaultAllocator::CreateBuffer(const size_t size, BufferUsage usage, MemoryUsage memoryUsage)
	{
		VT_PROFILE_FUNCTION();

		const size_t hash = Utility::GetHashFromBufferSpec(size, usage, memoryUsage);

		{
			std::scoped_lock lock{ m_bufferAllocationMutex };
			if (auto buffer = m_allocationCache.TryGetBufferAllocationFromHash(hash))
			{
				return buffer;
			}
		}

		D3D12_RESOURCE_DESC resourceDesc{};
		resourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
		resourceDesc.Alignment = 0;
		resourceDesc.Width = size;
		resourceDesc.Height = 1;
		resourceDesc.DepthOrArraySize = 1;
		resourceDesc.MipLevels = 1;
		resourceDesc.Format = DXGI_FORMAT_UNKNOWN;
		resourceDesc.SampleDesc = { 1, 0 };
		resourceDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
		resourceDesc.Flags = D3D12_RESOURCE_FLAG_NONE;

		if ((memoryUsage & MemoryUsage::GPU) != MemoryUsage::None)
		{
			resourceDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS;
		}

		D3D12MA::ALLOCATION_DESC allocDesc{};
		allocDesc.HeapType = D3D12_HEAP_TYPE_DEFAULT;
		allocDesc.ExtraHeapFlags = D3D12_HEAP_FLAG_NONE;
		allocDesc.Flags = D3D12MA::ALLOCATION_FLAG_NONE;

		if ((memoryUsage & MemoryUsage::CPUToGPU) != MemoryUsage::None)
		{
			allocDesc.HeapType = D3D12_HEAP_TYPE_UPLOAD;
		}
		else if ((memoryUsage & MemoryUsage::GPUToCPU) != MemoryUsage::None)
		{
			allocDesc.HeapType = D3D12_HEAP_TYPE_READBACK;
		}

		if ((memoryUsage & MemoryUsage::Dedicated) != MemoryUsage::None)
		{
			allocDesc.Flags = D3D12MA::ALLOCATION_FLAG_COMMITTED;
		}

		RefPtr<D3D12BufferAllocation> allocation = RefPtr<D3D12BufferAllocation>::Create(hash);
		VT_D3D12_CHECK(m_allocator->CreateResource(&allocDesc, &resourceDesc, Utility::GetResourceStateFromUsage(usage), nullptr, &allocation->m_allocation, IID_PPV_ARGS(&allocation->m_resource)));

		allocation->m_size = size;

		{
			std::scoped_lock lock{ m_bufferAllocationMutex };
			m_activeBufferAllocations.push_back(allocation);
		}
		return allocation;
	}

	RefPtr<Allocation> D3D12DefaultAllocator::CreateImage(const ImageSpecification& imageSpecification, MemoryUsage memoryUsage)
	{
		VT_PROFILE_FUNCTION();

		const size_t hash = Utility::GetHashFromImageSpec(imageSpecification, memoryUsage);

		{
			std::scoped_lock lock{ m_imageAllocationMutex };
			if (auto buffer = m_allocationCache.TryGetImageAllocationFromHash(hash))
			{
				return buffer;
			}
		}

		D3D12_RESOURCE_DESC resourceDesc = Utility::GetD3D12ResourceDesc(imageSpecification);

		D3D12MA::ALLOCATION_DESC allocDesc{};
		allocDesc.HeapType = D3D12_HEAP_TYPE_DEFAULT;
		allocDesc.ExtraHeapFlags = D3D12_HEAP_FLAG_NONE;
		allocDesc.Flags = D3D12MA::ALLOCATION_FLAG_NONE;

		if ((memoryUsage & MemoryUsage::CPUToGPU) != MemoryUsage::None)
		{
			allocDesc.HeapType = D3D12_HEAP_TYPE_UPLOAD;
		}
		else if ((memoryUsage & MemoryUsage::GPUToCPU) != MemoryUsage::None)
		{
			allocDesc.HeapType = D3D12_HEAP_TYPE_READBACK;
		}

		if ((memoryUsage & MemoryUsage::Dedicated) != MemoryUsage::None)
		{
			allocDesc.Flags = D3D12MA::ALLOCATION_FLAG_COMMITTED;
		}

		RefPtr<D3D12ImageAllocation> allocation = RefPtr<D3D12ImageAllocation>::Create(hash);
		VT_D3D12_CHECK(m_allocator->CreateResource(&allocDesc, &resourceDesc, D3D12_RESOURCE_STATE_COMMON, nullptr, &allocation->m_allocation, IID_PPV_ARGS(&allocation->m_resource)));

		allocation->m_size = allocation->m_allocation->GetSize();

		{
			std::scoped_lock lock{ m_imageAllocationMutex };
			m_activeImageAllocations.push_back(allocation);
		}

		return allocation;
	}

	void D3D12DefaultAllocator::DestroyBuffer(RefPtr<Allocation> allocation)
	{
		m_allocationCache.QueueBufferAllocationForRemoval(allocation);
	}

	void D3D12DefaultAllocator::DestroyImage(RefPtr<Allocation> allocation)
	{
		m_allocationCache.QueueImageAllocationForRemoval(allocation);
	}

	void D3D12DefaultAllocator::Update()
	{
		const auto allocationsToRemove = m_allocationCache.UpdateAndGetAllocationsToDestroy();

		for (const auto& alloc : allocationsToRemove.bufferAllocations)
		{
			DestroyBufferInternal(alloc);
		}

		for (const auto& alloc : allocationsToRemove.imageAllocations)
		{
			DestroyImageInternal(alloc);
		}
	}

	void* D3D12DefaultAllocator::GetHandleImpl() const
	{
		return m_allocator;
	}

	void D3D12DefaultAllocator::DestroyBufferInternal(RefPtr<Allocation> allocation)
	{
		const D3D12ImageAllocation& imageAlloc = allocation->AsRef<D3D12ImageAllocation>();
		imageAlloc.m_allocation->Release();
		imageAlloc.m_resource->Release();

		std::scoped_lock lock{ m_bufferAllocationMutex };
		if (const auto it = std::ranges::find(m_activeBufferAllocations, allocation); it != m_activeBufferAllocations.end())
		{
			m_activeBufferAllocations.erase(it);
		}
	}

	void D3D12DefaultAllocator::DestroyImageInternal(RefPtr<Allocation> allocation)
	{
		const D3D12BufferAllocation& imageAlloc = allocation->AsRef<D3D12BufferAllocation>();
		imageAlloc.m_allocation->Release();
		imageAlloc.m_resource->Release();

		std::scoped_lock lock{ m_imageAllocationMutex };
		if (const auto it = std::ranges::find(m_activeImageAllocations, allocation); it != m_activeImageAllocations.end())
		{
			m_activeImageAllocations.erase(it);
		}
	}
}
