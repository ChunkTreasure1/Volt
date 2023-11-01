#include "rhipch.h"
#include "StorageBufferSet.h"

#include "VoltRHI/Buffers/StorageBuffer.h"
#include "VoltRHI/Buffers/BufferViewSet.h"

namespace Volt::RHI
{
	StorageBufferSet::StorageBufferSet(uint32_t bufferCount)
		: m_count(bufferCount)
	{
	}
	
	StorageBufferSet::~StorageBufferSet()
	{
		m_storageBuffers.clear();
	}

	Ref<BufferViewSet> StorageBufferSet::GetBufferViewSet(uint32_t set, uint32_t binding) const
	{
		std::vector<Ref<BufferView>> views;

		for (const auto& buffer : m_storageBuffers.at(set).at(binding))
		{
			views.push_back(buffer->GetView());
		}

		Ref<BufferViewSet> viewSet = BufferViewSet::Create(views);
		return viewSet;
	}

	Ref<StorageBufferSet> StorageBufferSet::Create(uint32_t bufferCount)
	{
		return CreateRefRHI<StorageBufferSet>(bufferCount);
	}

	Ref<StorageBuffer> StorageBufferSet::AddInternal(const uint32_t size, uint32_t elementCount, BufferUsage memoryUsage)
	{
		return StorageBuffer::Create(elementCount, size, "", memoryUsage);
	}
}
