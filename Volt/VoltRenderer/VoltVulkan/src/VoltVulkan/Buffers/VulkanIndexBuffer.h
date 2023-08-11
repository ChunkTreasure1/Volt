#pragma once

#include <VoltRHI/Buffers/IndexBuffer.h>

struct VkBuffer_T;
struct VmaAllocation_T;

namespace Volt::RHI
{
	class VulkanIndexBuffer : public IndexBuffer
	{
	public:
		VulkanIndexBuffer(std::span<uint32_t> indices);
		VulkanIndexBuffer(const uint32_t* indices, const uint32_t count);
		~VulkanIndexBuffer() override;

		const uint32_t GetCount() const override;
		inline constexpr ResourceType GetType() const override { return ResourceType::IndexBuffer; }

	protected:
		void* GetHandleImpl() override;

	private:
		void SetData(const void* data, const uint32_t size);

		VkBuffer_T* m_buffer = nullptr;
		VmaAllocation_T* m_allocation = nullptr;

		uint32_t m_count = 0;
	};
}
