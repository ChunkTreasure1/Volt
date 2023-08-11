#pragma once

#include "VoltRHI/Core/RHIResource.h"
#include <span>

namespace Volt::RHI
{
	class IndexBuffer : public RHIResource
	{
	public:
		virtual const uint32_t GetCount() const = 0;

		static Ref<IndexBuffer> Create(std::span<uint32_t> indices);
		static Ref<IndexBuffer> Create(const uint32_t* indices, const uint32_t count);

	protected:
		IndexBuffer() = default;
		~IndexBuffer() override = default;
	};
}
