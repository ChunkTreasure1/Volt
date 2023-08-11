#pragma once

#include "VoltRHI/Core/RHIResource.h"

namespace Volt::RHI
{
	class BufferView;

	class ConstantBuffer : public RHIResource
	{ 
	public:
		~ConstantBuffer() override = default;

		virtual Ref<BufferView> GetView() = 0;
		virtual const uint32_t GetSize() const = 0;
		virtual void SetData(const void* data, const uint32_t size) = 0;
		virtual void Unmap() = 0;

		template<typename T>
		T* Map();

		static Ref<ConstantBuffer> Create(const uint32_t size, const void* data = nullptr);

	protected:
		virtual void* MapInternal() = 0;

		ConstantBuffer() = default;
	};

	template<typename T>
	inline T* ConstantBuffer::Map()
	{
		return reinterpret_cast<T*>(MapInternal());
	}
}
