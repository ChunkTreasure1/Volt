#pragma once

#include "VoltRHI/Core/RHIResource.h"

namespace Volt::RHI
{
	class VTRHI_API VertexBuffer : public RHIResource
	{
	public:
		virtual void SetData(const void* data, uint32_t size) = 0;

		static Ref<VertexBuffer> Create(const uint32_t size, const void* data);

	protected:
		VertexBuffer() = default;
		~VertexBuffer() override = default;
	};
}
