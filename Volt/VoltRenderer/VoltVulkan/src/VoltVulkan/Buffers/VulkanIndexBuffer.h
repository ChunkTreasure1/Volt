#pragma once

#include <VoltRHI/Buffers/IndexBuffer.h>

namespace Volt::RHI
{
	class Allocation;
	class VulkanIndexBuffer : public IndexBuffer
	{
	public:
		VulkanIndexBuffer(std::span<uint32_t> indices);
		VulkanIndexBuffer(const uint32_t* indices, const uint32_t count);
		~VulkanIndexBuffer() override;

		const uint32_t GetCount() const override;
		inline constexpr ResourceType GetType() const override { return ResourceType::IndexBuffer; }
		void SetName(std::string_view name) override;

	protected:
		void* GetHandleImpl() override;

	private:
		void SetData(const void* data, const uint32_t size);

		Ref<Allocation> m_allocation;

		uint32_t m_count = 0;
	};
}
