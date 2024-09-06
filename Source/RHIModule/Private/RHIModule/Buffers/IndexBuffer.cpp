#include "rhipch.h"

#include "RHIModule/Buffers/IndexBuffer.h"
#include "RHIModule/RHIProxy.h"

namespace Volt::RHI
{
	RefPtr<IndexBuffer> IndexBuffer::Create(std::span<uint32_t> indices)
	{
		return RHIProxy::GetInstance().CreateIndexBuffer(indices);
	}
}
