#pragma once

#include "VoltRHI/Core/RHIResource.h"
#include <span>

namespace Volt::RHI
{
	class VTRHI_API IndexBuffer : public RHIResource
	{
	public:
		virtual const uint32_t GetCount() const = 0;

		static RefPtr<IndexBuffer> Create(std::span<uint32_t> indices);

	protected:
		IndexBuffer() = default;
		~IndexBuffer() override = default;
	};
}
