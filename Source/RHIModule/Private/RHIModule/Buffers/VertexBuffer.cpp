#include "rhipch.h"

#include "RHIModule/Buffers/VertexBuffer.h"
#include "RHIModule/RHIProxy.h"

namespace Volt::RHI
{
	RefPtr<VertexBuffer> VertexBuffer::Create(const void* data, const uint32_t size, const uint32_t stride)
	{
		return RHIProxy::GetInstance().CreateVertexBuffer(data, size, stride);
	}
}
