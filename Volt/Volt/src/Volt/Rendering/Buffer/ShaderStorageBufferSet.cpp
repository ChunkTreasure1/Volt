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

	void ShaderStorageBufferSet::ResizeWithElementCount(uint32_t set, uint32_t binding, uint32_t newElementCount)
	{
		for (auto& buffer : myShaderStorageBuffers[set][binding])
		{
			buffer->ResizeWithElementCount(newElementCount);
		}
	}

	Ref<ShaderStorageBufferSet> ShaderStorageBufferSet::Create(uint32_t bufferCount)
	{
		return CreateRef<ShaderStorageBufferSet>(bufferCount);
	}
}
