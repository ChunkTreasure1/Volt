#pragma once

#include "Volt/Core/Graphics/VulkanAllocator.h"
#include "Volt/Rendering/Vertex.h"

#include <vector>

namespace Volt
{
	class VertexBuffer
	{
	public:
		VertexBuffer(const void* data, uint32_t size, bool mappable);
		VertexBuffer(uint32_t size, bool mappable);
		~VertexBuffer();

		void SetData(const void* data, uint32_t size);
		void SetData(VkCommandBuffer commandBuffer, const void* data, uint32_t size);
		void SetDataMapped(VkCommandBuffer commandBuffer, const void* data, uint32_t size);

		void Bind(VkCommandBuffer commandBuffer, uint32_t binding = 0) const;

		const uint64_t GetDeviceAddress() const; 

		static Ref<VertexBuffer> Create(const void* data, uint32_t size, bool mappable = false);
		static Ref<VertexBuffer> Create(uint32_t size, bool mappable = false);

	private:
		void Invalidate(const void* data, uint32_t size);

		uint32_t mySize = 0;
		bool myMappable = false;

		VkBuffer myBuffer = nullptr;
		VmaAllocation myBufferAllocation = nullptr;

		VkBuffer myStagingBuffer = nullptr;
		VmaAllocation myStagingAllocation = nullptr;
	};
}
