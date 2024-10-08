#include "dxpch.h"
#include "D3D12RHIModule/Buffers/D3D12IndexBuffer.h"

#include <RHIModule/Buffers/CommandBuffer.h>
#include <RHIModule/Memory/Allocation.h>
#include <RHIModule/RHIProxy.h>
#include <RHIModule/Utility/ResourceUtility.h>

#include <CoreUtilities/StringUtility.h>

namespace Volt::RHI
{
	D3D12IndexBuffer::D3D12IndexBuffer(std::span<const uint32_t> indices)
		: m_count(static_cast<uint32_t>(indices.size()))
	{
		GraphicsContext::GetResourceStateTracker()->AddResource(this, BarrierStage::None, BarrierAccess::None);
		SetData(indices.data(), static_cast<uint32_t>(sizeof(uint32_t) * indices.size()));
	}

	D3D12IndexBuffer::~D3D12IndexBuffer()
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
	
	const uint32_t D3D12IndexBuffer::GetCount() const
	{
		return m_count;
	}
	
	void D3D12IndexBuffer::SetName(std::string_view name)
	{
		m_name = std::string(name);
		std::wstring wName = Utility::ToWString(name);
		m_allocation->GetResourceHandle<ID3D12Resource*>()->SetName(wName.c_str());
	}

	std::string_view D3D12IndexBuffer::GetName() const
	{
		return m_name;
	}
	
	const uint64_t D3D12IndexBuffer::GetDeviceAddress() const
	{
		return 0;
	}
	
	const uint64_t D3D12IndexBuffer::GetByteSize() const
	{
		return m_allocation->GetSize();
	}
	
	void* D3D12IndexBuffer::GetHandleImpl() const
	{
		return m_allocation->GetResourceHandle<ID3D12Resource*>();
	}
	
	void D3D12IndexBuffer::SetData(const void* data, const uint32_t size)
	{
		RefPtr<Allocation> stagingAllocation;

		uint64_t bufferSize = size;

		if (m_allocation)
		{
			RHIProxy::GetInstance().DestroyResource([allocation = m_allocation]()
			{
				GraphicsContext::GetDefaultAllocator()->DestroyBuffer(allocation);
			});

			m_allocation = nullptr;
		}

		auto allocator = GraphicsContext::GetDefaultAllocator();

		if (data)
		{
			stagingAllocation = allocator->CreateBuffer(bufferSize, BufferUsage::TransferSrc, MemoryUsage::CPU);

			// Copy to staging buffer
			{
				void* buffData = stagingAllocation->Map<void>();
				memcpy_s(buffData, size, data, size);
				stagingAllocation->Unmap();
			}
		}

		// Create GPU buffer
		{
			m_allocation = allocator->CreateBuffer(bufferSize, BufferUsage::IndexBuffer | BufferUsage::TransferDst);
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

				cmdBuffer->CopyBufferRegion(stagingAllocation, 0, m_allocation, 0, bufferSize);

				{
					ResourceBarrierInfo barrier{};
					barrier.type = BarrierType::Buffer;
					ResourceUtility::InitializeBarrierSrcFromCurrentState(barrier.bufferBarrier(), this);

					barrier.bufferBarrier().dstAccess = BarrierAccess::IndexBuffer;
					barrier.bufferBarrier().dstStage = BarrierStage::IndexInput;
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
