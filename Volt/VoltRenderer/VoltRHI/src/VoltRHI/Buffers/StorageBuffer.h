#pragma once

#include "VoltRHI/Core/RHIResource.h"

namespace Volt::RHI
{
	class BufferView;
	class Allocator;
	class CommandBuffer;
	class Allocation;

	class StorageBuffer : public RHIResource
	{
	public:
		~StorageBuffer() override = default;

		virtual void ResizeByteSize(const size_t byteSize) = 0;
		virtual void Resize(const uint32_t size) = 0;

		virtual const size_t GetSize() const = 0;
		virtual const uint32_t GetCount() const = 0;
		virtual Weak<Allocation> GetAllocation() const = 0;

		virtual void Unmap() = 0;
		virtual void SetData(const void* data, const size_t size) = 0;
		virtual void SetData(Ref<CommandBuffer> commandBuffer, const void* data, const size_t size) = 0;

		virtual Ref<BufferView> GetView() = 0;

		template<typename T>
		T* Map();

		static Ref<StorageBuffer> Create(uint32_t count, size_t elementSize, std::string_view name, BufferUsage bufferUsage = BufferUsage::StorageBuffer, MemoryUsage memoryUsage = MemoryUsage::GPU);
		static Ref<StorageBuffer> Create(size_t size, std::string_view name, BufferUsage bufferUsage = BufferUsage::StorageBuffer, MemoryUsage memoryUsage = MemoryUsage::GPU);
		static Ref<StorageBuffer> Create(size_t size, Ref<Allocator> customAllocator, std::string_view name, BufferUsage bufferUsage = BufferUsage::StorageBuffer, MemoryUsage memoryUsage = MemoryUsage::GPU);

	protected:
		virtual void* MapInternal() = 0;

		StorageBuffer() = default;
	};

	template<typename T>
	inline T* StorageBuffer::Map()
	{
		return reinterpret_cast<T*>(MapInternal());
	}
}
