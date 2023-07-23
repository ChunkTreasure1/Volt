#include "dxpch.h"
#include "D3D12DeviceQueue.h"

#include "VoltD3D12/Graphics/D3D12GraphicsDevice.h"

namespace Volt
{
	D3D12_COMMAND_LIST_TYPE GetD3D12QueueType(QueueType type)
	{
		D3D12_COMMAND_LIST_TYPE d3d12Type = {};
		switch (type)
		{
		case Volt::QueueType::Graphics:
			d3d12Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
			break;
		case Volt::QueueType::Compute:
			d3d12Type = D3D12_COMMAND_LIST_TYPE_COMPUTE;
			break;
		case Volt::QueueType::TransferCopy:
			d3d12Type = D3D12_COMMAND_LIST_TYPE_COPY;
			break;
		}
		return d3d12Type;
	}

    D3D12DeviceQueue::D3D12DeviceQueue(const DeviceQueueCreateInfo& createInfo)
    {
		m_queueType = createInfo.queueType;

		auto d3d12QueueType = GetD3D12QueueType(createInfo.queueType);

		auto d3d12GraphicsDevice = GraphicsContext::GetDevice()->As<D3D12GraphicsDevice>();
		auto d3d12Device = d3d12GraphicsDevice->GetHandle<ID3D12Device2*>();

		CreateCommandQueue(createInfo.queueType);

		VT_D3D12_CHECK(d3d12Device->CreateCommandAllocator(d3d12QueueType, VT_D3D12_ID(m_commandAllocator)));

		VT_D3D12_CHECK(d3d12Device->CreateFence(0, D3D12_FENCE_FLAG_SHARED, VT_D3D12_ID(m_fence)));

		m_windowsFenceHandle = CreateEventEx(nullptr, nullptr, false, EVENT_ALL_ACCESS);

		//Niklas: we offset the fence values so that we dont accidentally wait on the wrong queue.
		m_fenceStartValue = static_cast<size_t>(d3d12QueueType) << 56;
		m_fenceValue = m_fenceStartValue;

		m_fence->Signal(m_fenceValue);
    }

    D3D12DeviceQueue::~D3D12DeviceQueue()
    {
		m_fence->Signal(m_fenceValue);

		WaitForQueue();

		CloseHandle(m_windowsFenceHandle);

		VT_D3D12_DELETE(m_fence);

		VT_D3D12_DELETE(m_commandAllocator);

		DestroyCommandQueue(m_queueType);
    }

    void* D3D12DeviceQueue::GetHandleImpl()
    {
        return nullptr;
    }

	void D3D12DeviceQueue::CreateCommandQueue(QueueType type)
	{
		if (s_commandQueues.contains(type) == false)
		{
			s_commandQueues.at(type) = { nullptr, 0 };
		}

		auto& queue = s_commandQueues.at(type);

		if (!queue.first)
		{
			auto d3d12QueueType = GetD3D12QueueType(type);
			auto d3d12GraphicsDevice = GraphicsContext::GetDevice()->As<D3D12GraphicsDevice>();
			auto d3d12Device = d3d12GraphicsDevice->GetHandle<ID3D12Device2*>();

			D3D12_COMMAND_QUEUE_DESC queueDesc = {};
			queueDesc.Type = d3d12QueueType;
			queueDesc.NodeMask = 0;
			queueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;

			VT_D3D12_CHECK(d3d12Device->CreateCommandQueue(&queueDesc, VT_D3D12_ID(queue.first)));
		}

		queue.second++;
	}

	void D3D12DeviceQueue::DestroyCommandQueue(QueueType type)
	{
		auto& queue = s_commandQueues.at(type);

		if (queue.second <= 1)
		{
			VT_D3D12_DELETE(queue.first);
		}
		queue.second--;
	}

    void D3D12DeviceQueue::WaitForQueue()
    {
		if (m_fenceValue != m_fenceStartValue)
		{
			return;
		}

		const size_t previousFenceValue = m_fenceValue - 1;

		if (m_fence->GetCompletedValue() < previousFenceValue)
		{
			m_fence->SetEventOnCompletion(previousFenceValue, m_windowsFenceHandle);
			WaitForSingleObject(m_windowsFenceHandle, INFINITE);
			m_commandAllocator->Reset();
		}
    }

    void D3D12DeviceQueue::Execute(const std::vector<Ref<CommandBuffer>>& commandBuffer)
    {
    }
}
