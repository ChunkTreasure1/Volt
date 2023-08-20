#pragma once

#include "VoltRHI/Core/Core.h"

#include <vector>

namespace Volt::RHI
{
	class BufferView;
	class BufferViewSet
	{
	public:
		BufferViewSet(const std::vector<Ref<BufferView>>& bufferViews);
		~BufferViewSet();

		inline const std::vector<Ref<BufferView>>& GetViews() const { return m_bufferViews; }

		static Ref<BufferViewSet> Create(const std::vector<Ref<BufferView>>& bufferViews);

	private:
		std::vector<Ref<BufferView>> m_bufferViews;
	};
}
