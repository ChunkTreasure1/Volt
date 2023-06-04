#include "vtpch.h"
#include "VertexBufferSet.h"

#include "Volt/Rendering/Buffer/VertexBuffer.h"

namespace Volt
{
	VertexBufferSet::VertexBufferSet(uint32_t count, const void* data, uint32_t size, bool mappable)
	{
		for (uint32_t i = 0; i < count; i++)
		{
			myVertexBuffers.emplace_back(VertexBuffer::Create(data, size, mappable));
		}
	}
	
	VertexBufferSet::~VertexBufferSet()
	{
		myVertexBuffers.clear();
	}

	Ref<VertexBufferSet> VertexBufferSet::Create(uint32_t count, const void* data, uint32_t size, bool mappable)
	{
		return CreateRef<VertexBufferSet>(count, data, size, mappable);
	}
}
