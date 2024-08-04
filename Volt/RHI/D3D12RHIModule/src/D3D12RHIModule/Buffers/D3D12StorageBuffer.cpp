#include "dxpch.h"
#include "D3D12StorageBuffer.h"

#include <RHIModule/Graphics/GraphicsContext.h>

#include <RHIModule/Buffers/CommandBuffer.h>
#include <RHIModule/Buffers/BufferView.h>

#include <RHIModule/Memory/Allocation.h>
#include <RHIModule/Memory/MemoryCommon.h>
#include <RHIModule/RHIProxy.h>

#include <RHIModule/Utility/ResourceUtility.h>

#include <CoreUtilities/StringUtility.h>

namespace Volt::RHI
{
	D3D12StorageBuffer::D3D12StorageBuffer(uint32_t count, uint64_t elementSize, std::string_view name, BufferUsage bufferUsage, MemoryUsage memoryUsage, RefPtr<Allocator> allocator)
		: m_count(count), m_elementSize(elementSize), m_name(name), m_bufferUsage(bufferUsage), m_memoryUsage(memoryUsage), m_allocator(allocator)
	{
		GraphicsContext::GetResourceStateTracker()->AddResource(this, BarrierStage::None, BarrierAccess::None);

		if (!m_allocator)
		{
			m_allocator = GraphicsContext::GetDefaultAllocator();
		}

		Invalidate(m_elementSize * m_count);
		SetName(m_name);
	}

	D3D12StorageBuffer::~D3D12StorageBuffer()
	{
		GraphicsContext::GetResourceStateTracker()->RemoveResource(this);
		Release();
	}

	void D3D12StorageBuffer::Resize(const uint64_t byteSize)
	{
		m_count = static_cast<uint32_t>(byteSize / 4);

		Invalidate(byteSize);
		SetName(m_name);
	}

	void D3D12StorageBuffer::ResizeWithCount(const uint32_t count)
	{
		m_count = count;
		const uint64_t newSize = m_count * m_elementSize;

		Invalidate(newSize);
		SetName(m_name);
	}

	const size_t D3D12StorageBuffer::GetElementSize() const
	{
		return m_elementSize;
	}

	const uint32_t D3D12StorageBuffer::GetCount() const
	{
		return m_count;
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
		RefPtr<Allocation> stagingAllocation = m_allocator->CreateBuffer(size, BufferUsage::TransferSrc, MemoryUsage::CPUToGPU);;

		void* mappedPtr = stagingAllocation->Map<void>();
		memcpy_s(mappedPtr, size, data, size);
		stagingAllocation->Unmap();

		RefPtr<CommandBuffer> cmdBuffer = CommandBuffer::Create();
		cmdBuffer->Begin();

		{
			ResourceBarrierInfo barrier{};
			barrier.type = BarrierType::Buffer;
			ResourceUtility::InitializeBarrierSrcFromCurrentState(barrier.bufferBarrier(), this);

			barrier.bufferBarrier().dstStage = BarrierStage::Copy;
			barrier.bufferBarrier().dstAccess = BarrierAccess::CopyDest;
			barrier.bufferBarrier().offset = 0;
			barrier.bufferBarrier().size = size;
			barrier.bufferBarrier().resource = this;
			
			cmdBuffer->ResourceBarrier({ barrier });
		}

		cmdBuffer->CopyBufferRegion(stagingAllocation, 0, m_allocation, 0, size);

		{
			ResourceBarrierInfo barrier{};
			barrier.type = BarrierType::Buffer;
			ResourceUtility::InitializeBarrierSrcFromCurrentState(barrier.bufferBarrier(), this);

			barrier.bufferBarrier().dstStage = BarrierStage::ComputeShader | BarrierStage::VertexShader | BarrierStage::PixelShader;
			barrier.bufferBarrier().dstAccess = BarrierAccess::ShaderRead;
			barrier.bufferBarrier().offset = 0;
			barrier.bufferBarrier().size = size;
			barrier.bufferBarrier().resource = this;

			cmdBuffer->ResourceBarrier({ barrier });
		}

		cmdBuffer->End();
		cmdBuffer->Execute();

		RHIProxy::GetInstance().DestroyResource([allocator = m_allocator, allocation = stagingAllocation]()
		{
			allocator->DestroyBuffer(allocation);
		});
	}

	void D3D12StorageBuffer::SetData(RefPtr<CommandBuffer> commandBuffer, const void* data, const size_t size)
	{
		RefPtr<Allocation> stagingAllocation = m_allocator->CreateBuffer(size, BufferUsage::TransferSrc, MemoryUsage::CPUToGPU);

		void* mappedPtr = stagingAllocation->Map<void>();
		memcpy_s(mappedPtr, m_byteSize, data, size);
		stagingAllocation->Unmap();

		{
			const auto& currentState = GraphicsContext::GetResourceStateTracker()->GetCurrentResourceState(this);

			ResourceBarrierInfo barrier{};
			barrier.type = BarrierType::Buffer;
			barrier.bufferBarrier().srcStage = currentState.stage;
			barrier.bufferBarrier().srcAccess = currentState.access;
			barrier.bufferBarrier().dstStage = BarrierStage::Copy;
			barrier.bufferBarrier().dstAccess = BarrierAccess::CopyDest;
			barrier.bufferBarrier().offset = 0;
			barrier.bufferBarrier().size = size;
			barrier.bufferBarrier().resource = this;

			commandBuffer->ResourceBarrier({ barrier });
		}

		commandBuffer->CopyBufferRegion(stagingAllocation, 0, m_allocation, 0, size);

		{
			const auto& currentState = GraphicsContext::GetResourceStateTracker()->GetCurrentResourceState(this);

			ResourceBarrierInfo barrier{};
			barrier.type = BarrierType::Buffer;
			barrier.bufferBarrier().srcStage = currentState.stage;
			barrier.bufferBarrier().srcAccess = currentState.access;
			barrier.bufferBarrier().dstStage = BarrierStage::ComputeShader | BarrierStage::VertexShader | BarrierStage::PixelShader;
			barrier.bufferBarrier().dstAccess = BarrierAccess::ShaderRead;
			barrier.bufferBarrier().offset = 0;
			barrier.bufferBarrier().size = size;
			barrier.bufferBarrier().resource = this;

			commandBuffer->ResourceBarrier({ barrier });
		}

		RHIProxy::GetInstance().DestroyResource([allocator = m_allocator, allocation = stagingAllocation]()
		{
			allocator->DestroyBuffer(allocation);
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
		m_allocation = m_allocator->CreateBuffer(byteSize, m_bufferUsage | BufferUsage::TransferDst | BufferUsage::StorageBuffer, m_memoryUsage);
	}

	void D3D12StorageBuffer::Release()
	{
		if (!m_allocation)
		{
			return;
		}

		RHIProxy::GetInstance().DestroyResource([allocator = m_allocator, allocation = m_allocation]()
		{
			allocator->DestroyBuffer(allocation);
		});

		m_allocation = nullptr;
	}
}

