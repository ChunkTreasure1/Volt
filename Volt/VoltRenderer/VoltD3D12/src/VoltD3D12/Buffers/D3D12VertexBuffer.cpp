#include "dxpch.h"
#include "D3D12VertexBuffer.h"

#include <VoltRHI/Graphics/GraphicsContext.h>

#include <VoltRHI/Buffers/CommandBuffer.h>
#include <VoltRHI/Memory/Allocation.h>
#include <VoltRHI/RHIProxy.h>


#include <CoreUtilities/StringUtility.h>

#include <d3d12.h>

namespace Volt::RHI
{
	D3D12VertexBuffer::D3D12VertexBuffer(const uint32_t size, const void* data)
	{
		Invalidate(data, size);
	}

	D3D12VertexBuffer::~D3D12VertexBuffer()
	{
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

	void D3D12VertexBuffer::SetName(std::string_view name)
	{
		std::wstring wName = Utility::ToWString(name);
		m_allocation->GetResourceHandle<ID3D12Resource*>()->SetName(wName.c_str());
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
				cmdBuffer->CopyBufferRegion(stagingAllocation, 0, m_allocation, 0, size);
				cmdBuffer->End();
				cmdBuffer->Execute();
			}

			allocator->DestroyBuffer(stagingAllocation);
		}

	}
}
