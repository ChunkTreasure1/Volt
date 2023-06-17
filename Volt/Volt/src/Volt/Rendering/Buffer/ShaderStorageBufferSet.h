#pragma once

#include "Volt/Core/Base.h"
#include "Volt/Rendering/Buffer/ShaderStorageBuffer.h"

#include <cstdint>
#include <vector>
#include <map>

namespace Volt
{
	class ShaderStorageBuffer;
	class ShaderStorageBufferSet
	{
	public:
		ShaderStorageBufferSet(uint32_t bufferCount);
		~ShaderStorageBufferSet();

		inline const Ref<ShaderStorageBuffer> Get(uint32_t set, uint32_t binding, uint32_t index) const { return myShaderStorageBuffers.at(set).at(binding).at(index); }
		inline bool HasBufferAt(uint32_t set, uint32_t binding) const { return myShaderStorageBuffers.contains(set) && myShaderStorageBuffers.at(set).contains(binding); }

		void ResizeWithElementCount(uint32_t set, uint32_t binding, uint32_t newElementCount);

		template<typename T>
		void Add(uint32_t set, uint32_t binding, uint32_t count, MemoryUsage usage = MemoryUsage::None);

		static Ref<ShaderStorageBufferSet> Create(uint32_t bufferCount);

	private:
		uint32_t myCount = 0;
		std::map<uint32_t, std::map<uint32_t, std::vector<Ref<ShaderStorageBuffer>>>> myShaderStorageBuffers;
	};

	template<typename T>
	inline void ShaderStorageBufferSet::Add(uint32_t set, uint32_t binding, uint32_t count, MemoryUsage usage)
	{
		for (uint32_t i = 0; i < myCount; i++)
		{
			myShaderStorageBuffers[set][binding].emplace_back(ShaderStorageBuffer::Create(sizeof(T), count, usage));
		}
	}
}
