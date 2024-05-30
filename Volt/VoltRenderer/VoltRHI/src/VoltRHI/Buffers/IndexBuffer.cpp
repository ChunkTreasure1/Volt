#include <rhipch.h>
#include "IndexBuffer.h"

#include "VoltRHI/RHIProxy.h"

namespace Volt::RHI
{
	RefPtr<IndexBuffer> IndexBuffer::Create(std::span<uint32_t> indices)
	{
		return RHIProxy::GetInstance().CreateIndexBuffer(indices);
	}
}
