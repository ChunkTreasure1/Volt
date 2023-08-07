#include "dxpch.h"
#include "D3D12DeviceQueue.h"

#include "VoltD3D12/Graphics/D3D12GraphicsDevice.h"
#include <VoltD3D12/Buffers/D3D12CommandBuffer.h>

namespace Volt::RHI
{
	D3D12DeviceQueue::D3D12DeviceQueue(const DeviceQueueCreateInfo& createInfo)
	{
		m_queueType = createInfo.queueType;
		m_device = reinterpret_cast<D3D12GraphicsDevice*>(createInfo.graphicsDevice);
		m_currentFence = nullptr;
		m_currentFenceValue = 0;
		CreateCommandQueue(createInfo.queueType);
	}

	D3D12DeviceQueue::~D3D12DeviceQueue()
	{

		WaitForQueue();


		DestroyCommandQueue(m_queueType);
	}

	void* D3D12DeviceQueue::GetHandleImpl()
	{
		return m_commandQueue;
	}

	void D3D12DeviceQueue::CreateCommandQueue(QueueType type)
	{
		auto d3d12QueueType = GetD3D12QueueType(type);
		auto d3d12GraphicsDevice = m_device;
		auto d3d12Device = d3d12GraphicsDevice->GetHandle<ID3D12Device2*>();

		D3D12_COMMAND_QUEUE_DESC queueDesc = {};
		queueDesc.Type = d3d12QueueType;
		queueDesc.NodeMask = 0;
		queueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;

		VT_D3D12_CHECK(d3d12Device->CreateCommandQueue(&queueDesc, VT_D3D12_ID(m_commandQueue)));

		std::wstring name;

		name = L"CommandQueue - TYPE [";
		switch (type)
		{
			case Volt::RHI::QueueType::Graphics:
				name += L"Graphics";
				break;
			case Volt::RHI::QueueType::Compute:
				name += L"Compute";
				break;
			case Volt::RHI::QueueType::TransferCopy:
				name += L"TransferCopy";
				break;
			default:
				break;
		}
		name += L"]";

		m_commandQueue->SetName(name.c_str());

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

	void D3D12DeviceQueue::Wait(D3D12Fence& fence)
	{
		m_commandQueue->Wait(fence.Get(), fence.Value());
	}

	void D3D12DeviceQueue::Signal(D3D12Fence& fence, const size_t customID)
	{
		m_commandQueue->Signal(fence.Get(), customID);
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

		m_commandQueue->Signal(currentFenceData.Get(), currentFenceData.Value()++);
		m_currentFence = currentFenceData.Get();
		m_currentFenceValue = currentFenceData.Value();
	}
}
