#include "rhipch.h"
#include "StorageBuffer.h"

#include "VoltRHI/Graphics/GraphicsContext.h"

#include <VoltVulkan/Buffers/VulkanStorageBuffer.h>

namespace Volt::RHI
{
	Ref<StorageBuffer> StorageBuffer::Create(uint32_t count, size_t elementSize, BufferUsage bufferUsage, MemoryUsage memoryUsage)
	{
		const auto api = GraphicsContext::GetAPI();

		switch (api)
		{
			case GraphicsAPI::D3D12:
			case GraphicsAPI::Mock:
			case GraphicsAPI::MoltenVk:
				break;

			case GraphicsAPI::Vulkan: return CreateRefRHI<VulkanStorageBuffer>(count, elementSize, bufferUsage, memoryUsage); break;
		}

		return nullptr;
	}

	Ref<StorageBuffer> StorageBuffer::Create(size_t size, BufferUsage bufferUsage, MemoryUsage memoryUsage)
	{
		const auto api = GraphicsContext::GetAPI();

		switch (api)
		{
			case GraphicsAPI::D3D12:
			case GraphicsAPI::Mock:
			case GraphicsAPI::MoltenVk:
				break;

			case GraphicsAPI::Vulkan: return CreateRefRHI<VulkanStorageBuffer>(size, bufferUsage, memoryUsage); break;
		}

		return nullptr;
	}

	Ref<StorageBuffer> StorageBuffer::Create(size_t size, Ref<Allocator> customAllocator, BufferUsage bufferUsage, MemoryUsage memoryUsage)
	{
		const auto api = GraphicsContext::GetAPI();

		switch (api)
		{
			case GraphicsAPI::D3D12:
			case GraphicsAPI::Mock:
			case GraphicsAPI::MoltenVk:
				break;

			case GraphicsAPI::Vulkan: return CreateRefRHI<VulkanStorageBuffer>(size, customAllocator, bufferUsage, memoryUsage); break;
		}

		return nullptr;
	}
}
