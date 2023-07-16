#pragma once

#include "Volt/Core/Graphics/VulkanAllocatorVolt.h"

namespace Volt
{
	class GPUImageMemoryPool
	{
	public:
		GPUImageMemoryPool(size_t poolSize, VkImageUsageFlags supportedUsages);
		~GPUImageMemoryPool();

		inline VmaPool GetPool() const { return myPool; }

	private:
		void Invalidate();
		void Release();

		size_t mySize = 0;
		VmaPool myPool = nullptr;

		VkImageUsageFlags mySupportedUsages;
	};
}
