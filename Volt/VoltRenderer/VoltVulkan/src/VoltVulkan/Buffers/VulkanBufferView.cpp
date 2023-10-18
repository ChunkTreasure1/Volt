#include "vkpch.h"
#include "VulkanBufferView.h"

#include "VoltRHI/Core/RHIResource.h"

namespace Volt::RHI
{
	VulkanBufferView::VulkanBufferView(const BufferViewSpecification& specification)
		: m_buffer(specification.bufferResource)
	{
	}

	const uint64_t VulkanBufferView::GetDeviceAddress() const
	{
		return m_buffer->GetDeviceAddress();
	}

	void* VulkanBufferView::GetHandleImpl() const
	{
		return m_buffer->GetHandle<void*>();
	}
}
