#include "dxpch.h"
#include "D3D12UniformBuffer.h"

#include <RHIModule/Memory/Allocation.h>

#include <RHIModule/Buffers/BufferView.h>
#include <RHIModule/RHIProxy.h>

#include <CoreUtilities/StringUtility.h>

namespace Volt::RHI
{
	D3D12UniformBuffer::D3D12UniformBuffer(const uint32_t size, const void* data, const uint32_t count, std::string_view name)
		: m_size(size)
	{
		GraphicsContext::GetResourceStateTracker()->AddResource(this, BarrierStage::None, BarrierAccess::None);

		m_allocation = GraphicsContext::GetDefaultAllocator()->CreateBuffer(size * count, BufferUsage::UniformBuffer, MemoryUsage::CPUToGPU);

		if (data)
		{
			SetData(data, size);
		}

		SetName(name);
	}

	D3D12UniformBuffer::~D3D12UniformBuffer()
	{
		GraphicsContext::GetResourceStateTracker()->RemoveResource(this);
		
		if (m_allocation == nullptr)
		{
			return;
		}

		RHIProxy::GetInstance().DestroyResource([allocation = m_allocation]()
		{
			GraphicsContext::GetDefaultAllocator()->DestroyBuffer(allocation);
		});

		m_allocation = nullptr;
	}

	RefPtr<BufferView> D3D12UniformBuffer::GetView()
	{
		BufferViewSpecification spec{};
		spec.bufferResource = this;

		return BufferView::Create(spec);
	}

	const uint32_t D3D12UniformBuffer::GetSize() const
	{
		return m_size;
	}

	void D3D12UniformBuffer::SetData(const void* data, const uint32_t size)
	{
		void* bufferData = m_allocation->Map<void>();
		memcpy_s(bufferData, m_size, data, size);
		m_allocation->Unmap();
	}

	void D3D12UniformBuffer::Unmap()
	{
		m_allocation->Unmap();
	}

	void D3D12UniformBuffer::SetName(std::string_view name)
	{
		if (!m_allocation)
		{
			return;
		}

		std::wstring str = Utility::ToWString(name);
		m_allocation->GetResourceHandle<ID3D12Resource*>()->SetName(str.c_str());
	}

	const uint64_t D3D12UniformBuffer::GetDeviceAddress() const
	{
		return 0;
	}

	const uint64_t D3D12UniformBuffer::GetByteSize() const
	{
		return m_allocation->GetSize();
	}

	void* D3D12UniformBuffer::MapInternal(const uint32_t index)
	{
		const uint32_t offset = m_size * index;

		uint8_t* bytePtr = m_allocation->Map<uint8_t>();
		return &bytePtr[offset];
	}

	void* D3D12UniformBuffer::GetHandleImpl() const
	{
		return m_allocation->GetResourceHandle<ID3D12Resource*>();
	}
}
