#include "rhipch.h"
#include "UniformBuffer.h"

#include "VoltRHI/RHIProxy.h"

namespace Volt::RHI
{
	RefPtr<UniformBuffer> UniformBuffer::Create(const uint32_t size, const void* data)
	{
		return RHIProxy::GetInstance().CreateUniformBuffer(size, data);
	}
}
