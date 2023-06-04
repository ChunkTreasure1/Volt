#pragma once

#include "Volt/Core/Graphics/VulkanAllocator.h"

namespace Volt
{
	class CombinedIndexBuffer
	{
	public:
		CombinedIndexBuffer(const uint32_t maxIndexCount);
		~CombinedIndexBuffer();

		void Bind(VkCommandBuffer commandBuffer) const;
		const uint64_t AppendIndices(const uint32_t* indices, const uint64_t indexCount);
		static Ref<CombinedIndexBuffer> Create(const uint32_t maxIndexCount);

	private:
		void Initialize();
		void Release();

		void Resize(uint64_t newSize);

		VkBuffer myBuffer = nullptr;
		VmaAllocation myAllocation = nullptr;
	
		uint64_t myCurrentIndexCount = 0;
		uint64_t myTotalBufferSize = 0;
		uint64_t myConsumedSize = 0;
	};
}
