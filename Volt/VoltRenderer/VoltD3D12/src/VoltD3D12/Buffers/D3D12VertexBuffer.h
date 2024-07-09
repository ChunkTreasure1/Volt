#pragma once

#include <VoltRHI/Buffers/VertexBuffer.h>

namespace Volt::RHI
{
	class Allocation;
	class D3D12VertexBuffer : public VertexBuffer
	{
	public:
		D3D12VertexBuffer(const uint32_t size, const void* data);
		~D3D12VertexBuffer() override;

		void SetData(const void* data, uint32_t size) override;
		inline constexpr ResourceType GetType() const override { return ResourceType::VertexBuffer; }
		void SetName(std::string_view name) override;
		const uint64_t GetDeviceAddress() const override;
		const uint64_t GetByteSize() const override;

	protected:
		void* GetHandleImpl() const override;

	private:
		void Invalidate(const void* data, const uint32_t size);

		RefPtr<Allocation> m_allocation;
	};
}
