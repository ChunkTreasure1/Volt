#include "vkpch.h"
#include "VulkanRHIModule/Buffers/VulkanUniformBuffer.h"

#include "VulkanRHIModule/Common/VulkanFunctions.h"
#include "VulkanRHIModule/Graphics/VulkanPhysicalGraphicsDevice.h"

#include <RHIModule/Buffers/BufferView.h>

#include <RHIModule/Graphics/GraphicsContext.h>
#include <RHIModule/Graphics/GraphicsDevice.h>

#include <RHIModule/Memory/Allocation.h>
#include <RHIModule/Memory/MemoryUtility.h>
#include <RHIModule/RHIProxy.h>

namespace Volt::RHI
{
	VulkanUniformBuffer::VulkanUniformBuffer(const uint32_t size, const void* data, const uint32_t count, std::string_view name)
		: m_size(size)
	{
		GraphicsContext::GetResourceStateTracker()->AddResource(this, BarrierStage::None, BarrierAccess::None);

		const auto& deviceProperties = GraphicsContext::GetPhysicalDevice()->As<VulkanPhysicalGraphicsDevice>()->GetProperties();
		const uint64_t alignedSize = Utility::Align(size, deviceProperties.limits.minUniformBufferOffsetAlignment);

		const VkDeviceSize bufferSize = alignedSize * count;
		m_allocation = GraphicsContext::GetDefaultAllocator()->CreateBuffer(bufferSize, BufferUsage::UniformBuffer, MemoryUsage::CPUToGPU);

		if (data)
		{
			SetData(data, size);
		}

		SetName(name);
	}

	VulkanUniformBuffer::~VulkanUniformBuffer()
	{
		GraphicsContext::GetResourceStateTracker()->RemoveResource(this);

		if (m_allocation == nullptr)
		{
			return;
		}

		RHIProxy::GetInstance().DestroyResource([allocation = m_allocation]() 
		{
			GraphicsContext::GetDefaultAllocator()->DestroyBuffer(allocation);
		});

		m_allocation = nullptr;
	}

	RefPtr<BufferView> VulkanUniformBuffer::GetView()
	{
		BufferViewSpecification spec{};
		spec.bufferResource = this;

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
		m_name = std::string(name);

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

	std::string_view VulkanUniformBuffer::GetName() const
	{
		return m_name;
	}

	const uint64_t VulkanUniformBuffer::GetDeviceAddress() const
	{
		return m_allocation->GetDeviceAddress();
	}

	const uint64_t VulkanUniformBuffer::GetByteSize() const
	{
		return m_allocation->GetSize();
	}

	void* VulkanUniformBuffer::MapInternal(const uint32_t index)
	{
		const auto& deviceProperties = GraphicsContext::GetPhysicalDevice()->As<VulkanPhysicalGraphicsDevice>()->GetProperties();
		const uint32_t offset = Utility::Align(m_size, deviceProperties.limits.minUniformBufferOffsetAlignment) * index;

		uint8_t* bytePtr = m_allocation->Map<uint8_t>();
		return &bytePtr[offset];
	}

	void* VulkanUniformBuffer::GetHandleImpl() const
	{
		return m_allocation->GetResourceHandle<VkBuffer>();
	}
}
