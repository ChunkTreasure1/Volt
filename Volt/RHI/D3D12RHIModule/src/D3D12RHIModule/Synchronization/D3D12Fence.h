#pragma once

#include "D3D12RHIModule/Common/ComPtr.h"

#include <RHIModule/Synchronization/Fence.h>

struct ID3D12Fence;

namespace Volt::RHI
{
	class D3D12Fence : public Fence
	{
	public:
		D3D12Fence(const FenceCreateInfo& createInfo);
		~D3D12Fence() override;

		void Reset() const override;
		FenceStatus GetStatus() const override;
		void WaitUntilSignaled() const override;

	protected:
		void* GetHandleImpl() const override;

	private:
		ComPtr<ID3D12Fence> m_fence;
		void* m_windowsFenceEvent = nullptr;
		uint64_t m_fenceValue = 0;
	};
}
