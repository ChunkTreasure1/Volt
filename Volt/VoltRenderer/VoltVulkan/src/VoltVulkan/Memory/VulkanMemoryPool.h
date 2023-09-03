#pragma once

#include <VoltRHI/Memory/MemoryPool.h>

struct VmaPool_T;

namespace Volt::RHI
{
	class VulkanMemoryPool : public MemoryPool
	{
	public:
		VulkanMemoryPool(MemoryUsage memoryUsage);
		~VulkanMemoryPool() override;

	protected:
		void* GetHandleImpl() const override;

	private:
		VmaPool_T* m_pool;
	};
}
