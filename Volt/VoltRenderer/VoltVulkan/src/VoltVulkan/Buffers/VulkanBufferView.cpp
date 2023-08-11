#include "vkpch.h"
#include "VulkanBufferView.h"

#include "VoltRHI/Core/RHIResource.h"

namespace Volt::RHI
{
	VulkanBufferView::VulkanBufferView(const BufferViewSpecification& specification)
		: m_buffer(specification.bufferResource)
	{
	}

	void* VulkanBufferView::GetHandleImpl()
	{
		return m_buffer.lock()->GetHandle<void*>();
	}
}
