#include "vkpch.h"
#include "VulkanConstantBuffer.h"

#include "VoltVulkan/Graphics/VulkanAllocator.h"

#include <VoltRHI/Buffers/BufferView.h>

namespace Volt::RHI
{
	VulkanConstantBuffer::VulkanConstantBuffer(const uint32_t size, const void* data)
		: m_size(size)
	{
		const VkDeviceSize bufferSize = static_cast<VkDeviceSize>(size);
		VulkanAllocator allocator{};

		VkBufferCreateInfo info{};
		info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
		info.size = bufferSize;
		info.usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
		info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

		m_allocation = allocator.AllocateBuffer(info, VMA_MEMORY_USAGE_CPU_TO_GPU, m_buffer);

		if (data)
		{
			SetData(data, size);
		}
	}

	VulkanConstantBuffer::~VulkanConstantBuffer()
	{
		if (m_buffer == nullptr)
		{
			return;
		}

		// #TODO_Ivar: move to deletion queue
		VulkanAllocator allocator{};
		allocator.DestroyBuffer(m_buffer, m_allocation);
	}

	Ref<BufferView> VulkanConstantBuffer::GetView()
	{
		BufferViewSpecification spec{};
		spec.bufferResource = As<VulkanConstantBuffer>();

		return BufferView::Create(spec);
	}

	const uint32_t VulkanConstantBuffer::GetSize() const
	{
		return m_size;
	}

	void VulkanConstantBuffer::SetData(const void* data, const uint32_t size)
	{
		VulkanAllocator allocator{};
		void* bufferData = allocator.MapMemory<void*>(m_allocation);

		memcpy_s(bufferData, m_size, data, size);

		allocator.UnmapMemory(m_allocation);
	}

	void VulkanConstantBuffer::Unmap()
	{
		VulkanAllocator allocator{};
		allocator.UnmapMemory(m_allocation);
	}

	void* VulkanConstantBuffer::MapInternal()
	{
		VulkanAllocator allocator{};

		return allocator.MapMemory<void*>(m_allocation);
	}

	void* VulkanConstantBuffer::GetHandleImpl()
	{
		return m_buffer;
	}
}
