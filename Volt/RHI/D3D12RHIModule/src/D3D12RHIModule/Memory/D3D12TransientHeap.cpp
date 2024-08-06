#include "dxpch.h"
#include "D3D12TransientHeap.h"

#include "D3D12RHIModule/Memory/D3D12Allocation.h"
#include "D3D12RHIModule/Common/D3D12Helpers.h"

#include <RHIModule/Memory/MemoryUtility.h>

namespace Volt::RHI
{
	D3D12TransientHeap::D3D12TransientHeap(const TransientHeapCreateInfo& info)
		: m_createInfo(info)
	{
		if ((info.flags & TransientHeapFlags::AllowBuffers) != TransientHeapFlags::None)
		{
			InitializeAsBufferHeap();
		}
		else if ((info.flags & TransientHeapFlags::AllowTextures) != TransientHeapFlags::None || (info.flags & TransientHeapFlags::AllowRenderTargets) != TransientHeapFlags::None)
		{
			InitializeAsImageHeap();
		}
	}

	D3D12TransientHeap::~D3D12TransientHeap()
	{
		for (auto& page : m_pageAllocations)
		{
			if (page.handle)
			{
				ID3D12Heap* heap = static_cast<ID3D12Heap*>(page.handle);
				heap->Release();
			}
		}
	}
	
	RefPtr<Allocation> D3D12TransientHeap::CreateBuffer(const TransientBufferCreateInfo& createInfo)
	{
		VT_PROFILE_FUNCTION();
		VT_ENSURE((m_createInfo.flags & TransientHeapFlags::AllowBuffers) != TransientHeapFlags::None);

		auto device = GraphicsContext::GetDevice()->GetHandle<ID3D12Device10*>();

		auto [pageIndex, blockAlloc] = FindNextAvailableBlock(createInfo.size);

		if (blockAlloc.size == 0)
		{
			VT_LOGC(LogVerbosity::Error, LogD3D12RHI, "Unable to find available allocation block for buffer allocation of size {0}!", createInfo.size);
			return nullptr;
		}

		D3D12_RESOURCE_DESC1 resourceDesc{};
		resourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
		resourceDesc.Alignment = 0;
		resourceDesc.Width = createInfo.size;
		resourceDesc.Height = 1;
		resourceDesc.DepthOrArraySize = 1;
		resourceDesc.MipLevels = 1;
		resourceDesc.Format = DXGI_FORMAT_UNKNOWN;
		resourceDesc.SampleDesc = { 1, 0 };
		resourceDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
		resourceDesc.Flags = D3D12_RESOURCE_FLAG_NONE;

		if ((createInfo.usage & BufferUsage::StorageBuffer) != BufferUsage::None)
		{
			resourceDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS;
		}

		const auto& page = m_pageAllocations.at(pageIndex);

		ID3D12Resource* resource = nullptr;
		device->CreatePlacedResource2(static_cast<ID3D12Heap*>(page.handle), blockAlloc.offset, &resourceDesc, D3D12_BARRIER_LAYOUT_UNDEFINED, nullptr, 0, nullptr, VT_D3D12_ID(resource));

		RefPtr<D3D12TransientBufferAllocation> bufferAlloc = RefPtr<D3D12TransientBufferAllocation>::Create(createInfo.hash);
		bufferAlloc->m_resource = resource;
		bufferAlloc->m_allocationBlock = blockAlloc;
		bufferAlloc->m_heapId = m_heapId;
		bufferAlloc->m_size = createInfo.size;

		return bufferAlloc;
	}
	
	RefPtr<Allocation> D3D12TransientHeap::CreateImage(const TransientImageCreateInfo& createInfo)
	{
		VT_PROFILE_FUNCTION();
		VT_ENSURE((m_createInfo.flags & TransientHeapFlags::AllowTextures) != TransientHeapFlags::None);

		auto device = GraphicsContext::GetDevice()->GetHandle<ID3D12Device10*>();
		auto [pageIndex, blockAlloc] = FindNextAvailableBlock(createInfo.size);

		if (blockAlloc.size == 0)
		{
			VT_LOGC(LogVerbosity::Error, LogD3D12RHI, "Unable to find available allocation block for image allocation of size {0}!", createInfo.size);
			return nullptr;
		}

		D3D12_RESOURCE_DESC1 resourceDesc = Utility::GetD3D12ResourceDesc(createInfo.imageSpecification);
		D3D12_BARRIER_LAYOUT initialLayout = D3D12_BARRIER_LAYOUT_COMMON;

		switch (createInfo.imageSpecification.usage)
		{
			case ImageUsage::Attachment:
			case ImageUsage::AttachmentStorage:
				if (Utility::IsDepthFormat(createInfo.imageSpecification.format))
				{
					initialLayout = D3D12_BARRIER_LAYOUT_DEPTH_STENCIL_WRITE;
				}
				else
				{
					initialLayout = D3D12_BARRIER_LAYOUT_RENDER_TARGET;
				}
				break;

			case ImageUsage::Storage:
				initialLayout = D3D12_BARRIER_LAYOUT_UNORDERED_ACCESS;
				break;

			case ImageUsage::Texture:
				initialLayout = D3D12_BARRIER_LAYOUT_COMMON;
				break;
		}

		const auto& page = m_pageAllocations.at(pageIndex);
		ID3D12Resource* resource = nullptr;
		device->CreatePlacedResource2(static_cast<ID3D12Heap*>(page.handle), blockAlloc.offset, &resourceDesc, initialLayout, nullptr, 0, nullptr, VT_D3D12_ID(resource));

		RefPtr<D3D12TransientImageAllocation> imageAlloc = RefPtr<D3D12TransientImageAllocation>::Create(createInfo.hash);
		imageAlloc->m_resource = resource;
		imageAlloc->m_allocationBlock = blockAlloc;
		imageAlloc->m_heapId = m_heapId;
		imageAlloc->m_size = createInfo.size;

		return imageAlloc;
	}
	
	void D3D12TransientHeap::ForfeitBuffer(RefPtr<Allocation> allocation)
	{
		VT_PROFILE_FUNCTION();

		RefPtr<D3D12TransientBufferAllocation> bufferAlloc = allocation.As<D3D12TransientBufferAllocation>();
		if (!bufferAlloc)
		{
			return;
		}

		auto device = GraphicsContext::GetDevice();

		bufferAlloc->m_resource->Release();
		AllocationBlock allocBlock = bufferAlloc->m_allocationBlock;
		ForfeitAllocationBlock(allocBlock);
	}
	
	void D3D12TransientHeap::ForfeitImage(RefPtr<Allocation> allocation)
	{
		VT_PROFILE_FUNCTION();

		RefPtr<D3D12TransientImageAllocation> imageAlloc = allocation.As<D3D12TransientImageAllocation>();
		if (!imageAlloc)
		{
			return;
		}

		auto device = GraphicsContext::GetDevice();

		{
			VT_PROFILE_SCOPE("D3D12 Destroy Image");
			imageAlloc->m_resource->Release();
		}

		AllocationBlock allocBlock = imageAlloc->m_allocationBlock;
		ForfeitAllocationBlock(allocBlock);
	}
	
	const bool D3D12TransientHeap::IsAllocationSupported(const uint64_t size, TransientHeapFlags heapFlags) const
	{
		if ((m_createInfo.flags & heapFlags) == TransientHeapFlags::None)
		{
			return false;
		}

		for (const auto& page : m_pageAllocations)
		{
			if (IsAllocationSupportedInPage(page, size))
			{
				return true;
			}
		}

		return false;
	}
	
	const UUID64 D3D12TransientHeap::GetHeapID() const
	{
		return m_heapId;
	}
	
	void* D3D12TransientHeap::GetHandleImpl() const
	{
		return nullptr;
	}
	
	std::pair<uint32_t, AllocationBlock> D3D12TransientHeap::FindNextAvailableBlock(const uint64_t size)
	{
		VT_PROFILE_FUNCTION();

		std::scoped_lock lock{ m_allocationMutex };

		PageAllocation* pageAllocation = nullptr;
		uint32_t pageIndex = 0;

		for (auto& page : m_pageAllocations)
		{
			if (IsAllocationSupportedInPage(page, size))
			{
				pageAllocation = &page;
				break;
			}

			pageIndex++;
		}

		if (!pageAllocation)
		{
			return { 0, { 0, 0 } };
		}

		int32_t blockIndex = -1;

		for (int32_t i = 0; const auto & availableBlocks : pageAllocation->availableBlocks)
		{
			if (availableBlocks.size >= size)
			{
				blockIndex = i;
				break;
			}

			i++;
		}

		AllocationBlock resultBlock{};

		if (blockIndex != -1)
		{
			AllocationBlock oldBlock = pageAllocation->availableBlocks.at(blockIndex);
			pageAllocation->availableBlocks.erase(pageAllocation->availableBlocks.begin() + blockIndex);

			// If we didn't use the whole block, create a new available block with the smaller allocation
			if (oldBlock.size > size)
			{
				auto& newBlock = pageAllocation->availableBlocks.emplace_back();
				newBlock.size = oldBlock.size - size;
				newBlock.offset = oldBlock.offset + size;
			}

			resultBlock.offset = oldBlock.offset;
			resultBlock.size = size;
		}
		else
		{
			resultBlock.offset = pageAllocation->tail;
			resultBlock.size = size;

			pageAllocation->tail += size;
		}

		resultBlock.pageId = pageIndex;
		pageAllocation->usedSize += size;

		return { pageIndex, resultBlock };
	}
	
	void D3D12TransientHeap::ForfeitAllocationBlock(const AllocationBlock& allocBlock)
	{
		VT_PROFILE_FUNCTION();

		std::scoped_lock lock{ m_allocationMutex };

		auto& pageAllocation = m_pageAllocations.at(allocBlock.pageId);

		uint64_t finalOffset = allocBlock.offset;
		uint64_t finalSize = allocBlock.size;

		Vector<size_t> blocksToMerge;

		for (size_t index = 0; const auto & block : pageAllocation.availableBlocks)
		{
			if (block.offset + block.size == finalOffset)
			{
				finalOffset = block.offset;
				finalSize += block.size;
				blocksToMerge.emplace_back(index);
			}
			else if (block.offset == finalOffset + finalSize)
			{
				finalSize += block.size;
				blocksToMerge.emplace_back(index);
			}

			index++;
		}

		for (int32_t i = static_cast<int32_t>(blocksToMerge.size()) - 1; i >= 0; i--)
		{
			pageAllocation.availableBlocks.erase(pageAllocation.availableBlocks.begin() + blocksToMerge.at(i));
			blocksToMerge.erase(blocksToMerge.begin() + i);
		}

		uint64_t finalEndOffset = finalOffset + finalSize;
		if (finalEndOffset == pageAllocation.tail)
		{
			pageAllocation.tail = finalOffset;
		}
		else
		{
			pageAllocation.availableBlocks.emplace_back(finalSize, finalOffset, 0);
		}

		pageAllocation.usedSize -= allocBlock.size;
	}

	const bool D3D12TransientHeap::IsAllocationSupportedInPage(const PageAllocation& page, const uint64_t size) const
	{
		if (page.GetRemainingSize() < size)
		{
			return false;
		}

		for (const auto& availBloc : page.availableBlocks)
		{
			if (availBloc.size >= size)
			{
				return true;
			}
		}

		if (page.GetRemainingTailSize() >= size)
		{
			return true;
		}

		return false;
	}
	
	void D3D12TransientHeap::InitializeAsBufferHeap()
	{
		m_memoryRequirements.size = m_createInfo.pageSize;
		m_memoryRequirements.alignment = D3D12_DEFAULT_RESOURCE_PLACEMENT_ALIGNMENT;
		m_memoryRequirements.memoryTypeBits = 0;

		m_memoryRequirements.size = Utility::Align(m_createInfo.pageSize, m_memoryRequirements.alignment);
	
		D3D12_HEAP_DESC heapDesc{};
		heapDesc.SizeInBytes = m_memoryRequirements.size;
		heapDesc.Alignment = m_memoryRequirements.alignment;
		
		heapDesc.Properties.Type = D3D12_HEAP_TYPE_DEFAULT;
		heapDesc.Properties.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
		heapDesc.Properties.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
		heapDesc.Properties.CreationNodeMask = 0;
		heapDesc.Properties.VisibleNodeMask = 0;

		heapDesc.Flags = D3D12_HEAP_FLAG_ALLOW_ONLY_BUFFERS;

		ID3D12Device2* d3d12Device = GraphicsContext::GetDevice()->GetHandle<ID3D12Device2*>();

		for (uint32_t i = 0; i < MAX_PAGE_COUNT; i++)
		{
			m_pageAllocations[i].size = m_memoryRequirements.size;
			m_pageAllocations[i].alignment = m_memoryRequirements.alignment;
			
			ID3D12Heap* heap = nullptr;
			VT_D3D12_CHECK(d3d12Device->CreateHeap(&heapDesc, VT_D3D12_ID(heap)));

			m_pageAllocations[i].handle = heap;
		}
	}

	void D3D12TransientHeap::InitializeAsImageHeap()
	{
		m_memoryRequirements.size = m_createInfo.pageSize;
		m_memoryRequirements.alignment = D3D12_DEFAULT_RESOURCE_PLACEMENT_ALIGNMENT;
		m_memoryRequirements.memoryTypeBits = 0;

		m_memoryRequirements.size = Utility::Align(m_createInfo.pageSize, m_memoryRequirements.alignment);

		D3D12_HEAP_DESC heapDesc{};
		heapDesc.SizeInBytes = m_memoryRequirements.size;
		heapDesc.Alignment = m_memoryRequirements.alignment;

		heapDesc.Properties.Type = D3D12_HEAP_TYPE_DEFAULT;
		heapDesc.Properties.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
		heapDesc.Properties.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
		heapDesc.Properties.CreationNodeMask = 0;
		heapDesc.Properties.VisibleNodeMask = 0;

		heapDesc.Flags = D3D12_HEAP_FLAG_ALLOW_ALL_BUFFERS_AND_TEXTURES;

		ID3D12Device2* d3d12Device = GraphicsContext::GetDevice()->GetHandle<ID3D12Device2*>();

		for (uint32_t i = 0; i < MAX_PAGE_COUNT; i++)
		{
			m_pageAllocations[i].size = m_memoryRequirements.size;
			m_pageAllocations[i].alignment = m_memoryRequirements.alignment;

			ID3D12Heap* heap = nullptr;
			VT_D3D12_CHECK(d3d12Device->CreateHeap(&heapDesc, VT_D3D12_ID(heap)));

			m_pageAllocations[i].handle = heap;
		}
	}
}
