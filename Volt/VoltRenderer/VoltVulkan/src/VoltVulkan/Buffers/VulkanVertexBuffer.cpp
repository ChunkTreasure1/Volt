#include "vkpch.h"
#include "VulkanVertexBuffer.h"

#include "VoltVulkan/Graphics/VulkanAllocator.h"
#include "VoltVulkan/Common/VulkanFunctions.h"

#include <VoltRHI/Buffers/CommandBuffer.h>

#include <VoltRHI/Graphics/GraphicsContext.h>
#include <VoltRHI/Graphics/GraphicsDevice.h>

namespace Volt::RHI
{
	VulkanVertexBuffer::VulkanVertexBuffer(const void* data, const uint32_t size)
	{
		Invalidate(data, size);
	}

	VulkanVertexBuffer::~VulkanVertexBuffer()
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

	void VulkanVertexBuffer::SetData(const void* data, uint32_t size)
	{
		// #TODO_Ivar: Implement
	}

	void VulkanVertexBuffer::SetName(std::string_view name)
	{
		VkDebugUtilsObjectNameInfoEXT nameInfo{};
		nameInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT;
		nameInfo.objectType = VK_OBJECT_TYPE_BUFFER;
		nameInfo.objectHandle = (uint64_t)m_buffer;
		nameInfo.pObjectName = name.data();

		auto device = GraphicsContext::GetDevice();
		vkSetDebugUtilsObjectNameEXT(device->GetHandle<VkDevice>(), &nameInfo);
	}

	void* VulkanVertexBuffer::GetHandleImpl()
	{
		return m_buffer;
	}

	void VulkanVertexBuffer::Invalidate(const void* data, const uint32_t size)
	{
		VkBuffer stagingBuffer = nullptr;
		VmaAllocation stagingAllocation = nullptr;
		VkDeviceSize bufferSize = size;
		VulkanAllocator allocator{};

		if (m_buffer != VK_NULL_HANDLE)
		{
			allocator.DestroyBuffer(m_buffer, m_allocation);
			m_buffer = nullptr;
			m_allocation = nullptr;
		}

		if (data != nullptr)
		{
			// Create staging buffer
			{
				VkBufferCreateInfo bufferInfo{};
				bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
				bufferInfo.size = bufferSize;
				bufferInfo.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
				bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

				stagingAllocation = allocator.AllocateBuffer(bufferInfo, VMA_MEMORY_USAGE_CPU_ONLY, stagingBuffer);
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
			VkBufferCreateInfo bufferInfo{};
			bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
			bufferInfo.size = bufferSize;
			bufferInfo.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_BUILD_INPUT_READ_ONLY_BIT_KHR | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT;
			bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

			m_allocation = allocator.AllocateBuffer(bufferInfo, VMA_MEMORY_USAGE_GPU_ONLY, m_buffer);
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
