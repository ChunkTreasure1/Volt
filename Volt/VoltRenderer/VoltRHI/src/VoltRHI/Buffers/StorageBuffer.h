#pragma once

#include "VoltRHI/Core/RHIResource.h"
#include "VoltRHI/Memory/Allocator.h"
#include "VoltRHI/Buffers/BufferView.h"

namespace Volt::RHI
{
	class CommandBuffer;
	class Allocation;

	class VTRHI_API StorageBuffer : public RHIResource
	{
	public:
		~StorageBuffer() override = default;

		virtual void Resize(const uint64_t size) = 0;
		virtual void ResizeWithCount(const uint32_t count) = 0;

		virtual const uint64_t GetElementSize() const = 0;
		virtual const uint32_t GetCount() const = 0;
		virtual WeakPtr<Allocation> GetAllocation() const = 0;

		virtual void Unmap() = 0;
		virtual void SetData(const void* data, const size_t size) = 0;
		virtual void SetData(RefPtr<CommandBuffer> commandBuffer, const void* data, const size_t size) = 0;

		virtual RefPtr<BufferView> GetView() = 0;

		template<typename T>
		T* Map();

		template<typename T>
		static RefPtr<StorageBuffer> Create(uint32_t count, std::string_view name, BufferUsage bufferUsage = BufferUsage::StorageBuffer, MemoryUsage memoryUsage = MemoryUsage::GPU, RefPtr<Allocator> allocator = nullptr);
		static RefPtr<StorageBuffer> Create(uint32_t count, uint64_t elementSize, std::string_view name, BufferUsage bufferUsage = BufferUsage::StorageBuffer, MemoryUsage memoryUsage = MemoryUsage::GPU, RefPtr<Allocator> allocator = nullptr);

	protected:
		virtual void* MapInternal() = 0;

		StorageBuffer() = default;
	};

	template<typename T>
	inline T* StorageBuffer::Map()
	{
		return reinterpret_cast<T*>(MapInternal());
	}

	template<typename T>
	inline RefPtr<StorageBuffer> StorageBuffer::Create(uint32_t count, std::string_view name, BufferUsage bufferUsage, MemoryUsage memoryUsage, RefPtr<Allocator> allocator)
	{
		return Create(count, sizeof(T), name, bufferUsage, memoryUsage, allocator);
	}
}
