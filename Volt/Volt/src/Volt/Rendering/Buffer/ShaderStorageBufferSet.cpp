#include "vtpch.h"
#include "ShaderStorageBufferSet.h"

namespace Volt
{
	ShaderStorageBufferSet::ShaderStorageBufferSet(uint32_t bufferCount)
		: myCount(bufferCount)
	{}

	ShaderStorageBufferSet::~ShaderStorageBufferSet()
	{
		myShaderStorageBuffers.clear();
	}

	Ref<ShaderStorageBufferSet> ShaderStorageBufferSet::Create(uint32_t bufferCount)
	{
		return CreateRef<ShaderStorageBufferSet>(bufferCount);
	}
}