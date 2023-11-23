#include "rhipch.h"
#include "UniformBufferSet.h"

#include "VoltRHI/Buffers/UniformBuffer.h"
#include "VoltRHI/Buffers/BufferViewSet.h"

namespace Volt::RHI
{
	UniformBufferSet::UniformBufferSet(uint32_t bufferCount)
		: m_count(bufferCount)
	{
	}
	
	UniformBufferSet::~UniformBufferSet()
	{
		m_uniformBuffers.clear();
	}

	Ref<BufferViewSet> UniformBufferSet::GetBufferViewSet(uint32_t set, uint32_t binding) const
	{
		std::vector<Ref<BufferView>> views;

		for (const auto& buffer : m_uniformBuffers.at(set).at(binding))
		{
			views.push_back(buffer->GetView());
		}

		Ref<BufferViewSet> viewSet = BufferViewSet::Create(views);
		return viewSet;
	}

	Ref<UniformBufferSet> UniformBufferSet::Create(uint32_t bufferCount)
	{
		return CreateRef<UniformBufferSet>(bufferCount);
	}

	Ref<UniformBuffer> UniformBufferSet::AddInternal(const uint32_t size)
	{
		return UniformBuffer::Create(size, nullptr);
	}
}
