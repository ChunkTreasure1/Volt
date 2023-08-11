#pragma once

#include <VoltRHI/Buffers/BufferView.h>

struct VkBuffer_T;

namespace Volt::RHI
{
	class VulkanBufferView : public BufferView
	{
	public:
		VulkanBufferView(const BufferViewSpecification& specification);
		
	protected:
		void* GetHandleImpl() override;

	private:
		Weak<RHIResource> m_buffer;
	};
}
