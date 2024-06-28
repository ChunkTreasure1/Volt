#pragma once

#include "VoltD3D12/Common/ComPtr.h"

#include <VoltRHI/Synchronization/Semaphore.h>

struct ID3D12Fence;

namespace Volt::RHI
{
	class D3D12Semaphore : public Semaphore
	{
	public:
		D3D12Semaphore(const SemaphoreCreateInfo& info);
		~D3D12Semaphore() override;

		void Signal(const uint64_t signalValue) override;
		void Wait() override;

		const uint64_t GetValue() const override;
		const uint64_t IncrementAndGetValue() override;

	protected:
		void* GetHandleImpl() const override;

	private:
		ComPtr<ID3D12Fence> m_fence;
		void* m_windowsFenceEvent = nullptr;
		uint64_t m_fenceValue = 0;
	};
}
