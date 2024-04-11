#pragma once

#include "VoltRHI/Core/RHIResource.h"

namespace Volt::RHI
{
	class BufferView;

	class VTRHI_API UniformBuffer : public RHIResource
	{ 
	public:
		~UniformBuffer() override = default;

		virtual Ref<BufferView> GetView() = 0;
		virtual const uint32_t GetSize() const = 0;
		virtual void SetData(const void* data, const uint32_t size) = 0;
		virtual void Unmap() = 0;

		template<typename T>
		inline T* Map();

		template<typename T>
		inline void SetData(const T& data);

		static Ref<UniformBuffer> Create(const uint32_t size, const void* data = nullptr);

	protected:
		virtual void* MapInternal() = 0;

		UniformBuffer() = default;
	};

	template<typename T>
	inline T* UniformBuffer::Map()
	{
		return reinterpret_cast<T*>(MapInternal());
	}

	template<typename T>
	inline void UniformBuffer::SetData(const T& data)
	{
		SetData(&data, sizeof(T));
	}
}
