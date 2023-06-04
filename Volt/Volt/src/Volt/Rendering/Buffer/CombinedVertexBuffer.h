#pragma once

#include "Volt/Core/Graphics/VulkanAllocator.h"

#include <vulkan/vulkan.h>

namespace Volt
{
	class CombinedVertexBuffer
	{
	public:
		CombinedVertexBuffer(const uint64_t vertexSize, const uint64_t maxVertexCount);
		~CombinedVertexBuffer();

		void Bind(VkCommandBuffer commandBuffer, uint32_t slot = 0) const;

		const uint64_t AppendVertices(const void* data, const uint64_t count);
		static Ref<CombinedVertexBuffer> Create(const uint64_t vertexSize, const uint64_t maxVertexCount);

	private:
		void Initialize();
		void Release();

		void Resize(uint64_t newSize);

		VkBuffer myBuffer = nullptr;
		VmaAllocation myAllocation = nullptr;
	
		const uint64_t myVertexSize;

		uint64_t myCurrentVertexCount = 0;
		uint64_t myTotalBufferSize = 0;
		uint64_t myConsumedSize = 0;
	};
}
