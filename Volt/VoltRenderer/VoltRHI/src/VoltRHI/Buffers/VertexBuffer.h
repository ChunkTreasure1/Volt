#pragma once

#include "VoltRHI/Core/RHIResource.h"

namespace Volt::RHI
{
	class VertexBuffer : public RHIResource
	{
	public:
		virtual void SetData(const void* data, uint32_t size) = 0;

		static Ref<VertexBuffer> Create(const void* data, const uint32_t size);

	protected:
		VertexBuffer() = default;
		~VertexBuffer() override = default;
	};
}