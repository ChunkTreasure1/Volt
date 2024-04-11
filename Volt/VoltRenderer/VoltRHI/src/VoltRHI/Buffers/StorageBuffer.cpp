#include "rhipch.h"
#include "StorageBuffer.h"

#include "VoltRHI/RHIProxy.h"

namespace Volt::RHI
{
	Ref<StorageBuffer> StorageBuffer::Create(uint32_t count, size_t elementSize, std::string_view name, BufferUsage bufferUsage, MemoryUsage memoryUsage)
	{
		return RHIProxy::GetInstance().CreateStorageBuffer(count, elementSize, name, bufferUsage, memoryUsage);
	}

	Ref<StorageBuffer> StorageBuffer::Create(size_t size, std::string_view name, BufferUsage bufferUsage, MemoryUsage memoryUsage)
	{
		return RHIProxy::GetInstance().CreateStorageBuffer(size, name, bufferUsage, memoryUsage);
	}

	Ref<StorageBuffer> StorageBuffer::Create(size_t size, Ref<Allocator> customAllocator, std::string_view name, BufferUsage bufferUsage, MemoryUsage memoryUsage)
	{
		return RHIProxy::GetInstance().CreateStorageBuffer(size, customAllocator, name, bufferUsage, memoryUsage);
	}
}
