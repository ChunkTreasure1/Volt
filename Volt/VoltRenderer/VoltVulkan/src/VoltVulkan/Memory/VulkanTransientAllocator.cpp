#include "vkpch.h"
#include "VulkanTransientAllocator.h"

#include <VoltRHI/Graphics/GraphicsContext.h>
#include <VoltRHI/Graphics/PhysicalGraphicsDevice.h>

#include <vulkan/vulkan.h>

namespace Volt::RHI
{
	VulkanTransientAllocator::VulkanTransientAllocator()
	{
		auto physicalDevice = GraphicsContext::GetPhysicalDevice();

		VkPhysicalDeviceProperties deviceProperties{};
		vkGetPhysicalDeviceProperties(physicalDevice->GetHandle<VkPhysicalDevice>(), &deviceProperties);

		VkPhysicalDeviceMemoryProperties memoryProperties{};
		vkGetPhysicalDeviceMemoryProperties(physicalDevice->GetHandle<VkPhysicalDevice>(), &memoryProperties);

		m_pageSize = deviceProperties.limits.bufferImageGranularity;
		m_memoryBlockMinSize = 128 * 1024 * m_pageSize;
	}
	
	VulkanTransientAllocator::~VulkanTransientAllocator()
	{
	}

	Ref<Allocation> VulkanTransientAllocator::CreateBuffer(const size_t size, BufferUsage usage, MemoryUsage memoryUsage)
	{
		return Ref<Allocation>();
	}

	Ref<Allocation> VulkanTransientAllocator::CreateImage(const ImageSpecification& imageSpecification, MemoryUsage memoryUsage)
	{
		return Ref<Allocation>();
	}

	Ref<Allocation> VulkanTransientAllocator::CreateImage(const ImageSpecification& imageSpecification, Ref<MemoryPool> pool, MemoryUsage memoryUsage)
	{
		return Ref<Allocation>();
	}

	void VulkanTransientAllocator::DestroyBuffer(Ref<Allocation> allocation)
	{
	}

	void VulkanTransientAllocator::DestroyImage(Ref<Allocation> allocation)
	{
	}

	void* VulkanTransientAllocator::GetHandleImpl() const
	{
		return nullptr;
	}
}
