#pragma once

#include <RHIModule/Buffers/VertexBuffer.h>

namespace Volt::RHI
{
	class Allocation;
	class D3D12VertexBuffer : public VertexBuffer
	{
	public:
		D3D12VertexBuffer(const void* data, const uint32_t size, const uint32_t stride);
		~D3D12VertexBuffer() override;

		void SetData(const void* data, uint32_t size) override;
		uint32_t GetStride() const override;
		inline constexpr ResourceType GetType() const override { return ResourceType::VertexBuffer; }
		void SetName(std::string_view name) override;
		std::string_view GetName() const override;
		const uint64_t GetDeviceAddress() const override;
		const uint64_t GetByteSize() const override;

	protected:
		void* GetHandleImpl() const override;

	private:
		void Invalidate(const void* data, const uint32_t size);

		std::string m_name;
		RefPtr<Allocation> m_allocation;
		uint32_t m_stride = 0;
	};
}
