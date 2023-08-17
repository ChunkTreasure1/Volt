#include "vkpch.h"
#include "VulkanStorageBuffer.h"

#include "VoltVulkan/Graphics/VulkanAllocator.h"
#include "VoltVulkan/Common/VulkanFunctions.h"

#include <VoltRHI/Graphics/GraphicsContext.h>
#include <VoltRHI/Graphics/GraphicsDevice.h>
#include <VoltRHI/Buffers/BufferView.h>
#include <VoltRHI/Buffers/CommandBuffer.h>

namespace Volt::RHI
{
	VulkanStorageBuffer::VulkanStorageBuffer(const uint32_t count, const size_t elementSize, MemoryUsage bufferUsage)
		: m_byteSize(count * elementSize), m_size(count), m_elementSize(elementSize), m_memoryUsage(bufferUsage)
	{
		Invalidate(elementSize * count);
	}
	
	VulkanStorageBuffer::~VulkanStorageBuffer()
	{
		Release();
	}

	void VulkanStorageBuffer::ResizeByteSize(const size_t byteSize)
	{
		Invalidate(byteSize);
	}
	
	void VulkanStorageBuffer::Resize(const uint32_t size)
	{
		const size_t newSize = size * m_elementSize;
		Invalidate(newSize);
	}
	
	const size_t VulkanStorageBuffer::GetByteSize() const
	{
		return m_byteSize;
	}
	
	const uint32_t VulkanStorageBuffer::GetSize() const
	{
		return m_size;
	}
	
	void VulkanStorageBuffer::Unmap()
	{
		VulkanAllocator allocator{};
		allocator.UnmapMemory(m_allocation);
	}

	void VulkanStorageBuffer::SetData(const void* data, const size_t size)
	{
		Ref<StorageBuffer> stagingBuffer = StorageBuffer::Create(1, 1, MemoryUsage::Staging);
		stagingBuffer->ResizeByteSize(size);

		void* mappedPtr = stagingBuffer->Map<void>();
		memcpy_s(mappedPtr, m_byteSize, data, size);
		stagingBuffer->Unmap();

		Ref<CommandBuffer> cmdBuffer = CommandBuffer::Create();
		cmdBuffer->Begin();

		cmdBuffer->CopyBufferRegion(stagingBuffer, 0, As<VulkanStorageBuffer>(), 0, size);

		cmdBuffer->End();
		cmdBuffer->Execute();
	}

	Ref<BufferView> VulkanStorageBuffer::GetView()
	{
		BufferViewSpecification spec{};
		spec.bufferResource = As<StorageBuffer>();

		Ref<BufferView> bufferView = BufferView::Create(spec);
		return bufferView;
	}

	void VulkanStorageBuffer::SetName(std::string_view name)
	{
		VkDebugUtilsObjectNameInfoEXT nameInfo{};
		nameInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT;
		nameInfo.objectType = VK_OBJECT_TYPE_BUFFER;
		nameInfo.objectHandle = (uint64_t)m_buffer;
		nameInfo.pObjectName = name.data();

		auto device = GraphicsContext::GetDevice();
		vkSetDebugUtilsObjectNameEXT(device->GetHandle<VkDevice>(), &nameInfo);
	}
	
	void* VulkanStorageBuffer::GetHandleImpl()
	{
		return m_buffer;
	}
	
	void* VulkanStorageBuffer::MapInternal()
	{
		VulkanAllocator allocator{};
		return allocator.MapMemory<void*>(m_allocation);
	}

	void VulkanStorageBuffer::Invalidate(const size_t byteSize)
	{
		Release();
		m_byteSize = byteSize;

		const VkDeviceSize bufferSize = byteSize;
		const bool isStagingBuffer = (m_memoryUsage & MemoryUsage::Staging) != MemoryUsage::Default;

		VkBufferCreateInfo info{};
		info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
		info.size = bufferSize;
		info.usage = isStagingBuffer ? VK_BUFFER_USAGE_TRANSFER_SRC_BIT : VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT;
		info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

		if ((m_memoryUsage & MemoryUsage::Indirect) != MemoryUsage::Default)
		{
			info.usage |= VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT;
			info.usage |= VK_BUFFER_USAGE_TRANSFER_DST_BIT;
		}

		VmaMemoryUsage usage = VMA_MEMORY_USAGE_GPU_ONLY;

		if ((m_memoryUsage & MemoryUsage::CPUToGPU) != MemoryUsage::Default || isStagingBuffer)
		{
			usage = VMA_MEMORY_USAGE_CPU_TO_GPU;
		}

		VulkanAllocator allocator{};
		m_allocation = allocator.AllocateBuffer(info, usage, m_buffer);
	}

	void VulkanStorageBuffer::Release()
	{
		if (!m_buffer)
		{
			return;
		}

		GraphicsContext::DestroyResource([buffer = m_buffer, allocation = m_allocation]() 
		{
			VulkanAllocator allocator{};
			allocator.DestroyBuffer(buffer, allocation);
		});

		m_buffer = nullptr;
		m_allocation = nullptr;
	}
}
