#pragma once

#include "Volt/Core/Base.h"

#include <cstdint>
#include <vector>

namespace Volt
{
	class UniformBuffer;
	class UniformBufferSet
	{
	public:
		UniformBufferSet(uint32_t bufferCount);
		~UniformBufferSet();

		inline const Ref<UniformBuffer> Get(uint32_t set, uint32_t binding, uint32_t index) const { return myUniformBuffers.at(set).at(binding).at(index); }

		template<typename T>
		void Add(uint32_t set, uint32_t binding);

		static Ref<UniformBufferSet> Create(uint32_t bufferCount);

	private:
		uint32_t myCount = 0;
		std::map<uint32_t, std::map<uint32_t, std::vector<Ref<UniformBuffer>>>> myUniformBuffers;
	};

	template<typename T>
	inline void UniformBufferSet::Add(uint32_t set, uint32_t binding)
	{
		for (uint32_t i = 0; i < myCount; i++)
		{
			myUniformBuffers[set][binding].emplace_back(UniformBuffer::Create(sizeof(T), nullptr));
		}
	}
}