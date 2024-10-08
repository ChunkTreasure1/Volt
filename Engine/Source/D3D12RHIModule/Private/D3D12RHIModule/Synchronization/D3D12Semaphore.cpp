#include "dxpch.h"
#include "D3D12RHIModule/Synchronization/D3D12Semaphore.h"

namespace Volt::RHI
{
	D3D12Semaphore::D3D12Semaphore(const SemaphoreCreateInfo& info)
	{
		auto* d3d12Device = GraphicsContext::GetDevice()->GetHandle<ID3D12Device2*>();
		VT_D3D12_CHECK(d3d12Device->CreateFence(0, D3D12_FENCE_FLAG_SHARED, VT_D3D12_ID(m_fence)));
		m_windowsFenceEvent = ::CreateEvent(NULL, FALSE, FALSE, NULL);
	}

	D3D12Semaphore::~D3D12Semaphore()
	{
		if (m_windowsFenceEvent)
		{
			::CloseHandle(m_windowsFenceEvent);
		}

		m_fence = nullptr;
	}

	void D3D12Semaphore::Signal(const uint64_t signalValue)
	{
		m_fence->Signal(signalValue);
	}

	void D3D12Semaphore::Wait()
	{
		if (m_fenceValue == 0)
		{
			return;
		}

		if (m_fence->GetCompletedValue() < m_fenceValue)
		{
			m_fence->SetEventOnCompletion(m_fenceValue, m_windowsFenceEvent);
			::WaitForSingleObject(m_windowsFenceEvent, INFINITE);
		}
	}

	const uint64_t D3D12Semaphore::GetValue() const
	{
		return m_fenceValue;
	}

	const uint64_t D3D12Semaphore::IncrementAndGetValue()
	{
		return ++m_fenceValue;
	}

	void* D3D12Semaphore::GetHandleImpl() const
	{
		return m_fence.Get();
	}
}
