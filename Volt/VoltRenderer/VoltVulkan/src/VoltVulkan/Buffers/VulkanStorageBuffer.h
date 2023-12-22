#pragma once

#include <VoltRHI/Buffers/StorageBuffer.h>

namespace Volt::RHI
{
	class Allocation;
	class Allocator;

	class VulkanStorageBuffer : public StorageBuffer
	{
	public:
		VulkanStorageBuffer(const uint32_t count, const size_t elementSize, std::string_view name, BufferUsage bufferUsage, MemoryUsage memoryUsage);
		VulkanStorageBuffer(const size_t size, std::string_view name, BufferUsage bufferUsage, MemoryUsage memoryUsage);
		VulkanStorageBuffer(const size_t size, Ref<Allocator> customAllocator, std::string_view name, BufferUsage bufferUsage, MemoryUsage memoryUsage);
		~VulkanStorageBuffer() override;

		void ResizeByteSize(const size_t byteSize) override;
		void Resize(const uint32_t size) override;

		const size_t GetByteSize() const override;
		const uint32_t GetSize() const override;
		Weak<Allocation> GetAllocation() const override;

		void Unmap() override;
		void SetData(const void* data, const size_t size) override;
		void SetData(Ref<CommandBuffer> commandBuffer, const void* data, const size_t size) override;

		Ref<BufferView> GetView() override;

		inline constexpr ResourceType GetType() const override { return ResourceType::StorageBuffer; }
		void SetName(std::string_view name) override;
		const uint64_t GetDeviceAddress() const override;

	protected:
		void* GetHandleImpl() const override;
		void* MapInternal() override;

	private:
		void Invalidate(const size_t byteSize);
		void Release();

		size_t m_elementSize = 0;
		size_t m_byteSize = 0;
		uint32_t m_size = 0;

		std::string m_name;

		Ref<Allocation> m_allocation;
		Weak<Allocator> m_customAllocator;

		bool m_allocatedUsingCustomAllocator = false;

		BufferUsage m_bufferUsage = BufferUsage::StorageBuffer;
		MemoryUsage m_memoryUsage = MemoryUsage::GPU;
	};
}
