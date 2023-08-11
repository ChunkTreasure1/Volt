#pragma once

#include <VoltRHI/Buffers/StorageBuffer.h>

struct VkBuffer_T;
struct VmaAllocation_T;

namespace Volt::RHI
{
	class VulkanStorageBuffer : public StorageBuffer
	{
	public:
		VulkanStorageBuffer(const uint32_t count, const size_t elementSize, MemoryUsage bufferUsage);
		~VulkanStorageBuffer() override;

		void ResizeByteSize(const size_t byteSize) override;
		void Resize(const uint32_t size) override;

		const size_t GetByteSize() const override;
		const uint32_t GetSize() const override;

		void Unmap() override;

		Ref<BufferView> GetView() override;

		inline constexpr ResourceType GetType() const override { return ResourceType::StorageBuffer; }
		void SetName(std::string_view name) override;

	protected:
		void* GetHandleImpl() override;
		void* MapInternal() override;

	private:
		void Release();

		size_t m_byteSize = 0;
		uint32_t m_size = 0;

		VkBuffer_T* m_buffer = nullptr;
		VmaAllocation_T* m_allocation = nullptr;

		MemoryUsage m_memoryUsage = MemoryUsage::Default;
	};
}
