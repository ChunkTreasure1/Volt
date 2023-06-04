#include "vtpch.h"
#include "UniformBufferSet.h"

#include "Volt/Rendering/Buffer/UniformBuffer.h"

namespace Volt
{
	UniformBufferSet::UniformBufferSet(uint32_t bufferCount)
		: myCount(bufferCount)
	{
	}

	UniformBufferSet::~UniformBufferSet()
	{
		myUniformBuffers.clear();
	}

	Ref<UniformBufferSet> UniformBufferSet::Create(uint32_t bufferCount)
	{
		return CreateRef<UniformBufferSet>(bufferCount);
	}
}