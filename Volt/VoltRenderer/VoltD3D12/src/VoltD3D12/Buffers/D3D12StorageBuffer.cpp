#include "dxpch.h"
#include "D3D12StorageBuffer.h"

#include <VoltRHI/Graphics/GraphicsContext.h>

#include <VoltRHI/Buffers/CommandBuffer.h>
#include <VoltRHI/Buffers/BufferView.h>

#include <VoltRHI/Memory/Allocation.h>
#include <VoltRHI/Memory/MemoryCommon.h>
#include <VoltRHI/RHIProxy.h>

#include <CoreUtilities/StringUtility.h>

namespace Volt::RHI
{
	D3D12StorageBuffer::D3D12StorageBuffer(const uint32_t count, const size_t elementSize, std::string_view name, BufferUsage bufferUsage, MemoryUsage memoryUsage)
		: m_byteSize(count* elementSize), m_size(count), m_elementSize(elementSize), m_bufferUsage(bufferUsage), m_memoryUsage(memoryUsage), m_name(name)
	{
		Invalidate(elementSize * count);
		SetName(name);
	}

	D3D12StorageBuffer::D3D12StorageBuffer(const size_t size, std::string_view name, BufferUsage bufferUsage, MemoryUsage memoryUsage)
		: m_bufferUsage(bufferUsage), m_memoryUsage(memoryUsage), m_name(name)
	{
		Invalidate(size);
		SetName(name);
	}

	D3D12StorageBuffer::D3D12StorageBuffer(const size_t size, RefPtr<Allocator> customAllocator, std::string_view name, BufferUsage bufferUsage, MemoryUsage memoryUsage)
		: m_allocatedUsingCustomAllocator(true), m_customAllocator(customAllocator), m_bufferUsage(bufferUsage), m_memoryUsage(memoryUsage), m_name(name)
	{
		Invalidate(size);
		SetName(name);
	}

	D3D12StorageBuffer::~D3D12StorageBuffer()
	{
		Release();
	}

	void D3D12StorageBuffer::ResizeByteSize(const size_t byteSize)
	{
		Invalidate(byteSize);
		SetName(m_name);
	}

	void D3D12StorageBuffer::Resize(const uint32_t size)
	{
		const size_t newSize = size * m_elementSize;
		Invalidate(newSize);

		SetName(m_name);
	}

	const size_t D3D12StorageBuffer::GetElementSize() const
	{
		return m_elementSize;
	}

	const size_t D3D12StorageBuffer::GetSize() const
	{
		return m_byteSize;
	}

	const uint32_t D3D12StorageBuffer::GetCount() const
	{
		return m_size;
	}

	WeakPtr<Allocation> D3D12StorageBuffer::GetAllocation() const
	{
		return m_allocation;
	}

	void D3D12StorageBuffer::Unmap()
	{
		m_allocation->Unmap();
	}

	void D3D12StorageBuffer::SetData(const void* data, const size_t size)
	{
		RefPtr<Allocation> stagingAllocation = nullptr;

		if (m_allocatedUsingCustomAllocator)
		{
			stagingAllocation = m_customAllocator->CreateBuffer(size, BufferUsage::TransferSrc, MemoryUsage::CPUToGPU);
		}
		else
		{
			stagingAllocation = GraphicsContext::GetDefaultAllocator().CreateBuffer(size, BufferUsage::TransferSrc, MemoryUsage::CPUToGPU);
		}

		void* mappedPtr = stagingAllocation->Map<void>();
		memcpy_s(mappedPtr, size, data, size);
		stagingAllocation->Unmap();

		RefPtr<CommandBuffer> cmdBuffer = CommandBuffer::Create();
		cmdBuffer->Begin();

		ResourceBarrierInfo barrier{};
		barrier.type = BarrierType::Buffer;
		barrier.bufferBarrier().srcStage = BarrierStage::ComputeShader | BarrierStage::VertexShader | BarrierStage::PixelShader;
		barrier.bufferBarrier().srcAccess = BarrierAccess::None;
		barrier.bufferBarrier().dstStage = BarrierStage::Copy;
		barrier.bufferBarrier().dstAccess = BarrierAccess::TransferDestination;
		barrier.bufferBarrier().offset = 0;
		barrier.bufferBarrier().size = size;
		barrier.bufferBarrier().resource = WeakPtr<D3D12StorageBuffer>(this);

		cmdBuffer->ResourceBarrier({ barrier });

		cmdBuffer->CopyBufferRegion(stagingAllocation, 0, m_allocation, 0, size);

		barrier.bufferBarrier().srcStage = BarrierStage::Copy;
		barrier.bufferBarrier().srcAccess = BarrierAccess::TransferDestination;
		barrier.bufferBarrier().dstStage = BarrierStage::ComputeShader | BarrierStage::VertexShader | BarrierStage::PixelShader;
		barrier.bufferBarrier().dstAccess = BarrierAccess::None;

		cmdBuffer->ResourceBarrier({ barrier });

		cmdBuffer->End();
		cmdBuffer->Execute();

		RHIProxy::GetInstance().DestroyResource([allocatedUsingCustomAllocator = m_allocatedUsingCustomAllocator, customAllocator = m_customAllocator, allocation = stagingAllocation]()
		{
			if (allocatedUsingCustomAllocator)
			{
				customAllocator->DestroyBuffer(allocation);
			}
			else
			{
				GraphicsContext::GetDefaultAllocator().DestroyBuffer(allocation);
			}
		});
	}

	void D3D12StorageBuffer::SetData(RefPtr<CommandBuffer> commandBuffer, const void* data, const size_t size)
	{
		RefPtr<Allocation> stagingAllocation = nullptr;

		if (m_allocatedUsingCustomAllocator)
		{
			stagingAllocation = m_customAllocator->CreateBuffer(size, BufferUsage::TransferSrc, MemoryUsage::CPUToGPU);
		}
		else
		{
			stagingAllocation = GraphicsContext::GetDefaultAllocator().CreateBuffer(size, BufferUsage::TransferSrc, MemoryUsage::CPUToGPU);
		}

		void* mappedPtr = stagingAllocation->Map<void>();
		memcpy_s(mappedPtr, m_byteSize, data, size);
		stagingAllocation->Unmap();

		ResourceBarrierInfo barrier{};
		barrier.bufferBarrier().srcStage = BarrierStage::All;
		barrier.bufferBarrier().srcAccess = BarrierAccess::None;
		barrier.bufferBarrier().dstStage = BarrierStage::Copy;
		barrier.bufferBarrier().dstAccess = BarrierAccess::TransferDestination;
		barrier.bufferBarrier().offset = 0;
		barrier.bufferBarrier().size = size;
		barrier.bufferBarrier().resource = WeakPtr<D3D12StorageBuffer>(this);

		commandBuffer->ResourceBarrier({ barrier });

		commandBuffer->CopyBufferRegion(stagingAllocation, 0, m_allocation, 0, size);

		barrier.bufferBarrier().srcStage = BarrierStage::Copy;
		barrier.bufferBarrier().srcAccess = BarrierAccess::TransferDestination;
		barrier.bufferBarrier().dstStage = BarrierStage::All;
		barrier.bufferBarrier().dstAccess = BarrierAccess::None;

		commandBuffer->ResourceBarrier({ barrier });

		RHIProxy::GetInstance().DestroyResource([allocatedUsingCustomAllocator = m_allocatedUsingCustomAllocator, customAllocator = m_customAllocator, allocation = stagingAllocation]()
		{
			if (allocatedUsingCustomAllocator)
			{
				customAllocator->DestroyBuffer(allocation);
			}
			else
			{
				GraphicsContext::GetDefaultAllocator().DestroyBuffer(allocation);
			}
		});
	}
	
	RefPtr<BufferView> D3D12StorageBuffer::GetView()
	{
		if (m_view)
		{
			return m_view;
		}

		BufferViewSpecification spec{};
		spec.bufferResource = this;

		m_view = BufferView::Create(spec);
		return m_view;
	}

	void D3D12StorageBuffer::SetName(std::string_view name)
	{
		if (!m_allocation)
		{
			return;
		}

		std::wstring str = Utility::ToWString(name);
		m_allocation->GetResourceHandle<ID3D12Resource*>()->SetName(str.c_str());
	}

	const uint64_t D3D12StorageBuffer::GetDeviceAddress() const
	{
		return 0;
	}

	const uint64_t D3D12StorageBuffer::GetByteSize() const
	{
		return m_allocation->GetSize();
	}

	void* D3D12StorageBuffer::GetHandleImpl() const
	{
		return m_allocation->GetResourceHandle<ID3D12Resource*>();
	}

	void* D3D12StorageBuffer::MapInternal()
	{
		return m_allocation->Map<void>();
	}

	void D3D12StorageBuffer::Invalidate(const size_t byteSize)
	{
		Release();
		m_byteSize = std::max(byteSize, Memory::GetMinBufferAllocationSize());

		if (m_allocatedUsingCustomAllocator)
		{
			m_allocation = m_customAllocator->CreateBuffer(byteSize, m_bufferUsage | BufferUsage::TransferDst | BufferUsage::StorageBuffer, m_memoryUsage);
		}
		else
		{
			m_allocation = GraphicsContext::GetDefaultAllocator().CreateBuffer(byteSize, m_bufferUsage | BufferUsage::TransferDst | BufferUsage::StorageBuffer, m_memoryUsage);
		}
	}

	void D3D12StorageBuffer::Release()
	{
		if (!m_allocation)
		{
			return;
		}

		RHIProxy::GetInstance().DestroyResource([allocatedUsingCustomAllocator = m_allocatedUsingCustomAllocator, customAllocator = m_customAllocator, allocation = m_allocation]()
		{
			if (allocatedUsingCustomAllocator)
			{
				customAllocator->DestroyBuffer(allocation);
			}
			else
			{
				GraphicsContext::GetDefaultAllocator().DestroyBuffer(allocation);
			}
		});

		m_allocation = nullptr;
	}
}

