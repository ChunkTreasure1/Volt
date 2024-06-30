#include "rhipch.h"
#include "VertexBuffer.h"

#include "VoltRHI/RHIProxy.h"

namespace Volt::RHI
{
	RefPtr<VertexBuffer> VertexBuffer::Create(const uint32_t size, const void* data)
	{
		return RHIProxy::GetInstance().CreateVertexBuffer(size, data);
	}
}
