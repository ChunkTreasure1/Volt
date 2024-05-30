#include "vkpch.h"
#include "VulkanVertexBuffer.h"

#include "VoltVulkan/Common/VulkanFunctions.h"

#include <VoltRHI/Buffers/CommandBuffer.h>

#include <VoltRHI/Graphics/GraphicsContext.h>
#include <VoltRHI/Graphics/GraphicsDevice.h>

#include <VoltRHI/Memory/Allocation.h>
#include <VoltRHI/RHIProxy.h>

namespace Volt::RHI
{
	VulkanVertexBuffer::VulkanVertexBuffer(const uint32_t size, const void* data)
	{
		Invalidate(data, size);
	}

	VulkanVertexBuffer::~VulkanVertexBuffer()
	{
		if (!m_allocation)
		{
			return;
		}

		RHIProxy::GetInstance().DestroyResource([allocation = m_allocation]()
		{
			GraphicsContext::GetDefaultAllocator().DestroyBuffer(allocation);
		});

		m_allocation = nullptr;
	}

	void VulkanVertexBuffer::SetData(const void* data, uint32_t size)
	{
		// #TODO_Ivar: Implement
	}

	void VulkanVertexBuffer::SetName(std::string_view name)
	{
		if (!Volt::RHI::vkSetDebugUtilsObjectNameEXT)
		{
			return;
		}

		VkDebugUtilsObjectNameInfoEXT nameInfo{};
		nameInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT;
		nameInfo.objectType = VK_OBJECT_TYPE_BUFFER;
		nameInfo.objectHandle = (uint64_t)m_allocation->GetResourceHandle<VkBuffer>();
		nameInfo.pObjectName = name.data();

		auto device = GraphicsContext::GetDevice();
		Volt::RHI::vkSetDebugUtilsObjectNameEXT(device->GetHandle<VkDevice>(), &nameInfo);
	}

	const uint64_t VulkanVertexBuffer::GetDeviceAddress() const
	{
		return m_allocation->GetDeviceAddress();
	}

	const uint64_t VulkanVertexBuffer::GetByteSize() const
	{
		return m_allocation->GetSize();
	}

	void* VulkanVertexBuffer::GetHandleImpl() const
	{
		return m_allocation->GetResourceHandle<VkBuffer>();
	}

	void VulkanVertexBuffer::Invalidate(const void* data, const uint32_t size)
	{
		VkDeviceSize bufferSize = size;
	
		RefPtr<Allocation> stagingAllocation;

		if (m_allocation)
		{
			RHIProxy::GetInstance().DestroyResource([allocation = m_allocation]()
			{
				GraphicsContext::GetDefaultAllocator().DestroyBuffer(allocation);
			});

			m_allocation = nullptr;
		}

		auto& allocator = GraphicsContext::GetDefaultAllocator();

		if (data != nullptr)
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
			m_allocation = allocator.CreateBuffer(bufferSize, BufferUsage::VertexBuffer | BufferUsage::TransferDst);
		}

		if (data)
		{
			// Copy from staging buffer to GPU buffer
			{
				RefPtr<CommandBuffer> cmdBuffer = CommandBuffer::Create();
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
