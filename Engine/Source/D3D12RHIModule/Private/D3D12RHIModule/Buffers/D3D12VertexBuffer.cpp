#include "dxpch.h"
#include "D3D12RHIModule/Buffers/D3D12VertexBuffer.h"

#include <RHIModule/Graphics/GraphicsContext.h>

#include <RHIModule/Buffers/CommandBuffer.h>
#include <RHIModule/Memory/Allocation.h>
#include <RHIModule/RHIProxy.h>

#include <RHIModule/Utility/ResourceUtility.h>

#include <CoreUtilities/StringUtility.h>

#include <d3d12/d3d12.h>

namespace Volt::RHI
{
	D3D12VertexBuffer::D3D12VertexBuffer(const void* data, const uint32_t size, const uint32_t stride)
		: m_stride(stride)
	{
		GraphicsContext::GetResourceStateTracker()->AddResource(this, BarrierStage::None, BarrierAccess::None);
		Invalidate(data, size);
	}

	D3D12VertexBuffer::~D3D12VertexBuffer()
	{
		GraphicsContext::GetResourceStateTracker()->RemoveResource(this);
		if (!m_allocation)
		{
			return;
		}

		RHIProxy::GetInstance().DestroyResource([allocation = m_allocation]() 
		{
			GraphicsContext::GetDefaultAllocator()->DestroyBuffer(allocation);
		});

		m_allocation = nullptr;
	}

	void D3D12VertexBuffer::SetData(const void* data, uint32_t size)
	{
		// #TODO_Ivar: Implement
	}

	uint32_t D3D12VertexBuffer::GetStride() const
	{
		return m_stride;
	}

	void D3D12VertexBuffer::SetName(std::string_view name)
	{
		m_name = std::string(name);
		std::wstring wName = Utility::ToWString(name);
		m_allocation->GetResourceHandle<ID3D12Resource*>()->SetName(wName.c_str());
	}

	std::string_view D3D12VertexBuffer::GetName() const
	{
		return m_name;
	}

	const uint64_t D3D12VertexBuffer::GetDeviceAddress() const
	{
		return 0;
	}

	const uint64_t D3D12VertexBuffer::GetByteSize() const
	{
		return m_allocation->GetSize();
	}

	void* D3D12VertexBuffer::GetHandleImpl() const
	{
		return m_allocation->GetResourceHandle<ID3D12Resource*>();
	}

	void D3D12VertexBuffer::Invalidate(const void* data, const uint32_t size)
	{
		RefPtr<Allocation> stagingAllocation;

		if (m_allocation)
		{
			RHIProxy::GetInstance().DestroyResource([allocation = m_allocation]()
			{
				GraphicsContext::GetDefaultAllocator()->DestroyBuffer(allocation);
			});

			m_allocation = nullptr;
		}

		auto allocator = GraphicsContext::GetDefaultAllocator();

		if (data != nullptr)
		{
			stagingAllocation = allocator->CreateBuffer(size, BufferUsage::TransferSrc, MemoryUsage::CPU);

			// Copy to staging buffer
			{
				void* buffData = stagingAllocation->Map<void>();
				memcpy_s(buffData, size, data, size);
				stagingAllocation->Unmap();
			}
		}

		// Create GPU buffer
		{
			m_allocation = allocator->CreateBuffer(size, BufferUsage::VertexBuffer | BufferUsage::TransferDst);
		}

		if (data)
		{
			// Copy from staging buffer to GPU buffer
			{
				RefPtr<CommandBuffer> cmdBuffer = CommandBuffer::Create();
				cmdBuffer->Begin();

				{
					ResourceBarrierInfo barrier{};
					barrier.type = BarrierType::Buffer;
					ResourceUtility::InitializeBarrierSrcFromCurrentState(barrier.bufferBarrier(), this);

					barrier.bufferBarrier().dstAccess = BarrierAccess::CopyDest;
					barrier.bufferBarrier().dstStage = BarrierStage::Copy;
					barrier.bufferBarrier().resource = this;
					barrier.bufferBarrier().offset = 0;
					barrier.bufferBarrier().size = GetByteSize();

					cmdBuffer->ResourceBarrier({ barrier });
				}

				cmdBuffer->CopyBufferRegion(stagingAllocation, 0, m_allocation, 0, size);

				{
					ResourceBarrierInfo barrier{};
					barrier.type = BarrierType::Buffer;
					ResourceUtility::InitializeBarrierSrcFromCurrentState(barrier.bufferBarrier(), this);

					barrier.bufferBarrier().dstAccess = BarrierAccess::VertexBuffer;
					barrier.bufferBarrier().dstStage = BarrierStage::VertexInput;
					barrier.bufferBarrier().resource = this;
					barrier.bufferBarrier().offset = 0;
					barrier.bufferBarrier().size = GetByteSize();

					cmdBuffer->ResourceBarrier({ barrier });
				}

				cmdBuffer->End();
				cmdBuffer->Execute();
			}

			allocator->DestroyBuffer(stagingAllocation);
		}

	}
}
