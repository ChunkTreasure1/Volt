#include "vtpch.h"
#include "ShaderStorageBuffer.h"

#include "Volt/Core/Graphics/GraphicsContext.h"

#include "Volt/Rendering/Renderer.h"

namespace Volt
{
	ShaderStorageBuffer::ShaderStorageBuffer(uint64_t elementSize, uint32_t elementCount, MemoryUsage usage)
		: myUsage(usage), myCurrentElementCount(elementCount), myElementSize(elementSize)
	{
		Resize(elementSize * elementCount);
	}

	ShaderStorageBuffer::~ShaderStorageBuffer()
	{
		Release();
	}

	void ShaderStorageBuffer::Resize(uint64_t newSize)
	{
		Release();

		mySize = newSize;

		auto device = GraphicsContext::GetDevice();
		const VkDeviceSize bufferSize = newSize;

		VulkanAllocator allocator{ "ShaderStorageBuffer - Create" };

		VkBufferCreateInfo info{};
		info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
		info.size = bufferSize;
		info.usage = VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;
		info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

		if ((myUsage & MemoryUsage::Indirect) != MemoryUsage::None)
		{
			info.usage |= VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT;
			info.usage |= VK_BUFFER_USAGE_TRANSFER_DST_BIT;
		}

		VmaAllocationCreateFlags createFlags = 0;

		if ((myUsage & MemoryUsage::CPUToGPU) != MemoryUsage::None)
		{
			createFlags = VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT | VMA_ALLOCATION_CREATE_MAPPED_BIT;
		}

		myAllocation = allocator.AllocateBuffer(info, createFlags, myBuffer);
	}

	void ShaderStorageBuffer::ResizeWithElementCount(uint32_t newElementCount)
	{
		const VkDeviceSize newSize = newElementCount * myElementSize;

		Release();

		myCurrentElementCount = newElementCount;
		mySize = newSize;

		auto device = GraphicsContext::GetDevice();
		const VkDeviceSize bufferSize = newSize;

		VulkanAllocator allocator{ "ShaderStorageBuffer - Create" };

		VkBufferCreateInfo info{};
		info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
		info.size = bufferSize;
		info.usage = VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;
		info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

		if ((myUsage & MemoryUsage::Indirect) != MemoryUsage::None)
		{
			info.usage |= VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT;
			info.usage |= VK_BUFFER_USAGE_TRANSFER_DST_BIT;
		}

		VmaMemoryUsage memUsage = VMA_MEMORY_USAGE_GPU_ONLY;

		if ((myUsage & MemoryUsage::CPUToGPU) != MemoryUsage::None)
		{
			memUsage = VMA_MEMORY_USAGE_CPU_TO_GPU;
		}

		myAllocation = allocator.AllocateBuffer(info, memUsage, myBuffer);
	}

	void ShaderStorageBuffer::Unmap()
	{
		VulkanAllocator allocator;
		allocator.UnmapMemory(myAllocation);
	}

	Ref<ShaderStorageBuffer> ShaderStorageBuffer::Create(uint64_t elementSize, uint32_t elementCount, MemoryUsage usage)
	{
		return CreateRef<ShaderStorageBuffer>(elementSize, elementCount, usage);
	}

	void ShaderStorageBuffer::Release()
	{
		if (myBuffer == VK_NULL_HANDLE)
		{
			return;
		}

		Renderer::SubmitResourceChange([buffer = myBuffer, allocation = myAllocation]()
		{
			VulkanAllocator allocator{ "ShaderStorageBuffer - Destroy" };
			allocator.DestroyBuffer(buffer, allocation);
		});

		myBuffer = nullptr;
		myAllocation = nullptr;
	}
}
