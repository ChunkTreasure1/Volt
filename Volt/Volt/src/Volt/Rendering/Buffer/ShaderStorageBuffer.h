#pragma once

#include "Volt/Core/Graphics/VulkanAllocatorVolt.h"

#include "Volt/Rendering/RendererCommon.h"

namespace Volt
{
	class ShaderStorageBuffer
	{
	public:
		ShaderStorageBuffer(uint64_t elementSize, uint32_t elementCount, MemoryUsage usage);
		~ShaderStorageBuffer();

		void Resize(uint64_t newSize);
		void ResizeWithElementCount(uint32_t newElementCount);

		inline const VkBuffer GetHandle() const { return myBuffer; }
		inline const uint64_t GetSize() const { return mySize; }
		inline const uint64_t GetTotalSize() const { return mySize; }
		inline const uint32_t GetElementCount() const { return myCurrentElementCount; }

		template<typename T>
		T* Map();
		void Unmap();

		static Ref<ShaderStorageBuffer> Create(uint64_t elementSize, uint32_t elementCount, MemoryUsage usage = MemoryUsage::None);

	private:
		void Release();
	
		uint64_t mySize = 0;
		uint64_t myElementSize = 0;
		uint32_t myCurrentElementCount = 0;

		MemoryUsage myUsage;

		VkBuffer myBuffer = nullptr;
		VmaAllocation myAllocation = nullptr;
	};

	template<typename T>
	inline T* ShaderStorageBuffer::Map()
	{
		VulkanAllocatorVolt allocator{};
		return allocator.MapMemory<T>(myAllocation);
	}
}
