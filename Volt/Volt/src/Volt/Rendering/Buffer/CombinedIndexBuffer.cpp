#include "vtpch.h"
#include "CombinedIndexBuffer.h"

#include "Volt/Core/Profiling.h"
#include "Volt/Core/Graphics/GraphicsContextVolt.h"
#include "Volt/Core/Graphics/GraphicsDeviceVolt.h"

#include "Volt/Rendering/Renderer.h"

namespace Volt
{
	CombinedIndexBuffer::CombinedIndexBuffer(const uint32_t maxIndexCount)
		: myTotalBufferSize(sizeof(uint32_t) * maxIndexCount)
	{
		Initialize();
	}

	CombinedIndexBuffer::~CombinedIndexBuffer()
	{
		Release();
	}

	void CombinedIndexBuffer::Bind(VkCommandBuffer commandBuffer) const
	{
		VT_PROFILE_FUNCTION();
		vkCmdBindIndexBuffer(commandBuffer, myBuffer, 0, VK_INDEX_TYPE_UINT32);
	}

	const uint64_t CombinedIndexBuffer::AppendIndices(const uint32_t* indices, const uint64_t indexCount)
	{
		if (myTotalBufferSize < myConsumedSize + sizeof(uint32_t) * indexCount)
		{
			const uint64_t newSize = gem::max(myConsumedSize + sizeof(uint32_t) * indexCount, (uint64_t)(myTotalBufferSize * 1.25));
			Resize(newSize);
		}

		const uint64_t location = myCurrentIndexCount;
		const uint64_t dataSize = sizeof(uint32_t) * indexCount;

		VkBuffer stagingBuffer;
		VmaAllocation stagingAllocation;
		VulkanAllocator allocator{ "CombinedIndexBuffer - AppendIndices" };

		// Create staging buffer
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
			memcpy_s(stagingData, dataSize, indices, dataSize);
			allocator.UnmapMemory(stagingAllocation);
		}

		// Copy from staging buffer to GPU buffer
		{
			auto device = GraphicsContextVolt::GetDevice();
			VkCommandBuffer cmdBuffer = device->GetSingleUseCommandBuffer(true);

			VkBufferCopy copy{};
			copy.srcOffset = 0;
			copy.dstOffset = myConsumedSize;
			copy.size = dataSize;

			vkCmdCopyBuffer(cmdBuffer, stagingBuffer, myBuffer, 1, &copy);
			device->FlushSingleUseCommandBuffer(cmdBuffer);
		}

		allocator.DestroyBuffer(stagingBuffer, stagingAllocation);
		myConsumedSize += dataSize;
		myCurrentIndexCount += indexCount;

		return location;
	}

	Ref<CombinedIndexBuffer> CombinedIndexBuffer::Create(const uint32_t maxIndexCount)
	{
		return CreateRef<CombinedIndexBuffer>(maxIndexCount);
	}

	void CombinedIndexBuffer::Initialize()
	{
		Resize(myTotalBufferSize);
	}

	void CombinedIndexBuffer::Release()
	{
		if (myBuffer == VK_NULL_HANDLE)
		{
			return;
		}

		Renderer::SubmitResourceChange([buffer = myBuffer, allocation = myAllocation]()
		{
			VulkanAllocator allocator{ "CombinedIndexBuffer - Destroy" };
			allocator.DestroyBuffer(buffer, allocation);
		});

		myAllocation = nullptr;
		myBuffer = nullptr;
	}

	void CombinedIndexBuffer::Resize(uint64_t newSize)
	{
		auto device = GraphicsContextVolt::GetDevice();
		device->WaitForIdle();

		VulkanAllocator allocator{};

		VmaAllocation newAlloc;
		VkBuffer newBuffer;

		// Create GPU buffer
		{
			VkBufferCreateInfo info{};
			info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
			info.size = newSize;
			info.usage = VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
			info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

			newAlloc = allocator.AllocateBuffer(info, VMA_MEMORY_USAGE_GPU_ONLY, newBuffer);
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
