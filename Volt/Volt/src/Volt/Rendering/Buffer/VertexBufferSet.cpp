#include "vtpch.h"
#include "VertexBufferSet.h"

#include <VoltRHI/Buffers/VertexBuffer.h>

namespace Volt
{
	VertexBufferSet::VertexBufferSet(uint32_t count, const void* data, uint32_t size)
	{
		for (uint32_t i = 0; i < count; i++)
		{
			m_vertexBuffers.emplace_back(RHI::VertexBuffer::Create(data, size));
		}
	}
	
	VertexBufferSet::~VertexBufferSet()
	{
		m_vertexBuffers.clear();
	}

	Ref<VertexBufferSet> VertexBufferSet::Create(uint32_t count, const void* data, uint32_t size)
	{
		return CreateRef<VertexBufferSet>(count, data, size);
	}
}
