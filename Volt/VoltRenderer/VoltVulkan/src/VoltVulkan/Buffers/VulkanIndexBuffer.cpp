#include "vkpch.h"
#include "VulkanIndexBuffer.h"

#include "VoltVulkan/Common/VulkanFunctions.h"

#include <VoltRHI/Buffers/CommandBuffer.h>

#include <VoltRHI/Graphics/GraphicsContext.h>
#include <VoltRHI/Graphics/GraphicsDevice.h>

#include <VoltRHI/Memory/Allocation.h>

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
		if (!m_allocation)
		{
			return;
		}

		GraphicsContext::DestroyResource([allocation = m_allocation]() 
		{
			GraphicsContext::GetDefaultAllocator().DestroyBuffer(allocation);
		});

		m_allocation = nullptr;
	}

	const uint32_t VulkanIndexBuffer::GetCount() const
	{
		return m_count;
	}

	void VulkanIndexBuffer::SetName(std::string_view name)
	{
		VkDebugUtilsObjectNameInfoEXT nameInfo{};
		nameInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT;
		nameInfo.objectType = VK_OBJECT_TYPE_BUFFER;
		nameInfo.objectHandle = (uint64_t)m_allocation->GetResourceHandle<VkBuffer>();
		nameInfo.pObjectName = name.data();

		auto device = GraphicsContext::GetDevice();
		vkSetDebugUtilsObjectNameEXT(device->GetHandle<VkDevice>(), &nameInfo);
	}

	void* VulkanIndexBuffer::GetHandleImpl() const
	{
		return m_allocation->GetResourceHandle<VkBuffer>();
	}

	void VulkanIndexBuffer::SetData(const void* data, const uint32_t size)
	{
		VkDeviceSize bufferSize = size;

		Ref<Allocation> stagingAllocation;

		if (m_allocation)
		{
			GraphicsContext::DestroyResource([allocation = m_allocation]() 
			{
				GraphicsContext::GetDefaultAllocator().DestroyBuffer(allocation);
			});

			m_allocation = nullptr;
		}

		auto& allocator = GraphicsContext::GetDefaultAllocator();

		if (data)
		{
			stagingAllocation = allocator.CreateBuffer(bufferSize, BufferUsage::TransferSrc, MemoryUsage::CPU);

			// Copy to staging buffer
			{
				void* buffData = stagingAllocation->Map<void>();
				memcpy_s(buffData, size, data, size);
				stagingAllocation->Unmap();
			}
		}

		// Create GPU buffer
		{
			m_allocation = allocator.CreateBuffer(bufferSize, BufferUsage::IndexBuffer | BufferUsage::TransferDst);
		}

		if (data)
		{
			// Copy from staging buffer to GPU buffer
			{
				Ref<CommandBuffer> cmdBuffer = CommandBuffer::Create();
				cmdBuffer->Begin();

				VkBufferCopy copy{};
				copy.srcOffset = 0;
				copy.dstOffset = 0;
				copy.size = bufferSize;

				vkCmdCopyBuffer(cmdBuffer->GetHandle<VkCommandBuffer>(), stagingAllocation->GetResourceHandle<VkBuffer>(), m_allocation->GetResourceHandle<VkBuffer>(), 1, &copy);

				cmdBuffer->End();
				cmdBuffer->Execute();
			}

			allocator.DestroyBuffer(stagingAllocation);
		}
	}
}
