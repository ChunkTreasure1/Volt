#pragma once

#include <VoltRHI/Buffers/IndexBuffer.h>

namespace Volt::RHI
{
	class Allocation;
	class D3D12IndexBuffer : public IndexBuffer
	{
	public:
		D3D12IndexBuffer(std::span<const uint32_t> indices);
		~D3D12IndexBuffer() override;

		const uint32_t GetCount() const override;
		inline constexpr ResourceType GetType() const override { return ResourceType::IndexBuffer; }
		void SetName(std::string_view name) override;
		const uint64_t GetDeviceAddress() const override;
		const uint64_t GetByteSize() const override;

	protected:
		void* GetHandleImpl() const override;

	private:
		void SetData(const void* data, const uint32_t size);

		RefPtr<Allocation> m_allocation;

		uint32_t m_count = 0;
	};
}
