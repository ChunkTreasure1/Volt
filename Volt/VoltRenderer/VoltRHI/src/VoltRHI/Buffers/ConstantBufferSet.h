#pragma once

#include "VoltRHI/Core/Core.h"

#include <vector>
#include <map>

namespace Volt::RHI
{
	class BufferViewSet;

	class ConstantBuffer;
	class ConstantBufferSet
	{
	public:
		ConstantBufferSet(uint32_t bufferCount);
		~ConstantBufferSet();

		inline const Ref<ConstantBuffer> Get(uint32_t set, uint32_t binding, uint32_t index) const { return m_constantBuffers.at(set).at(binding).at(index); }
		
		Ref<BufferViewSet> GetBufferViewSet(uint32_t set, uint32_t binding) const;

		template<typename T>
		void Add(uint32_t set, uint32_t binding);

		static Ref<ConstantBufferSet> Create(uint32_t bufferCount);

	private:
		Ref<ConstantBuffer> AddInternal(const uint32_t size);

		uint32_t m_count = 0;
		std::map<uint32_t, std::map<uint32_t, std::vector<Ref<ConstantBuffer>>>> m_constantBuffers;
	};

	template<typename T>
	inline void ConstantBufferSet::Add(uint32_t set, uint32_t binding)
	{
		for (uint32_t i = 0; i < m_count; i++)
		{
			m_constantBuffers[set][binding].emplace_back(AddInternal(sizeof(T)));
		}
	}
}
