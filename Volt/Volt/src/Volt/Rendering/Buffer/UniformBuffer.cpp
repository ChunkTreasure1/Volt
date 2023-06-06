#include "vtpch.h"
#include "UniformBuffer.h"

#include "Volt/Core/Graphics/GraphicsContextVolt.h"
#include "Volt/Core/Graphics/GraphicsDeviceVolt.h"

#include "Volt/Rendering/Shader/ShaderUtility.h"
#include "Volt/Rendering/Renderer.h"

namespace Volt
{
	UniformBuffer::UniformBuffer(uint32_t size, const void* data)
		: mySize(size), myTotalSize(size)
	{
		const VkDeviceSize bufferSize = (VkDeviceSize)size;
		VulkanAllocator allocator{ "UniformBuffer - Create" };

		// Create buffer
		{
			VkBufferCreateInfo info{};
			info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
			info.size = bufferSize;
			info.usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
			info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

			myAllocation = allocator.AllocateBuffer(info, VMA_MEMORY_USAGE_CPU_TO_GPU, myBuffer);
		}

		if (data)
		{
			SetData(data, size);
		}
	}

	UniformBuffer::UniformBuffer(uint32_t sizePerElement, uint32_t elementCount)
	{
		myIsDynamic = true;

		const uint64_t minUBOAlignment = GraphicsContextVolt::GetPhysicalDevice()->GetCapabilities().minUBOOffsetAlignment;
		uint32_t alignedSize = sizePerElement;

		if (minUBOAlignment > 0)
		{
			alignedSize = (uint32_t)Utility::GetAlignedSize((uint64_t)alignedSize, minUBOAlignment);
		}

		mySize = alignedSize;
		myTotalSize = alignedSize * elementCount;

		const VkDeviceSize bufferSize = (uint64_t)myTotalSize;
		VulkanAllocator allocator{ "UniformBuffer - Create" };

		// Create buffer
		{
			VkBufferCreateInfo info{};
			info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
			info.size = bufferSize;
			info.usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
			info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

			myAllocation = allocator.AllocateBuffer(info, VMA_MEMORY_USAGE_CPU_TO_GPU, myBuffer);
		}
	}

	UniformBuffer::~UniformBuffer()
	{
		if (myBuffer == VK_NULL_HANDLE)
		{
			return;
		}

		Renderer::SubmitResourceChange([buffer = myBuffer, allocation = myAllocation]()
		{
			VulkanAllocator allocator{};
			allocator.DestroyBuffer(buffer, allocation);
		});

	}

	void UniformBuffer::SetData(const void* data, uint32_t size)
	{
		VT_CORE_ASSERT(mySize >= size, "Unable to set data of larger size than buffer!");

		VkDeviceSize bufferSize = (VkDeviceSize)size;
		VulkanAllocator allocator{ "UniformBuffer - SetData" };

		void* bufferData = allocator.MapMemory<void*>(myAllocation);
		memcpy_s(bufferData, mySize, data, size);
		allocator.UnmapMemory(myAllocation);
	}

	void UniformBuffer::Unmap()
	{
		VulkanAllocator allocator{};
		allocator.UnmapMemory(myAllocation);
	}

	Ref<UniformBuffer> UniformBuffer::Create(uint32_t size, const void* data)
	{
		return CreateRef<UniformBuffer>(size, data);
	}

	Ref<UniformBuffer> UniformBuffer::Create(uint32_t sizePerElement, uint32_t elementCount)
	{
		return CreateRef<UniformBuffer>(sizePerElement, elementCount);
	}
}
