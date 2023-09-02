#pragma once

#include <VoltRHI/Buffers/BufferView.h>

struct VkBuffer_T;

namespace Volt::RHI
{
	class VulkanBufferView : public BufferView
	{
	public:
		VulkanBufferView(const BufferViewSpecification& specification);
		
		Ref<RHIResource> GetResource() const { return m_buffer; }

	protected:
		void* GetHandleImpl() const override;

	private:
		Weak<RHIResource> m_buffer;
	};
}
