#include "vtpch.h"
#include "VertexBuffer.h"

#include "Volt/Core/Graphics/GraphicsContextVolt.h"
#include "Volt/Core/Graphics/GraphicsDeviceVolt.h"

#include "Volt/Core/Profiling.h"

#include "Volt/Rendering/Renderer.h"

namespace Volt
{
	VertexBuffer::VertexBuffer(const void* data, uint32_t size, bool mapped)
		: mySize(size), myMappable(mapped)
	{
		VT_CORE_ASSERT(size > 0, "Size must be greater than zero!");

		Invalidate(data, size);
	}

	VertexBuffer::VertexBuffer(uint32_t size, bool mapped)
		: mySize(size), myMappable(mapped)
	{
		VT_CORE_ASSERT(size > 0, "Size must be greater than zero!");
		Invalidate(nullptr, size);
	}

	VertexBuffer::~VertexBuffer()
	{
		Renderer::SubmitResourceChange([buffer = myBuffer, allocation = myBufferAllocation, stagingBuffer = myStagingBuffer, stagingAlloc = myStagingAllocation]()
		{
			if (buffer != VK_NULL_HANDLE)
			{
				VulkanAllocatorVolt allocator{ "VertexBuffer - Destroy" };
				allocator.DestroyBuffer(buffer, allocation);
			}

			if (stagingBuffer != VK_NULL_HANDLE)
			{
				VulkanAllocatorVolt allocator{ "VertexBuffer - Destroy" };
				allocator.DestroyBuffer(stagingBuffer, stagingAlloc);
			}
		});

		myBuffer = nullptr;
		myBufferAllocation = nullptr;
		myStagingBuffer = nullptr;
		myStagingAllocation = nullptr;
	}

	void VertexBuffer::SetData(const void* data, uint32_t size)
	{
		//auto device = GraphicsContextVolt::GetDevice();
		//VkCommandBuffer cmdBuffer = device->GetSingleUseCommandBuffer(true);
		//
		//SetData(cmdBuffer, data, size);

		//device->FlushSingleUseCommandBuffer(cmdBuffer);
	}

	void VertexBuffer::SetData(VkCommandBuffer commandBuffer, const void* data, uint32_t size)
	{
		VT_PROFILE_FUNCTION();

		VulkanAllocatorVolt allocator{};

		if (!myStagingBuffer)
		{
			VkBufferCreateInfo bufferInfo{};
			bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
			bufferInfo.size = mySize;
			bufferInfo.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
			bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

			myStagingAllocation = allocator.AllocateBuffer(bufferInfo, VMA_MEMORY_USAGE_CPU_ONLY, myStagingBuffer);
		}

		// Copy to staging buffer
		{
			VT_PROFILE_SCOPE("Copy Data");

			void* buffData = allocator.MapMemory<void>(myStagingAllocation);
			memcpy_s(buffData, mySize, data, size);
			allocator.UnmapMemory(myStagingAllocation);
		}

		// Copy from staging buffer to GPU buffer
		{
			VkBufferCopy copy{};
			copy.srcOffset = 0;
			copy.dstOffset = 0;
			copy.size = size;

			vkCmdCopyBuffer(commandBuffer, myStagingBuffer, myBuffer, 1, &copy);
		}
	}

	void VertexBuffer::SetDataMapped(VkCommandBuffer, const void* data, uint32_t size)
	{
		VulkanAllocatorVolt allocator{};
		void* mapped = allocator.MapMemory<void>(myBufferAllocation);
		memcpy_s(mapped, mySize, data, size);
		allocator.UnmapMemory(myBufferAllocation);
	}

	void VertexBuffer::Bind(VkCommandBuffer commandBuffer, uint32_t binding) const
	{
		VT_PROFILE_FUNCTION();

		const VkDeviceSize offset = 0;
		vkCmdBindVertexBuffers(commandBuffer, binding, 1, &myBuffer, &offset);
	}

	const uint64_t VertexBuffer::GetDeviceAddress() const
	{
		//VkBufferDeviceAddressInfo bufferAddr{};
		//bufferAddr.sType = VK_STRUCTURE_TYPE_BUFFER_DEVICE_ADDRESS_INFO;
		//bufferAddr.pNext = nullptr;
		//bufferAddr.buffer = myBuffer;

		//auto device = GraphicsContextVolt::GetDevice();

		//return vkGetBufferDeviceAddress(device->GetHandle(), &bufferAddr);

		return 0;
	}

	Ref<VertexBuffer> VertexBuffer::Create(const void* data, uint32_t size, bool mapped)
	{
		return CreateRef<VertexBuffer>(data, size, mapped);
	}

	Ref<VertexBuffer> VertexBuffer::Create(uint32_t size, bool mapped)
	{
		return CreateRef<VertexBuffer>(size, mapped);
	}

	void VertexBuffer::Invalidate(const void* data, uint32_t size)
	{
		//auto device = GraphicsContextVolt::GetDevice();

		//VkBuffer stagingBuffer = nullptr;
		//VmaAllocation stagingAllocation = nullptr;
		//VkDeviceSize bufferSize = size;
		//VulkanAllocatorVolt allocator{ "VertexBuffer - Create" };

		//if (myBuffer != VK_NULL_HANDLE)
		//{
		//	allocator.DestroyBuffer(myBuffer, myBufferAllocation);
		//	myBuffer = nullptr;
		//	myBufferAllocation = nullptr;
		//}

		//if (data != nullptr)
		//{
		//	// Create staging buffer
		//	{
		//		VkBufferCreateInfo bufferInfo{};
		//		bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
		//		bufferInfo.size = bufferSize;
		//		bufferInfo.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
		//		bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

		//		stagingAllocation = allocator.AllocateBuffer(bufferInfo, VMA_MEMORY_USAGE_CPU_ONLY, stagingBuffer);
		//	}

		//	// Copy to staging buffer
		//	{
		//		void* buffData = allocator.MapMemory<void>(stagingAllocation);
		//		memcpy_s(buffData, size, data, size);
		//		allocator.UnmapMemory(stagingAllocation);
		//	}
		//}

		//// Create GPU buffer
		//{
		//	VkBufferCreateInfo bufferInfo{};
		//	bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
		//	bufferInfo.size = bufferSize;
		//	bufferInfo.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_BUILD_INPUT_READ_ONLY_BIT_KHR | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT;
		//	bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

		//	myBufferAllocation = allocator.AllocateBuffer(bufferInfo, myMappable ? VMA_MEMORY_USAGE_CPU_TO_GPU : VMA_MEMORY_USAGE_GPU_ONLY, myBuffer);
		//}

		//if (data != nullptr)
		//{
		//	// Copy from staging buffer to GPU buffer
		//	{
		//		VkCommandBuffer cmdBuffer = device->GetSingleUseCommandBuffer(true);

		//		VkBufferCopy copy{};
		//		copy.srcOffset = 0;
		//		copy.dstOffset = 0;
		//		copy.size = bufferSize;

		//		vkCmdCopyBuffer(cmdBuffer, stagingBuffer, myBuffer, 1, &copy);
		//		device->FlushSingleUseCommandBuffer(cmdBuffer);
		//	}

		//	allocator.DestroyBuffer(stagingBuffer, stagingAllocation);
		//}
	}
}
