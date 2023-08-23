#pragma once

#include <VoltRHI/Images/SamplerState.h>

struct VkSampler_T;

namespace Volt::RHI
{
	class VulkanSamplerState : public SamplerState
	{
	public:
		VulkanSamplerState(const SamplerStateCreateInfo& createInfo);
		~VulkanSamplerState() override;

	protected:
		void* GetHandleImpl() override;

	private:
		VkSampler_T* m_sampler = nullptr;
	};
}
