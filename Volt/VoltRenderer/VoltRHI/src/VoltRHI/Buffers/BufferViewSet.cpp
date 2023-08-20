#include "rhipch.h"
#include "BufferViewSet.h"

namespace Volt::RHI
{
	BufferViewSet::BufferViewSet(const std::vector<Ref<BufferView>>& bufferViews)
		: m_bufferViews(bufferViews)
	{
	}

	BufferViewSet::~BufferViewSet()
	{
		m_bufferViews.clear();
	}

	Ref<BufferViewSet> BufferViewSet::Create(const std::vector<Ref<BufferView>>& bufferViews)
	{
		return CreateRefRHI<BufferViewSet>(bufferViews);
	}
}
