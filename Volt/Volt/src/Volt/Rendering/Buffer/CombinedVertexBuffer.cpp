#include "vtpch.h"
#include "CombinedVertexBuffer.h"

#include "Volt/Core/Graphics/GraphicsDeviceVolt.h"
#include "Volt/Core/Graphics/GraphicsContextVolt.h"
#include "Volt/Core/Profiling.h"

#include "Volt/Rendering/Renderer.h"

namespace Volt
{
	CombinedVertexBuffer::CombinedVertexBuffer(const uint64_t vertexSize, const uint64_t maxVertexCount)
		: myVertexSize(vertexSize), myTotalBufferSize(vertexSize* maxVertexCount)
	{
		Initialize();
	}

	CombinedVertexBuffer::~CombinedVertexBuffer()
	{
		Release();
	}

	void CombinedVertexBuffer::Bind(VkCommandBuffer commandBuffer, uint32_t slot) const
	{
		VT_PROFILE_FUNCTION();

		constexpr VkDeviceSize offset = 0;
		vkCmdBindVertexBuffers(commandBuffer, slot, 1, &myBuffer, &offset);
	}

	const uint64_t CombinedVertexBuffer::AppendVertices(const void* data, const uint64_t count)
	{
		// If the buffer is too small, allocate at least enough or 1.25 times the size
		if (myTotalBufferSize < myConsumedSize + myVertexSize * count)
		{
			const uint64_t newSize = gem::max(myConsumedSize + myVertexSize * count, (uint64_t)(myTotalBufferSize * 1.25));
			Resize(newSize);
		}

		const uint64_t location = myCurrentVertexCount;
		const uint64_t dataSize = myVertexSize * count;

		VkBuffer stagingBuffer;
		VmaAllocation stagingAllocation;
		VulkanAllocator allocator{ "CombinedVertexBuffer - AppendVertices" };

		// Create Staging Buffer
		{
			VkBufferCreateInfo info{};
			info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
			info.size = dataSize;
			info.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
			info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

			stagingAllocation = allocator.AllocateBuffer(info, VMA_MEMORY_USAGE_CPU_TO_GPU, stagingBuffer);
		}

		// Copy data to staging buffer
		{
			void* stagingData = allocator.MapMemory<void>(stagingAllocation);
			memcpy_s(stagingData, dataSize, data, dataSize);
			allocator.UnmapMemory(stagingAllocation);
		}

		// Copy from staging buffer to GPU buffer
		{
			auto device = GraphicsContextVolt::GetDevice();
			VkCommandBuffer cmdBuffer = device->GetSingleUseCommandBuffer(true);

			VkBufferCopy copyInfo{};
			copyInfo.size = dataSize;
			copyInfo.srcOffset = 0;
			copyInfo.dstOffset = myConsumedSize;

			vkCmdCopyBuffer(cmdBuffer, stagingBuffer, myBuffer, 1, &copyInfo);
			device->FlushSingleUseCommandBuffer(cmdBuffer); // #TODO_Ivar: Switch to using transfer queue
		}

		allocator.DestroyBuffer(stagingBuffer, stagingAllocation);

		myConsumedSize += dataSize;
		myCurrentVertexCount += count;

		return location;
	}

	Ref<CombinedVertexBuffer> CombinedVertexBuffer::Create(const uint64_t vertexSize, const uint64_t maxVertexCount)
	{
		return CreateRef<CombinedVertexBuffer>(vertexSize, maxVertexCount);
	}

	void CombinedVertexBuffer::Initialize()
	{
		Resize(myTotalBufferSize);
	}

	void CombinedVertexBuffer::Release()
	{
		if (myBuffer == VK_NULL_HANDLE)
		{
			return;
		}

		Renderer::SubmitResourceChange([buffer = myBuffer, allocation = myAllocation]()
		{
			VulkanAllocator allocator{ "CombinedVertexBuffer - Destroy" };
			allocator.DestroyBuffer(buffer, allocation);
		});

		myBuffer = nullptr;
		myAllocation = nullptr;
	}

	void CombinedVertexBuffer::Resize(uint64_t newSize)
	{
		auto device = GraphicsContextVolt::GetDevice();
		device->WaitForIdle();

		VulkanAllocator allocator{ };

		VmaAllocation newAlloc;
		VkBuffer newBuffer;

		// Create GPU buffer
		{
			VkBufferCreateInfo createInfo{};
			createInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
			createInfo.size = newSize;
			createInfo.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
			createInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

			newAlloc = allocator.AllocateBuffer(createInfo, VMA_MEMORY_USAGE_GPU_ONLY, newBuffer);
		}

		// Copy old data into new buffer
		if (myBuffer)
		{
			auto commandBuffer = device->GetSingleUseCommandBuffer(true);
			
			VkBufferCopy copyInfo{};
			copyInfo.size = myTotalBufferSize;
			copyInfo.srcOffset = 0;
			copyInfo.dstOffset = 0;

			vkCmdCopyBuffer(commandBuffer, myBuffer, newBuffer, 1, &copyInfo);
			device->FlushSingleUseCommandBuffer(commandBuffer);
		}

		Release();

		myBuffer = newBuffer;
		myAllocation = newAlloc;

		myTotalBufferSize = newSize;
	}
}
