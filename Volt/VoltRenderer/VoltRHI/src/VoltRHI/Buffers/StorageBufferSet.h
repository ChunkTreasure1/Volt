#pragma once

#include "VoltRHI/Core/Core.h"
#include "VoltRHI/Core/RHICommon.h"

#include <vector>
#include <map>

namespace Volt::RHI
{
	class BufferViewSet;

	class StorageBuffer;
	class StorageBufferSet
	{
	public:
		StorageBufferSet(uint32_t bufferCount);
		~StorageBufferSet();

		inline const Ref<StorageBuffer> Get(uint32_t set, uint32_t binding, uint32_t index) const { return m_storageBuffers.at(set).at(binding).at(index); }
		
		Ref<BufferViewSet> GetBufferViewSet(uint32_t set, uint32_t binding) const;

		template<typename T>
		void Add(uint32_t set, uint32_t binding, uint32_t elementCount = 1, MemoryUsage memoryUsage = MemoryUsage::Default);

		static Ref<StorageBufferSet> Create(uint32_t bufferCount);

	private:
		Ref<StorageBuffer> AddInternal(const uint32_t size, uint32_t elementCount, MemoryUsage memoryUsage);

		uint32_t m_count = 0;
		std::map<uint32_t, std::map<uint32_t, std::vector<Ref<StorageBuffer>>>> m_storageBuffers;
	};

	template<typename T>
	inline void StorageBufferSet::Add(uint32_t set, uint32_t binding, uint32_t elementCount, MemoryUsage memoryUsage)
	{
		for (uint32_t i = 0; i < m_count; i++)
		{
			m_storageBuffers[set][binding].emplace_back(AddInternal(sizeof(T), elementCount, memoryUsage));
		}
	}
}
