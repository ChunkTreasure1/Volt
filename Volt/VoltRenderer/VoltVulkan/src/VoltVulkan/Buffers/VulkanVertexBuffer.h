#pragma once

#include "VoltRHI/Buffers/VertexBuffer.h"

struct VkBuffer_T;
struct VmaAllocation_T;

namespace Volt::RHI
{
	class VulkanVertexBuffer : public VertexBuffer
	{
	public:
		VulkanVertexBuffer(const void* data, const uint32_t size);
		~VulkanVertexBuffer() override;

		void SetData(const void* data, uint32_t size) override;
		inline constexpr ResourceType GetType() const override { return ResourceType::VertexBuffer; }

	protected:
		void* GetHandleImpl() override;

	private:
		void Invalidate(const void* data, const uint32_t size);

		VkBuffer_T* m_buffer = nullptr;
		VmaAllocation_T* m_allocation = nullptr;
	};
}
