#include "dxpch.h"
#include "D3D12DeviceQueue.h"

#include "VoltD3D12/Graphics/D3D12GraphicsDevice.h"
#include <VoltD3D12/Buffers/D3D12CommandBuffer.h>

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


		CreateCommandQueue(createInfo.queueType);
	}

	D3D12DeviceQueue::~D3D12DeviceQueue()
	{

		WaitForQueue();


		DestroyCommandQueue(m_queueType);
	}

	void* D3D12DeviceQueue::GetHandleImpl()
	{
		return nullptr;
	}

	void D3D12DeviceQueue::CreateCommandQueue(QueueType type)
	{
		auto d3d12QueueType = GetD3D12QueueType(type);
		auto d3d12GraphicsDevice = GraphicsContext::GetDevice()->As<D3D12GraphicsDevice>();
		auto d3d12Device = d3d12GraphicsDevice->GetHandle<ID3D12Device2*>();

		D3D12_COMMAND_QUEUE_DESC queueDesc = {};
		queueDesc.Type = d3d12QueueType;
		queueDesc.NodeMask = 0;
		queueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;

		VT_D3D12_CHECK(d3d12Device->CreateCommandQueue(&queueDesc, VT_D3D12_ID(m_commandQueue)));

	}

	void D3D12DeviceQueue::DestroyCommandQueue(QueueType type)
	{
		VT_D3D12_DELETE(m_commandQueue);
	}

	void D3D12DeviceQueue::WaitForQueue()
	{
		if (m_currentFence)
		{
			m_commandQueue->Wait(m_currentFence, m_currentFenceValue);
		}
	}

	void D3D12DeviceQueue::Execute(const std::vector<Ref<CommandBuffer>>& commandBuffer)
	{
		if (commandBuffer.empty())
		{
			VT_RHI_DEBUGBREAK();
			return;
		}

		std::vector<ID3D12CommandList*> cmdLists(commandBuffer.size());

		for (size_t i = 0; auto & cmdBuffer : commandBuffer)
		{
			cmdLists[i] = cmdBuffer->As<D3D12CommandBuffer>()->GetCommandData().commandList;
			i++;
		}

		auto& currentFenceData = commandBuffer.front()->As<D3D12CommandBuffer>()->GetFenceData();

		m_commandQueue->ExecuteCommandLists(static_cast<UINT>(cmdLists.size()), cmdLists.data());

		m_commandQueue->Signal(currentFenceData.fence, currentFenceData.fenceValue++);
		m_currentFence = currentFenceData.fence;
		m_currentFenceValue = currentFenceData.fenceValue;
	}
}
