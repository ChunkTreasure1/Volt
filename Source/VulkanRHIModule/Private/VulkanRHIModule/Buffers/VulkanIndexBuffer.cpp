#include "vkpch.h"
#include "VulkanRHIModule/Buffers/VulkanIndexBuffer.h"

#include "VulkanRHIModule/Common/VulkanFunctions.h"

#include <RHIModule/Buffers/CommandBuffer.h>

#include <RHIModule/Graphics/GraphicsContext.h>
#include <RHIModule/Graphics/GraphicsDevice.h>

#include <RHIModule/Memory/Allocation.h>
#include <RHIModule/RHIProxy.h>

namespace Volt::RHI
{
	VulkanIndexBuffer::VulkanIndexBuffer(std::span<const uint32_t> indices)
		: m_count(static_cast<uint32_t>(indices.size()))
	{
		GraphicsContext::GetResourceStateTracker()->AddResource(this, BarrierStage::None, BarrierAccess::None);
		SetData(indices.data(), static_cast<uint32_t>(sizeof(uint32_t) * indices.size()));
	}

	VulkanIndexBuffer::~VulkanIndexBuffer()
	{
		GraphicsContext::GetResourceStateTracker()->RemoveResource(this);

		if (!m_allocation)
		{
			return;
		}

		RHIProxy::GetInstance().DestroyResource([allocation = m_allocation]() 
		{
			GraphicsContext::GetDefaultAllocator()->DestroyBuffer(allocation);
		});

		m_allocation = nullptr;
	}

	const uint32_t VulkanIndexBuffer::GetCount() const
	{
		return m_count;
	}

	void VulkanIndexBuffer::SetName(std::string_view name)
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

	const uint64_t VulkanIndexBuffer::GetDeviceAddress() const
	{
		return m_allocation->GetDeviceAddress();
	}

	const uint64_t VulkanIndexBuffer::GetByteSize() const
	{
		return m_allocation->GetSize();
	}

	void* VulkanIndexBuffer::GetHandleImpl() const
	{
		return m_allocation->GetResourceHandle<VkBuffer>();
	}

	void VulkanIndexBuffer::SetData(const void* data, const uint32_t size)
	{
		VkDeviceSize bufferSize = size;

		RefPtr<Allocation> stagingAllocation;

		if (m_allocation)
		{
			RHIProxy::GetInstance().DestroyResource([allocation = m_allocation]() 
			{
				GraphicsContext::GetDefaultAllocator()->DestroyBuffer(allocation);
			});

			m_allocation = nullptr;
		}

		auto allocator = GraphicsContext::GetDefaultAllocator();

		if (data)
		{
			stagingAllocation = allocator->CreateBuffer(bufferSize, BufferUsage::TransferSrc, MemoryUsage::CPU);

			// Copy to staging buffer
			{
				void* buffData = stagingAllocation->Map<void>();
				memcpy_s(buffData, size, data, size);
				stagingAllocation->Unmap();
			}
		}

		// Create GPU buffer
		{
			m_allocation = allocator->CreateBuffer(bufferSize, BufferUsage::IndexBuffer | BufferUsage::TransferDst);
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

			allocator->DestroyBuffer(stagingAllocation);
		}
	}
}
