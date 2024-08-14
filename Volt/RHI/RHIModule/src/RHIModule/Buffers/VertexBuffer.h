#pragma once

#include "RHIModule/Core/RHIResource.h"

namespace Volt::RHI
{
	class VTRHI_API VertexBuffer : public RHIResource
	{
	public:
		virtual void SetData(const void* data, uint32_t size) = 0;
		virtual uint32_t GetStride() const = 0;

		static RefPtr<VertexBuffer> Create(const void* data, const uint32_t size, const uint32_t stride);

	protected:
		VertexBuffer() = default;
		~VertexBuffer() override = default;
	};
}
