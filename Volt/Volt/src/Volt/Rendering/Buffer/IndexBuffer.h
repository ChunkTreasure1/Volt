#pragma once

#include "Volt/Core/Graphics/VulkanAllocatorVolt.h"
#include <vector>

namespace Volt
{
	class IndexBuffer
	{
	public:
		IndexBuffer(const std::vector<uint32_t>& indices, uint32_t count);
		IndexBuffer(uint32_t* indices, uint32_t count);
		~IndexBuffer();

		void Bind(VkCommandBuffer commandBuffer);

		const uint64_t GetDeviceAddress() const;

		static Ref<IndexBuffer> Create(const std::vector<uint32_t>& pIndices, uint32_t count);
		static Ref<IndexBuffer> Create(uint32_t* pIndices, uint32_t count);

	private:
		void SetData(const void* data, uint32_t size);

		VkBuffer myBuffer = nullptr;
		VmaAllocation myBufferAllocation = nullptr;
		uint32_t m_count = 0;
	};
}
