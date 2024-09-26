#pragma once

#include <RHIModule/Synchronization/Fence.h>

struct VkFence_T;

namespace Volt::RHI
{
	class VulkanFence : public Fence
	{
	public:
		VulkanFence(const FenceCreateInfo& createInfo);
		~VulkanFence() override;

		void Reset() const override;
		FenceStatus GetStatus() const override;
		void WaitUntilSignaled() const override;

	protected:
		void* GetHandleImpl() const override;

	private:
		VkFence_T* m_fence = nullptr;
	};
}
