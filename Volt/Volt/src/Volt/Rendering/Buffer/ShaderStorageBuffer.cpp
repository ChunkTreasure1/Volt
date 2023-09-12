#include "vtpch.h"
#include "ShaderStorageBuffer.h"

#include "Volt/Rendering/Renderer.h"

namespace Volt
{
	ShaderStorageBuffer::ShaderStorageBuffer(uint64_t elementSize, uint32_t elementCount, MemoryUsage usage)
		: myUsage(usage), myCurrentElementCount(elementCount), myElementSize(elementSize)
	{
		Resize(elementSize * elementCount);
	}

	ShaderStorageBuffer::~ShaderStorageBuffer()
	{
		Release();
	}

	void ShaderStorageBuffer::Resize(uint64_t newSize)
	{
		Release();

		mySize = newSize;

		//auto device = GraphicsContextVolt::GetDevice();
		const VkDeviceSize bufferSize = newSize;


		VkBufferCreateInfo info{};
		info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
		info.size = bufferSize;
		info.usage = VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;
		info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

		if ((myUsage & MemoryUsage::Indirect) != MemoryUsage::None)
		{
			info.usage |= VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT;
			info.usage |= VK_BUFFER_USAGE_TRANSFER_DST_BIT;
		}

		if ((myUsage & MemoryUsage::CPUToGPU) != MemoryUsage::None)
		{
		}
	}

	void ShaderStorageBuffer::ResizeWithElementCount(uint32_t newElementCount)
	{
		const VkDeviceSize newSize = newElementCount * myElementSize;

		Release();

		myCurrentElementCount = newElementCount;
		mySize = newSize;

		//auto device = GraphicsContextVolt::GetDevice();
		const VkDeviceSize bufferSize = newSize;

		VkBufferCreateInfo info{};
		info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
		info.size = bufferSize;
		info.usage = VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;
		info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

		if ((myUsage & MemoryUsage::Indirect) != MemoryUsage::None)
		{
			info.usage |= VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT;
			info.usage |= VK_BUFFER_USAGE_TRANSFER_DST_BIT;
		}


		if ((myUsage & MemoryUsage::CPUToGPU) != MemoryUsage::None)
		{
		}

	}

	void ShaderStorageBuffer::Unmap()
	{
	}

	Ref<ShaderStorageBuffer> ShaderStorageBuffer::Create(uint64_t elementSize, uint32_t elementCount, MemoryUsage usage)
	{
		return CreateRef<ShaderStorageBuffer>(elementSize, elementCount, usage);
	}

	void ShaderStorageBuffer::Release()
	{
		if (myBuffer == VK_NULL_HANDLE)
		{
			return;
		}

		myBuffer = nullptr;
	}
}
