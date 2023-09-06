#include "vkpch.h"
#include "VulkanUniformBuffer.h"

#include "VoltVulkan/Common/VulkanFunctions.h"

#include <VoltRHI/Buffers/BufferView.h>

#include <VoltRHI/Graphics/GraphicsContext.h>
#include <VoltRHI/Graphics/GraphicsDevice.h>

#include <VoltRHI/Memory/Allocation.h>

namespace Volt::RHI
{
	VulkanUniformBuffer::VulkanUniformBuffer(const uint32_t size, const void* data)
		: m_size(size)
	{
		const VkDeviceSize bufferSize = static_cast<VkDeviceSize>(size);

		VkBufferCreateInfo info{};
		info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
		info.size = bufferSize;
		info.usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
		info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

		m_allocation = GraphicsContext::GetDefaultAllocator().CreateBuffer(bufferSize, BufferUsage::UniformBuffer, MemoryUsage::CPUToGPU);

		if (data)
		{
			SetData(data, size);
		}
	}

	VulkanUniformBuffer::~VulkanUniformBuffer()
	{
		if (m_allocation == nullptr)
		{
			return;
		}

		GraphicsContext::DestroyResource([allocation = m_allocation]() 
		{
			GraphicsContext::GetDefaultAllocator().DestroyBuffer(allocation);
		});

		m_allocation = nullptr;
	}

	Ref<BufferView> VulkanUniformBuffer::GetView()
	{
		BufferViewSpecification spec{};
		spec.bufferResource = As<VulkanUniformBuffer>();

		return BufferView::Create(spec);
	}

	const uint32_t VulkanUniformBuffer::GetSize() const
	{
		return m_size;
	}

	void VulkanUniformBuffer::SetData(const void* data, const uint32_t size)
	{
		void* bufferData = m_allocation->Map<void>();
		memcpy_s(bufferData, m_size, data, size);
		m_allocation->Unmap();
	}

	void VulkanUniformBuffer::Unmap()
	{
		m_allocation->Unmap();
	}

	void VulkanUniformBuffer::SetName(std::string_view name)
	{
		VkDebugUtilsObjectNameInfoEXT nameInfo{};
		nameInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT;
		nameInfo.objectType = VK_OBJECT_TYPE_BUFFER;
		nameInfo.objectHandle = (uint64_t)m_allocation->GetResourceHandle<VkBuffer>();
		nameInfo.pObjectName = name.data();

		auto device = GraphicsContext::GetDevice();
		vkSetDebugUtilsObjectNameEXT(device->GetHandle<VkDevice>(), &nameInfo);
	}

	void* VulkanUniformBuffer::MapInternal()
	{
		return m_allocation->Map<void>();
	}

	void* VulkanUniformBuffer::GetHandleImpl() const
	{
		return m_allocation->GetResourceHandle<VkBuffer>();
	}
}
