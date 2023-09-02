#pragma once

#include <VoltRHI/Memory/Allocation.h>

struct VmaAllocation_T;
struct VkBuffer_T;
struct VkImage_T;

namespace Volt::RHI
{
	class VulkanImageAllocation final : public Allocation
	{
	public:
		~VulkanImageAllocation() override = default;

		void Unmap() override;

	protected:
		void* GetResourceHandleInternal() const override;
		void* MapInternal() override;

	private:
		friend class VulkanAllocator;

		void* GetHandleImpl() const override;

		VkImage_T* m_resource = nullptr;
		VmaAllocation_T* m_allocation = nullptr;
	};

	class VulkanBufferAllocation final : public Allocation
	{
	public:
		~VulkanBufferAllocation() override = default;

		void Unmap() override;

	protected:
		void* GetResourceHandleInternal() const override;
		void* MapInternal() override;

	private:
		friend class VulkanAllocator;
		
		void* GetHandleImpl() const override;

		VkBuffer_T* m_resource = nullptr;
		VmaAllocation_T* m_allocation = nullptr;
	};
}
