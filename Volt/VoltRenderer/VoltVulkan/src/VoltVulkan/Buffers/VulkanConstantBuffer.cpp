#include "vkpch.h"
#include "VulkanConstantBuffer.h"

#include "VoltVulkan/Graphics/VulkanAllocator.h"
#include "VoltVulkan/Common/VulkanFunctions.h"

#include <VoltRHI/Buffers/BufferView.h>

#include <VoltRHI/Graphics/GraphicsContext.h>
#include <VoltRHI/Graphics/GraphicsDevice.h>

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

		GraphicsContext::DestroyResource([buffer = m_buffer, allocation = m_allocation]() 
		{
			VulkanAllocator allocator{};
			allocator.DestroyBuffer(buffer, allocation);
		});

		m_buffer = nullptr;
		m_allocation = nullptr;
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

	void VulkanConstantBuffer::SetName(std::string_view name)
	{
		VkDebugUtilsObjectNameInfoEXT nameInfo{};
		nameInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT;
		nameInfo.objectType = VK_OBJECT_TYPE_BUFFER;
		nameInfo.objectHandle = (uint64_t)m_buffer;
		nameInfo.pObjectName = name.data();

		auto device = GraphicsContext::GetDevice();
		vkSetDebugUtilsObjectNameEXT(device->GetHandle<VkDevice>(), &nameInfo);
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
