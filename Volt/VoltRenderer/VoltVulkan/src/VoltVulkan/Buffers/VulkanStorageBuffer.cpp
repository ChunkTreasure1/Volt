#include "vkpch.h"
#include "VulkanStorageBuffer.h"

#include "VoltVulkan/Common/VulkanFunctions.h"

#include <VoltRHI/Graphics/GraphicsContext.h>
#include <VoltRHI/Graphics/GraphicsDevice.h>
#include <VoltRHI/Buffers/BufferView.h>
#include <VoltRHI/Buffers/CommandBuffer.h>

#include <VoltRHI/Memory/MemoryCommon.h>
#include <VoltRHI/Memory/Allocation.h>

#include <VoltRHI/RHIProxy.h>

namespace Volt::RHI
{
	VulkanStorageBuffer::VulkanStorageBuffer(uint32_t count, uint64_t elementSize, std::string_view name, BufferUsage bufferUsage, MemoryUsage memoryUsage, RefPtr<Allocator> allocator)
		: m_count(count), m_elementSize(elementSize), m_name(name), m_bufferUsage(bufferUsage), m_memoryUsage(memoryUsage), m_allocator(allocator)
	{
		if (!m_allocator)
		{
			m_allocator = GraphicsContext::GetDefaultAllocator();
		}

		Invalidate(m_elementSize * m_count);
	}

	VulkanStorageBuffer::~VulkanStorageBuffer()
	{
		Release();
	}

	void VulkanStorageBuffer::Resize(const uint64_t byteSize)
	{
		m_count = static_cast<uint32_t>(byteSize);

		Invalidate(byteSize);
		SetName(m_name);
	}

	void VulkanStorageBuffer::ResizeWithCount(const uint32_t count)
	{
		m_count = count;
		const uint64_t newSize = m_count * m_elementSize;

		Invalidate(newSize);
		SetName(m_name);
	}

	const size_t VulkanStorageBuffer::GetByteSize() const
	{
		return m_allocation->GetSize();
	}

	const size_t VulkanStorageBuffer::GetElementSize() const
	{
		return m_elementSize;
	}

	const uint32_t VulkanStorageBuffer::GetCount() const
	{
		return m_count;
	}

	WeakPtr<Allocation> VulkanStorageBuffer::GetAllocation() const
	{
		return m_allocation;
	}

	void VulkanStorageBuffer::Unmap()
	{
		m_allocation->Unmap();
	}

	void VulkanStorageBuffer::SetData(const void* data, const size_t size)
	{
		RefPtr<Allocation> stagingAllocation = m_allocator->CreateBuffer(size, BufferUsage::TransferSrc, MemoryUsage::CPUToGPU);

		void* mappedPtr = stagingAllocation->Map<void>();
		memcpy_s(mappedPtr, size, data, size);
		stagingAllocation->Unmap();

		RefPtr<CommandBuffer> cmdBuffer = CommandBuffer::Create();
		cmdBuffer->Begin();

		ResourceBarrierInfo barrier{};
		barrier.type = BarrierType::Buffer;
		barrier.bufferBarrier().srcStage = BarrierStage::ComputeShader | BarrierStage::VertexShader | BarrierStage::PixelShader;
		barrier.bufferBarrier().srcAccess = BarrierAccess::None;
		barrier.bufferBarrier().dstStage = BarrierStage::Copy;
		barrier.bufferBarrier().dstAccess = BarrierAccess::TransferDestination;
		barrier.bufferBarrier().offset = 0;
		barrier.bufferBarrier().size = size;
		barrier.bufferBarrier().resource = WeakPtr<VulkanStorageBuffer>(this);

		cmdBuffer->ResourceBarrier({ barrier });

		cmdBuffer->CopyBufferRegion(stagingAllocation, 0, m_allocation, 0, size);

		barrier.bufferBarrier().srcStage = BarrierStage::Copy;
		barrier.bufferBarrier().srcAccess = BarrierAccess::TransferDestination;
		barrier.bufferBarrier().dstStage = BarrierStage::ComputeShader | BarrierStage::VertexShader | BarrierStage::PixelShader;
		barrier.bufferBarrier().dstAccess = BarrierAccess::None;

		cmdBuffer->ResourceBarrier({ barrier });

		cmdBuffer->End();
		cmdBuffer->Execute();

		RHIProxy::GetInstance().DestroyResource([allocator = m_allocator, allocation = stagingAllocation]()
		{
			allocator->DestroyBuffer(allocation);
		});
	}

	void VulkanStorageBuffer::SetData(RefPtr<CommandBuffer> commandBuffer, const void* data, const size_t size)
	{
		RefPtr<Allocation> stagingAllocation = m_allocator->CreateBuffer(size, BufferUsage::TransferSrc, MemoryUsage::CPUToGPU);

		void* mappedPtr = stagingAllocation->Map<void>();
		memcpy_s(mappedPtr, m_byteSize, data, size);
		stagingAllocation->Unmap();

		ResourceBarrierInfo barrier{};
		barrier.bufferBarrier().srcStage = BarrierStage::All;
		barrier.bufferBarrier().srcAccess = BarrierAccess::None;
		barrier.bufferBarrier().dstStage = BarrierStage::Copy;
		barrier.bufferBarrier().dstAccess = BarrierAccess::TransferDestination;
		barrier.bufferBarrier().offset = 0;
		barrier.bufferBarrier().size = size;
		barrier.bufferBarrier().resource = WeakPtr<VulkanStorageBuffer>(this);

		commandBuffer->ResourceBarrier({ barrier });

		commandBuffer->CopyBufferRegion(stagingAllocation, 0, m_allocation, 0, size);

		barrier.bufferBarrier().srcStage = BarrierStage::Copy;
		barrier.bufferBarrier().srcAccess = BarrierAccess::TransferDestination;
		barrier.bufferBarrier().dstStage = BarrierStage::All;
		barrier.bufferBarrier().dstAccess = BarrierAccess::None;

		commandBuffer->ResourceBarrier({ barrier });

		RHIProxy::GetInstance().DestroyResource([allocator = m_allocator, allocation = stagingAllocation]()
		{
			allocator->DestroyBuffer(allocation);
		});
	}

	RefPtr<BufferView> VulkanStorageBuffer::GetView()
	{
		if (m_view)
		{
			return m_view;
		}

		BufferViewSpecification spec{};
		spec.bufferResource = this;

		m_view = BufferView::Create(spec);
		return m_view;
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

	void VulkanStorageBuffer::Invalidate(const uint64_t byteSize)
	{
		Release();
		m_byteSize = std::max(byteSize, Memory::GetMinBufferAllocationSize());

		const VkDeviceSize bufferSize = m_byteSize;
		m_allocation = m_allocator->CreateBuffer(bufferSize, m_bufferUsage | BufferUsage::TransferDst | BufferUsage::StorageBuffer, m_memoryUsage);
	}

	void VulkanStorageBuffer::Release()
	{
		if (!m_allocation)
		{
			return;
		}

		RHIProxy::GetInstance().DestroyResource([allocator = m_allocator, allocation = m_allocation]()
		{
			allocator->DestroyBuffer(allocation);
		});

		m_allocation = nullptr;
	}
}
