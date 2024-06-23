#include "dxpch.h"
#include "D3D12Allocation.h"

namespace Volt::RHI
{
	D3D12ImageAllocation::D3D12ImageAllocation(const size_t hash)
	{
		m_allocationHash = hash;
	}
	
	void D3D12ImageAllocation::Unmap()
	{
	}
	
	const uint64_t D3D12ImageAllocation::GetDeviceAddress() const
	{
		return 0;
	}
	
	void* D3D12ImageAllocation::GetResourceHandleInternal() const
	{
		return m_resource.Get();
	}
	
	void* D3D12ImageAllocation::MapInternal()
	{
		return nullptr;
	}

	void* D3D12ImageAllocation::GetHandleImpl() const
	{
		return m_allocation;
	}

	D3D12BufferAllocation::D3D12BufferAllocation(const size_t hash)
	{
		m_allocationHash = hash;
	}

	void D3D12BufferAllocation::Unmap()
	{
	}

	const uint64_t D3D12BufferAllocation::GetDeviceAddress() const
	{
		return 0;
	}

	void* D3D12BufferAllocation::GetResourceHandleInternal() const
	{
		return m_resource.Get();
	}

	void* D3D12BufferAllocation::MapInternal()
	{
		return nullptr;
	}

	void* D3D12BufferAllocation::GetHandleImpl() const
	{
		return m_allocation;
	}
}
