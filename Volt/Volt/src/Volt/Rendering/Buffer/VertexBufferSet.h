#pragma once

#include "Volt/Core/Base.h"

#include <vector>

namespace Volt
{
	class VertexBuffer;
	class VertexBufferSet
	{
	public:
		VertexBufferSet(uint32_t count, const void* data, uint32_t size, bool mappable);
		~VertexBufferSet();

		inline const Ref<VertexBuffer> Get(uint32_t index) const { return myVertexBuffers.at(index); }

		static Ref<VertexBufferSet> Create(uint32_t count, const void* data, uint32_t size, bool mappable = false);

	private:
		std::vector<Ref<VertexBuffer>> myVertexBuffers;
	};
}
