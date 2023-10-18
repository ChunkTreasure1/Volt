#pragma once

#include <VoltRHI/Buffers/UniformBuffer.h>

namespace Volt::RHI
{
	class Allocation;
	class VulkanUniformBuffer : public UniformBuffer
	{
	public:
		VulkanUniformBuffer(const uint32_t size, const void* data);
		~VulkanUniformBuffer() override;

		Ref<BufferView> GetView() override;
		const uint32_t GetSize() const override;
		void SetData(const void* data, const uint32_t size) override;
		void Unmap() override;

		inline constexpr ResourceType GetType() const override { return ResourceType::UniformBuffer; }
		void SetName(std::string_view name) override;
		const uint64_t GetDeviceAddress() const override;

	protected:
		void* MapInternal() override;
		void* GetHandleImpl() const override;

	private:
		uint32_t m_size = 0;

		Ref<Allocation> m_allocation;
	};
}
