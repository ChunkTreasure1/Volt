#pragma once

#include <VoltRHI/Memory/Allocation.h>

struct VmaAllocation_T;
struct VkBuffer_T;
struct VkImage_T;
struct VkDeviceMemory_T;

namespace Volt::RHI
{
	class VulkanImageAllocation final : public Allocation
	{
	public:
		VulkanImageAllocation(const size_t hash);
		~VulkanImageAllocation() override = default;

		void Unmap() override;
		VT_NODISCARD VT_INLINE const UUID64 GetHeapID() const override { return 0; }
		VT_NODISCARD const uint64_t GetDeviceAddress() const override;
		VT_NODISCARD VT_INLINE const size_t GetHash() const override { return m_allocationHash; }
		VT_NODISCARD VT_INLINE const uint64_t GetSize() const override { return m_size; }

	protected:
		void* GetResourceHandleInternal() const override;
		void* MapInternal() override;

	private:
		friend class VulkanDefaultAllocator;

		void* GetHandleImpl() const override;

		VkImage_T* m_resource = nullptr;
		VmaAllocation_T* m_allocation = nullptr;
		size_t m_allocationHash = 0;
		uint64_t m_size = 0;
	};

	class VulkanBufferAllocation final : public Allocation
	{
	public:
		VulkanBufferAllocation(const size_t hash);
		~VulkanBufferAllocation() override = default;

		void Unmap() override;
		VT_NODISCARD VT_INLINE const UUID64 GetHeapID() const override { return 0; }
		VT_NODISCARD const uint64_t GetDeviceAddress() const override;
		VT_NODISCARD VT_INLINE const size_t GetHash() const override { return m_allocationHash; }
		VT_NODISCARD VT_INLINE const uint64_t GetSize() const override { return m_size; }

	protected:
		void* GetResourceHandleInternal() const override;
		void* MapInternal() override;

	private:
		friend class VulkanDefaultAllocator;

		void* GetHandleImpl() const override;

		VkBuffer_T* m_resource = nullptr;
		VmaAllocation_T* m_allocation = nullptr;
		size_t m_allocationHash = 0;
		uint64_t m_size = 0;
	};

	class VulkanTransientBufferAllocation : public Allocation
	{
	public:
		VulkanTransientBufferAllocation(const size_t hash);
		~VulkanTransientBufferAllocation() override = default;

		void Unmap() override;
		VT_NODISCARD VT_INLINE const UUID64 GetHeapID() const override { return m_heapId; }
		VT_NODISCARD const uint64_t GetDeviceAddress() const override;
		VT_NODISCARD VT_INLINE const size_t GetHash() const override { return m_allocationHash; }
		VT_NODISCARD VT_INLINE const uint64_t GetSize() const override { return m_size; }

	protected:
		void* GetResourceHandleInternal() const override;
		void* MapInternal() override;

		void* GetHandleImpl() const override;

	private:
		friend class VulkanTransientHeap;

		VkBuffer_T* m_resource = nullptr;
		VkDeviceMemory_T* m_memoryHandle = nullptr;
		size_t m_allocationHash = 0;
		uint64_t m_size = 0;

		AllocationBlock m_allocationBlock{};
		UUID64 m_heapId = 0;
	};

	class VulkanTransientImageAllocation : public Allocation
	{
	public:
		VulkanTransientImageAllocation(const size_t hash);
		~VulkanTransientImageAllocation() override = default;

		void Unmap() override;
		VT_NODISCARD VT_INLINE const UUID64 GetHeapID() const override { return m_heapId; }
		VT_NODISCARD const uint64_t GetDeviceAddress() const override;
		VT_NODISCARD VT_INLINE const size_t GetHash() const override { return m_allocationHash; }
		VT_NODISCARD VT_INLINE const uint64_t GetSize() const override { return m_size; }

	protected:
		void* GetResourceHandleInternal() const override;
		void* MapInternal() override;

		void* GetHandleImpl() const override;

	private:
		friend class VulkanTransientHeap;

		VkImage_T* m_resource = nullptr;
		VkDeviceMemory_T* m_memoryHandle = nullptr;
		size_t m_allocationHash = 0;
		uint64_t m_size = 0;

		AllocationBlock m_allocationBlock{};
		UUID64 m_heapId = 0;
	};
}
