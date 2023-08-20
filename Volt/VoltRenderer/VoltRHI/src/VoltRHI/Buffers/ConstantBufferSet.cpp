#include "rhipch.h"
#include "ConstantBufferSet.h"

#include "VoltRHI/Buffers/ConstantBuffer.h"
#include "VoltRHI/Buffers/BufferViewSet.h"

namespace Volt::RHI
{
	ConstantBufferSet::ConstantBufferSet(uint32_t bufferCount)
		: m_count(bufferCount)
	{
	}
	
	ConstantBufferSet::~ConstantBufferSet()
	{
		m_constantBuffers.clear();
	}

	Ref<BufferViewSet> ConstantBufferSet::GetBufferViewSet(uint32_t set, uint32_t binding) const
	{
		std::vector<Ref<BufferView>> views;

		for (const auto& buffer : m_constantBuffers.at(set).at(binding))
		{
			views.push_back(buffer->GetView());
		}

		Ref<BufferViewSet> viewSet = BufferViewSet::Create(views);
		return viewSet;
	}

	Ref<ConstantBufferSet> ConstantBufferSet::Create(uint32_t bufferCount)
	{
		return CreateRefRHI<ConstantBufferSet>(bufferCount);
	}

	Ref<ConstantBuffer> ConstantBufferSet::AddInternal(const uint32_t size)
	{
		return ConstantBuffer::Create(size, nullptr);
	}
}
