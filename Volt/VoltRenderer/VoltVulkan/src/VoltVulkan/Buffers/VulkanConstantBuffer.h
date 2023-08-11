#pragma once

#include <VoltRHI/Buffers/ConstantBuffer.h>

struct VkBuffer_T;
struct VmaAllocation_T;

namespace Volt::RHI
{
	class VulkanConstantBuffer : public ConstantBuffer
	{
	public:
		VulkanConstantBuffer(const uint32_t size, const void* data);
		~VulkanConstantBuffer() override;

		Ref<BufferView> GetView() override;
		const uint32_t GetSize() const override;
		void SetData(const void* data, const uint32_t size) override;
		void Unmap() override;

		inline constexpr ResourceType GetType() const override { return ResourceType::ConstantBuffer; }

	protected:
		void* MapInternal() override;
		void* GetHandleImpl() override;

	private:
		uint32_t m_size = 0;

		VkBuffer_T* m_buffer = nullptr;
		VmaAllocation_T* m_allocation = nullptr;
	};
}
