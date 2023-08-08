#include "vkpch.h"
#include "VulkanIndexBuffer.h"

#include "VoltVulkan/Graphics/VulkanAllocator.h"

#include <VoltRHI/Buffers/CommandBuffer.h>

namespace Volt::RHI
{
	VulkanIndexBuffer::VulkanIndexBuffer(std::span<uint32_t> indices)
		: m_count(static_cast<uint32_t>(indices.size()))
	{
		SetData(indices.data(), static_cast<uint32_t>(sizeof(uint32_t) * indices.size()));
	}

	VulkanIndexBuffer::VulkanIndexBuffer(const uint32_t* indices, const uint32_t count)
		: m_count(count)
	{
		SetData(indices, sizeof(uint32_t) * count);
	}

	VulkanIndexBuffer::~VulkanIndexBuffer()
	{
		if (!m_buffer)
		{
			return;
		}

		// #TODO_Ivar: Move to deletion queue

		VulkanAllocator allocator{};
		allocator.DestroyBuffer(m_buffer, m_allocation);

		m_buffer = nullptr;
		m_allocation = nullptr;
	}

	const uint32_t VulkanIndexBuffer::GetCount() const
	{
		return m_count;
	}

	void* VulkanIndexBuffer::GetHandleImpl()
	{
		return m_buffer;
	}

	void VulkanIndexBuffer::SetData(const void* data, const uint32_t size)
	{
		VkBuffer stagingBuffer = nullptr;
		VmaAllocation stagingAllocation = nullptr;

		VkDeviceSize bufferSize = size;
		VulkanAllocator allocator{};

		if (m_buffer != nullptr)
		{
			allocator.DestroyBuffer(m_buffer, m_allocation);
			m_buffer = nullptr;
			m_allocation = nullptr;
		}

		if (data != nullptr)
		{
			// Create staging buffer
			{
				VkBufferCreateInfo info{};
				info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
				info.size = bufferSize;
				info.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
				info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

				stagingAllocation = allocator.AllocateBuffer(info, VMA_MEMORY_USAGE_CPU_ONLY, stagingBuffer);
			}

			// Copy to staging buffer
			{
				void* buffData = allocator.MapMemory<void>(stagingAllocation);
				memcpy_s(buffData, size, data, size);
				allocator.UnmapMemory(stagingAllocation);
			}
		}

		// Create GPU buffer
		{
			VkBufferCreateInfo info{};
			info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
			info.size = bufferSize;
			info.usage = VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT;
			info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

			m_allocation = allocator.AllocateBuffer(info, VMA_MEMORY_USAGE_GPU_ONLY, m_buffer);
		}

		if (data != nullptr)
		{
			// Copy from staging buffer to GPU buffer
			{
				Ref<CommandBuffer> cmdBuffer = CommandBuffer::Create();
				cmdBuffer->Begin();

				VkBufferCopy copy{};
				copy.srcOffset = 0;
				copy.dstOffset = 0;
				copy.size = bufferSize;

				vkCmdCopyBuffer(cmdBuffer->GetHandle<VkCommandBuffer>(), stagingBuffer, m_buffer, 1, &copy);

				cmdBuffer->End();
				cmdBuffer->Execute();
			}

			allocator.DestroyBuffer(stagingBuffer, stagingAllocation);
		}
	}
}
