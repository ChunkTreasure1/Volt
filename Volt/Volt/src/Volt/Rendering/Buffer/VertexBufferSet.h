#pragma once

#include "Volt/Core/Base.h"

#include <vector>

namespace Volt
{
	namespace RHI
	{
		class VertexBuffer;
	}

	class VertexBufferSet
	{
	public:
		VertexBufferSet(uint32_t count, const void* data, uint32_t size);
		~VertexBufferSet();

		inline const Ref<RHI::VertexBuffer> Get(uint32_t index) const { return m_vertexBuffers.at(index); }

		static Ref<VertexBufferSet> Create(uint32_t count, const void* data, uint32_t size);

	private:
		std::vector<Ref<RHI::VertexBuffer>> m_vertexBuffers;
	};
}
