#include "dxpch.h"
#include "D3D12Fence.h"

namespace Volt::RHI
{
	D3D12Fence::D3D12Fence(const FenceCreateInfo& createInfo)
	{
		auto* d3d12Device = GraphicsContext::GetDevice()->GetHandle<ID3D12Device2*>();
		VT_D3D12_CHECK(d3d12Device->CreateFence(0, D3D12_FENCE_FLAG_SHARED, VT_D3D12_ID(m_fence)));
		m_windowsFenceEvent = ::CreateEventEx(nullptr, nullptr, false, EVENT_ALL_ACCESS);
	}
	
	D3D12Fence::~D3D12Fence()
	{
	}
	
	void D3D12Fence::Reset() const
	{
	}

	FenceStatus D3D12Fence::GetStatus() const
	{
		return FenceStatus();
	}

	void D3D12Fence::WaitUntilSignaled() const
	{
	}

	void* D3D12Fence::GetHandleImpl() const
	{
		return m_fence.Get();
	}
}
