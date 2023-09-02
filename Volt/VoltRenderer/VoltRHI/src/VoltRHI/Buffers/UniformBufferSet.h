#pragma once

#include "VoltRHI/Core/Core.h"

#include <vector>
#include <map>

namespace Volt::RHI
{
	class BufferViewSet;

	class UniformBuffer;
	class UniformBufferSet
	{
	public:
		UniformBufferSet(uint32_t bufferCount);
		~UniformBufferSet();

		inline const Ref<UniformBuffer> Get(uint32_t set, uint32_t binding, uint32_t index) const { return m_uniformBuffers.at(set).at(binding).at(index); }
		
		Ref<BufferViewSet> GetBufferViewSet(uint32_t set, uint32_t binding) const;

		template<typename T>
		void Add(uint32_t set, uint32_t binding);

		static Ref<UniformBufferSet> Create(uint32_t bufferCount);

	private:
		Ref<UniformBuffer> AddInternal(const uint32_t size);

		uint32_t m_count = 0;
		std::map<uint32_t, std::map<uint32_t, std::vector<Ref<UniformBuffer>>>> m_uniformBuffers;
	};

	template<typename T>
	inline void UniformBufferSet::Add(uint32_t set, uint32_t binding)
	{
		for (uint32_t i = 0; i < m_count; i++)
		{
			m_uniformBuffers[set][binding].emplace_back(AddInternal(sizeof(T)));
		}
	}
}
