#include "rhipch.h"
#include "UniformBuffer.h"

#include "VoltRHI/RHIProxy.h"

namespace Volt::RHI
{
	RefPtr<UniformBuffer> UniformBuffer::Create(const uint32_t size, const void* data, const uint32_t count, std::string_view name)
	{
		return RHIProxy::GetInstance().CreateUniformBuffer(size, data, count, name);
	}
}
