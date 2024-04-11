#include <rhipch.h>
#include "IndexBuffer.h"

#include "VoltRHI/RHIProxy.h"

namespace Volt::RHI
{
	Ref<IndexBuffer> IndexBuffer::Create(std::span<uint32_t> indices)
	{
		return RHIProxy::GetInstance().CreateIndexBuffer(indices);
	}
}
