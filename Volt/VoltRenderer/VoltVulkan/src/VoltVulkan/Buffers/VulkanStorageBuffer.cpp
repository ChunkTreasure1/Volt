#include "vkpch.h"
#include "VulkanStorageBuffer.h"

#include "VoltVulkan/Common/VulkanFunctions.h"

#include <VoltRHI/Graphics/GraphicsContext.h>
#include <VoltRHI/Graphics/GraphicsDevice.h>
#include <VoltRHI/Buffers/BufferView.h>
#include <VoltRHI/Buffers/CommandBuffer.h>

#include <VoltRHI/Memory/MemoryCommon.h>
#include <VoltRHI/Memory/Allocation.h>

namespace Volt::RHI
{
	VulkanStorageBuffer::VulkanStorageBuffer(const uint32_t count, const size_t elementSize, std::string_view name, BufferUsage bufferUsage, MemoryUsage memoryUsage)
		: m_byteSize(count * elementSize), m_size(count), m_elementSize(elementSize), m_bufferUsage(bufferUsage), m_memoryUsage(memoryUsage), m_name(name)
	{
		Invalidate(elementSize * count);
		SetName(name);
	}

	VulkanStorageBuffer::VulkanStorageBuffer(const size_t size, std::string_view name, BufferUsage bufferUsage, MemoryUsage memoryUsage)
		: m_bufferUsage(bufferUsage), m_memoryUsage(memoryUsage), m_name(name)
	{
		Invalidate(size);
		SetName(name);
	}

	VulkanStorageBuffer::VulkanStorageBuffer(const size_t size, Ref<Allocator> customAllocator, std::string_view name, BufferUsage bufferUsage, MemoryUsage memoryUsage)
		: m_allocatedUsingCustomAllocator(true), m_customAllocator(customAllocator), m_bufferUsage(bufferUsage), m_memoryUsage(memoryUsage), m_name(name)
	{
		Invalidate(size);
		SetName(name);
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

		SetName(m_name);
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
		m_allocation->Unmap();
	}

	void VulkanStorageBuffer::SetData(const void* data, const size_t size)
	{
		Ref<Allocation> stagingAllocation = nullptr;

		if (m_allocatedUsingCustomAllocator)
		{
			stagingAllocation = m_customAllocator->CreateBuffer(size, BufferUsage::TransferSrc, MemoryUsage::CPUToGPU);
		}
		else
		{
			stagingAllocation = GraphicsContext::GetDefaultAllocator().CreateBuffer(size, BufferUsage::TransferSrc, MemoryUsage::CPUToGPU);
		}

		void* mappedPtr = stagingAllocation->Map<void>();
		memcpy_s(mappedPtr, m_byteSize, data, size);
		stagingAllocation->Unmap();

		const Ref<CommandBuffer> cmdBuffer = CommandBuffer::Create();
		cmdBuffer->Begin();

		cmdBuffer->CopyBufferRegion(stagingAllocation, 0, m_allocation, 0, size);

		cmdBuffer->End();
		cmdBuffer->Execute();

		GraphicsContext::DestroyResource([allocatedUsingCustomAllocator = m_allocatedUsingCustomAllocator, customAllocator = m_customAllocator, allocation = stagingAllocation]()
		{
			if (allocatedUsingCustomAllocator)
			{
				customAllocator->DestroyBuffer(allocation);
			}
			else
			{
				GraphicsContext::GetDefaultAllocator().DestroyBuffer(allocation);
			}
		});
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
		if (!Volt::RHI::vkSetDebugUtilsObjectNameEXT)
		{
			return;
		}

		VkDebugUtilsObjectNameInfoEXT nameInfo{};
		nameInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT;
		nameInfo.objectType = VK_OBJECT_TYPE_BUFFER;
		nameInfo.objectHandle = reinterpret_cast<uint64_t>(m_allocation->GetResourceHandle<VkBuffer>());
		nameInfo.pObjectName = name.data();

		const auto device = GraphicsContext::GetDevice();
		Volt::RHI::vkSetDebugUtilsObjectNameEXT(device->GetHandle<VkDevice>(), &nameInfo);
	}

	const uint64_t VulkanStorageBuffer::GetDeviceAddress() const
	{
		return m_allocation->GetDeviceAddress();
	}

	void* VulkanStorageBuffer::GetHandleImpl() const
	{
		return m_allocation->GetResourceHandle<VkBuffer>();
	}

	void* VulkanStorageBuffer::MapInternal()
	{
		return m_allocation->Map<void>();
	}

	void VulkanStorageBuffer::Invalidate(const size_t byteSize)
	{
		Release();
		m_byteSize = std::max(byteSize, Memory::GetMinBufferAllocationSize());

		const VkDeviceSize bufferSize = m_byteSize;

		if (m_allocatedUsingCustomAllocator)
		{
			m_allocation = m_customAllocator->CreateBuffer(bufferSize, m_bufferUsage | BufferUsage::TransferDst | BufferUsage::StorageBuffer, m_memoryUsage);
		}
		else
		{
			m_allocation = GraphicsContext::GetDefaultAllocator().CreateBuffer(bufferSize, m_bufferUsage | BufferUsage::TransferDst | BufferUsage::StorageBuffer, m_memoryUsage);
		}
	}

	void VulkanStorageBuffer::Release()
	{
		if (!m_allocation)
		{
			return;
		}

		GraphicsContext::DestroyResource([allocatedUsingCustomAllocator = m_allocatedUsingCustomAllocator, customAllocator = m_customAllocator, allocation = m_allocation]()
		{
			if (allocatedUsingCustomAllocator)
			{
				customAllocator->DestroyBuffer(allocation);
			}
			else
			{
				GraphicsContext::GetDefaultAllocator().DestroyBuffer(allocation);
			}
		});

		m_allocation = nullptr;
	}
}