#pragma once

#include "VoltRHI/Core/RHIResource.h"

namespace Volt::RHI
{
	class BufferView;

	class StorageBuffer : public RHIResource
	{
	public:
		~StorageBuffer() override = default;

		virtual void ResizeByteSize(const size_t byteSize) = 0;
		virtual void Resize(const uint32_t size) = 0;

		virtual const size_t GetByteSize() const = 0;
		virtual const uint32_t GetSize() const = 0;

		virtual void Unmap() = 0;
		virtual void SetData(const void* data, const size_t size) = 0;

		virtual Ref<BufferView> GetView() = 0;

		template<typename T>
		T* Map();

		static Ref<StorageBuffer> Create(uint32_t count, size_t elementSize, MemoryUsage bufferUsage = MemoryUsage::Default);

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
