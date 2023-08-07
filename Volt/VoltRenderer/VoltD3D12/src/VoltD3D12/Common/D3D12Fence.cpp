#include "dxpch.h"
#include "D3D12Fence.h"

#include "VoltD3D12/Graphics/D3D12GraphicsDevice.h"

namespace Volt::RHI
{
	void D3D12Fence::Create(QueueType type)
	{
		m_type = type;

		auto d3d12device = GraphicsContext::GetDevice();

		VT_D3D12_CHECK(d3d12device->GetHandle<ID3D12Device2*>()->CreateFence(0, D3D12_FENCE_FLAG_SHARED, VT_D3D12_ID(m_fence)));

		m_windowsFenceHandle = CreateEventEx(nullptr, nullptr, false, EVENT_ALL_ACCESS);

		//Niklas: we offset the fence values so that we dont accidentally wait on the wrong queue.
		m_fenceStartValue = static_cast<size_t>(GetD3D12QueueType(type)) << 56;
		m_fenceValue = m_fenceStartValue;
	}
	ID3D12Fence* D3D12Fence::Get()
	{
		return m_fence;
	}

	size_t& D3D12Fence::Value()
	{
		return m_fenceValue;
	}

	size_t D3D12Fence::StartValue()
	{
		return m_fenceStartValue;
	}

	void D3D12Fence::Wait()
	{

		const size_t previousFenceValue = m_fenceValue - 1;

		if (m_fence->GetCompletedValue() < previousFenceValue)
		{
			m_fence->SetEventOnCompletion(previousFenceValue, m_windowsFenceHandle);
			WaitForSingleObject(m_windowsFenceHandle, INFINITE);
		}
	}
	void D3D12Fence::Signal()
	{
		m_fence->Signal(m_fenceValue);
	}
	void D3D12Fence::SignalStart()
	{
		m_fence->Signal(m_fenceStartValue);
	}
}
