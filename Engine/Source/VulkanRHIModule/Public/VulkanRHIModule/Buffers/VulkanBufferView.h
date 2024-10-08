#pragma once

#include "VulkanRHIModule/Core.h"
#include <RHIModule/Buffers/BufferView.h>

struct VkBuffer_T;

namespace Volt::RHI
{
	class VulkanBufferView : public BufferView
	{
	public:
		VulkanBufferView(const BufferViewSpecification& specification);
		~VulkanBufferView() override = default;

		[[nodiscard]] const uint64_t GetDeviceAddress() const override;

		RHIResource* GetResource() const { return m_buffer; }

	protected:
		void* GetHandleImpl() const override;

	private:
		RHIResource* m_buffer = nullptr;
	};
}
