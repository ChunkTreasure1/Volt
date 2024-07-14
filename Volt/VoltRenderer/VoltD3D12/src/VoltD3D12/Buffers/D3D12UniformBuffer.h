#pragma once

#include <VoltRHI/Buffers/UniformBuffer.h>

namespace Volt::RHI
{
	class Allocation;
	class D3D12UniformBuffer : public UniformBuffer
	{
	public:
		D3D12UniformBuffer(const uint32_t size, const void* data, const uint32_t count, std::string_view name);
		~D3D12UniformBuffer() override;

		RefPtr<BufferView> GetView() override;
		const uint32_t GetSize() const override;
		void SetData(const void* data, const uint32_t size) override;
		void Unmap() override;

		inline constexpr ResourceType GetType() const override { return ResourceType::UniformBuffer; }
		void SetName(std::string_view name) override;
		const uint64_t GetDeviceAddress() const override;
		const uint64_t GetByteSize() const override;

	protected:
		void* MapInternal(const uint32_t index) override;
		void* GetHandleImpl() const override;

	private:
		uint32_t m_size = 0;

		RefPtr<Allocation> m_allocation;
	};
}
