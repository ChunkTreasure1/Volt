#pragma once

#include "VoltVulkan/Core.h"

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
		const uint64_t GetDeviceAddress() const override;
		const uint64_t GetByteSize() const override;

	protected:
		void* GetHandleImpl() const override;

	private:
		void Invalidate(const void* data, const uint32_t size);

		Ref<Allocation> m_allocation;
	};

	VTVK_API Ref<VertexBuffer> CreateVulkanVertexBuffer(const void* data, const uint32_t size);
}
