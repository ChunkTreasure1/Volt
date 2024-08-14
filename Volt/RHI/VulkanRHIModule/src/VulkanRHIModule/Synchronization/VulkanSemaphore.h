#pragma once

#include <RHIModule/Synchronization/Semaphore.h>

struct VkSemaphore_T;

namespace Volt::RHI
{
	class VulkanSemaphore : public Semaphore
	{
	public:
		VulkanSemaphore(const SemaphoreCreateInfo& info);
		~VulkanSemaphore() override;

		void Signal(const uint64_t signalValue) override;
		void Wait() override;

		const uint64_t GetValue() const override;
		const uint64_t IncrementAndGetValue() override;

	protected:
		void* GetHandleImpl() const override;

	private:
		VkSemaphore_T* m_semaphore = nullptr;
		uint64_t m_currentValue = 0;
	};
}
