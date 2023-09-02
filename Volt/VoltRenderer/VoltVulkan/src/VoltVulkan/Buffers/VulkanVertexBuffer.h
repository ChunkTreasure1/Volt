#pragma once

#include <VoltRHI/Buffers/VertexBuffer.h>

namespace Volt::RHI
{
	class Allocation;
	class VulkanVertexBuffer : public VertexBuffer
	{
	public:
		VulkanVertexBuffer(const void* data, const uint32_t size);
		~VulkanVertexBuffer() override;

		void SetData(const void* data, uint32_t size) override;
		inline constexpr ResourceType GetType() const override { return ResourceType::VertexBuffer; }
		void SetName(std::string_view name) override;

	protected:
		void* GetHandleImpl() const override;

	private:
		void Invalidate(const void* data, const uint32_t size);

		Ref<Allocation> m_allocation;
	};
}
