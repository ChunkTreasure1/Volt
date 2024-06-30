#include "rhipch.h"
#include "StorageBuffer.h"

#include "VoltRHI/RHIProxy.h"
#include "VoltRHI/Memory/Allocator.h"

namespace Volt::RHI
{
	RefPtr<StorageBuffer> StorageBuffer::Create(uint32_t count, uint64_t elementSize, std::string_view name, BufferUsage bufferUsage, MemoryUsage memoryUsage, RefPtr<Allocator> allocator)
	{
		return RHIProxy::GetInstance().CreateStorageBuffer(count, elementSize, name, bufferUsage, memoryUsage, allocator);
	}
}
