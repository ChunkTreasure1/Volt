#include "vtpch.h"
#include "IndexBuffer.h"

#include "Volt/Core/Graphics/GraphicsContext.h"
#include "Volt/Core/Graphics/GraphicsDevice.h"

#include "Volt/Core/Profiling.h"

#include "Volt/Rendering/Renderer.h"

namespace Volt
{
	IndexBuffer::IndexBuffer(const std::vector<uint32_t>& indices, uint32_t count)
		: m_count(count)
	{
		SetData(indices.data(), sizeof(uint32_t) * count);
	}

	IndexBuffer::IndexBuffer(uint32_t* indices, uint32_t count)
		: m_count(count)
	{
		SetData(indices, sizeof(uint32_t) * count);
	}

	IndexBuffer::~IndexBuffer()
	{
		if (!myBuffer)
		{
			return;
		}

		Renderer::SubmitResourceChange([buffer = myBuffer, allocation = myBufferAllocation]()
		{
			VulkanAllocator allocator{};
			allocator.DestroyBuffer(buffer, allocation);
		});

		myBuffer = nullptr;
		myBufferAllocation = nullptr;
	}

	void IndexBuffer::Bind(VkCommandBuffer commandBuffer)
	{
		VT_PROFILE_FUNCTION();

		const VkDeviceSize offset = 0;
		vkCmdBindIndexBuffer(commandBuffer, myBuffer, offset, VK_INDEX_TYPE_UINT32);
	}

	const uint64_t IndexBuffer::GetDeviceAddress() const
	{
		VkBufferDeviceAddressInfo bufferAddr{};
		bufferAddr.sType = VK_STRUCTURE_TYPE_BUFFER_DEVICE_ADDRESS_INFO;
		bufferAddr.pNext = nullptr;
		bufferAddr.buffer = myBuffer;

		auto device = GraphicsContext::GetDevice();

		return vkGetBufferDeviceAddress(device->GetHandle(), &bufferAddr);
	}

	Ref<IndexBuffer> IndexBuffer::Create(const std::vector<uint32_t>& indices, uint32_t count)
	{
		return CreateRef<IndexBuffer>(indices, count);
	}

	Ref<IndexBuffer> IndexBuffer::Create(uint32_t* indices, uint32_t count)
	{
		return CreateRef<IndexBuffer>(indices, count);
	}

	void IndexBuffer::SetData(const void* data, uint32_t size)
	{
		auto device = GraphicsContext::GetDevice();

		VkBuffer stagingBuffer;
		VmaAllocation stagingAllocation;
		VkDeviceSize bufferSize = size;
		VulkanAllocator allocator{ "IndexBuffer - Create" };

		if (myBuffer != VK_NULL_HANDLE)
		{
			allocator.DestroyBuffer(myBuffer, myBufferAllocation);
			myBuffer = nullptr;
			myBufferAllocation = nullptr;
		}

		if (data != nullptr)
		{
			// Create staging buffer
			{
				VkBufferCreateInfo bufferInfo{};
				bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
				bufferInfo.size = bufferSize;
				bufferInfo.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
				bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

				stagingAllocation = allocator.AllocateBuffer(bufferInfo, VMA_MEMORY_USAGE_CPU_ONLY, stagingBuffer);
			}

			// Copy to staging buffer
			{
				void* buffData = allocator.MapMemory<void>(stagingAllocation);
				memcpy_s(buffData, size, data, size);
				allocator.UnmapMemory(stagingAllocation);
			}
		}

		// Create GPU buffer
		{
			VkBufferCreateInfo bufferInfo{};
			bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
			bufferInfo.size = bufferSize;
			bufferInfo.usage = VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_BUILD_INPUT_READ_ONLY_BIT_KHR | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT;
			bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

			myBufferAllocation = allocator.AllocateBuffer(bufferInfo, VMA_MEMORY_USAGE_GPU_ONLY, myBuffer);
		}

		if (data != nullptr)
		{
			// Copy from staging buffer to GPU buffer
			{
				VkCommandBuffer cmdBuffer = device->GetSingleUseCommandBuffer(true);

				VkBufferCopy copy{};
				copy.srcOffset = 0;
				copy.dstOffset = 0;
				copy.size = bufferSize;

				vkCmdCopyBuffer(cmdBuffer, stagingBuffer, myBuffer, 1, &copy);
				device->FlushSingleUseCommandBuffer(cmdBuffer);
			}

			allocator.DestroyBuffer(stagingBuffer, stagingAllocation);
		}
	}
}
