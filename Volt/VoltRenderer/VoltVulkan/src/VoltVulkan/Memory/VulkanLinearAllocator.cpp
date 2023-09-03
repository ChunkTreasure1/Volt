#include "vkpch.h"
#include "VulkanLinearAllocator.h"

#include <VoltRHI/Graphics/GraphicsContext.h>
#include <VoltRHI/Graphics/PhysicalGraphicsDevice.h>

#include <vulkan/vulkan.h>

namespace Volt::RHI
{
	VulkanLinearAllocator::VulkanLinearAllocator()
	{
		auto physicalDevice = GraphicsContext::GetPhysicalDevice();

		VkPhysicalDeviceProperties deviceProperties{};
		vkGetPhysicalDeviceProperties(physicalDevice->GetHandle<VkPhysicalDevice>(), &deviceProperties);

		VkPhysicalDeviceMemoryProperties memoryProperties{};
		vkGetPhysicalDeviceMemoryProperties(physicalDevice->GetHandle<VkPhysicalDevice>(), &memoryProperties);

		m_pageSize = deviceProperties.limits.bufferImageGranularity;
		m_memoryBlockMinSize = m_pageSize * 10;
	}
	
	VulkanLinearAllocator::~VulkanLinearAllocator()
	{
	}

	Ref<Allocation> VulkanLinearAllocator::CreateBuffer(const size_t size, BufferUsage usage, MemoryUsage memoryUsage)
	{
		return Ref<Allocation>();
	}

	Ref<Allocation> VulkanLinearAllocator::CreateImage(const ImageSpecification& imageSpecification, MemoryUsage memoryUsage)
	{
		return Ref<Allocation>();
	}

	Ref<Allocation> VulkanLinearAllocator::CreateImage(const ImageSpecification& imageSpecification, Ref<MemoryPool> pool, MemoryUsage memoryUsage)
	{
		return Ref<Allocation>();
	}

	void VulkanLinearAllocator::DestroyBuffer(Ref<Allocation> allocation)
	{
	}

	void VulkanLinearAllocator::DestroyImage(Ref<Allocation> allocation)
	{
	}

	void* VulkanLinearAllocator::GetHandleImpl() const
	{
		return nullptr;
	}
}
