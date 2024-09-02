#pragma once

#include "VulkanRHIModule/Core.h"

#include <RHIModule/Buffers/StorageBuffer.h>

namespace Volt::RHI
{
	class Allocation;
	class Allocator;

	class VulkanStorageBuffer : public StorageBuffer
	{
	public:
		VulkanStorageBuffer(uint32_t count, uint64_t elementSize, std::string_view name, BufferUsage bufferUsage = BufferUsage::StorageBuffer, MemoryUsage memoryUsage = MemoryUsage::GPU, RefPtr<Allocator> allocator = nullptr);
		~VulkanStorageBuffer() override;

		void Resize(const uint64_t size) override;
		void ResizeWithCount(const uint32_t count) override;

		const uint64_t GetElementSize() const override;
		const uint32_t GetCount() const override;
		WeakPtr<Allocation> GetAllocation() const override;

		void Unmap() override;
		void SetData(const void* data, const size_t size) override;
		void SetData(RefPtr<CommandBuffer> commandBuffer, const void* data, const size_t size) override;

		RefPtr<BufferView> GetView() override;

		inline constexpr ResourceType GetType() const override { return ResourceType::StorageBuffer; }
		void SetName(std::string_view name) override;
		const uint64_t GetDeviceAddress() const override;
		const uint64_t GetByteSize() const override;

	protected:
		void* GetHandleImpl() const override;
		void* MapInternal() override;

	private:
		void Invalidate(const uint64_t byteSize);
		void Release();

		uint64_t m_elementSize = 0;
		uint64_t m_byteSize = 0;
		uint32_t m_count = 0;

		std::string m_name;

		RefPtr<BufferView> m_view;
		RefPtr<Allocation> m_allocation;
		WeakPtr<Allocator> m_allocator;

		BufferUsage m_bufferUsage = BufferUsage::StorageBuffer;
		MemoryUsage m_memoryUsage = MemoryUsage::GPU;
	};
}
