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
		~VulkanImageAllocation() override = default;

		void Unmap() override;
		[[nodiscard]] inline const UUID64 GetHeapID() const override { return 0; }
		[[nodiscard]] const uint64_t GetDeviceAddress() const override;

	protected:
		void* GetResourceHandleInternal() const override;
		void* MapInternal() override;

	private:
		friend class VulkanDefaultAllocator;

		void* GetHandleImpl() const override;

		VkImage_T* m_resource = nullptr;
		VmaAllocation_T* m_allocation = nullptr;
	};

	class VulkanBufferAllocation final : public Allocation
	{
	public:
		~VulkanBufferAllocation() override = default;

		void Unmap() override;
		inline const UUID64 GetHeapID() const override { return 0; }
		[[nodiscard]] const uint64_t GetDeviceAddress() const override;

	protected:
		void* GetResourceHandleInternal() const override;
		void* MapInternal() override;

	private:
		friend class VulkanDefaultAllocator;

		void* GetHandleImpl() const override;

		VkBuffer_T* m_resource = nullptr;
		VmaAllocation_T* m_allocation = nullptr;
	};

	class VulkanTransientBufferAllocation : public Allocation
	{
	public:
		~VulkanTransientBufferAllocation() override = default;

		void Unmap() override;
		inline const UUID64 GetHeapID() const override { return m_heapId; }
		[[nodiscard]] const uint64_t GetDeviceAddress() const override;

	protected:
		void* GetResourceHandleInternal() const override;
		void* MapInternal() override;

		void* GetHandleImpl() const override;

	private:
		friend class VulkanTransientHeap;

		VkBuffer_T* m_resource = nullptr;
		VkDeviceMemory_T* m_memoryHandle = nullptr;

		AllocationBlock m_allocationBlock{};
		UUID64 m_heapId = 0;
	};

	class VulkanTransientImageAllocation : public Allocation
	{
	public:
		~VulkanTransientImageAllocation() override = default;

		void Unmap() override;
		inline const UUID64 GetHeapID() const override { return m_heapId; }
		[[nodiscard]] const uint64_t GetDeviceAddress() const override;

	protected:
		void* GetResourceHandleInternal() const override;
		void* MapInternal() override;

		void* GetHandleImpl() const override;

	private:
		friend class VulkanTransientHeap;

		VkImage_T* m_resource = nullptr;
		VkDeviceMemory_T* m_memoryHandle = nullptr;

		AllocationBlock m_allocationBlock{};
		UUID64 m_heapId = 0;
	};
}