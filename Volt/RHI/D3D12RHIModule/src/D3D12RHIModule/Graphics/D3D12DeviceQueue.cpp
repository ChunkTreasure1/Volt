#include "dxpch.h"
#include "D3D12DeviceQueue.h"

#include "D3D12RHIModule/Graphics/D3D12GraphicsDevice.h"
#include "D3D12RHIModule/Synchronization/D3D12Semaphore.h"
#include "D3D12RHIModule/Buffers/D3D12CommandBuffer.h"

namespace Volt::RHI
{
	D3D12DeviceQueue::D3D12DeviceQueue(const DeviceQueueCreateInfo& createInfo)
	{
		m_queueType = createInfo.queueType;
		m_device = reinterpret_cast<D3D12GraphicsDevice*>(createInfo.graphicsDevice);
		CreateCommandQueue(createInfo.queueType);
	}

	D3D12DeviceQueue::~D3D12DeviceQueue()
	{
		WaitForQueue();
		m_commandQueue = nullptr;
	}

	void* D3D12DeviceQueue::GetHandleImpl() const
	{
		return m_commandQueue.Get();
	}

	void D3D12DeviceQueue::CreateCommandQueue(QueueType type)
	{
		auto d3d12QueueType = GetD3D12QueueType(type);
		auto d3d12GraphicsDevice = m_device;
		auto d3d12Device = d3d12GraphicsDevice->GetHandle<ID3D12Device2*>();

		D3D12_COMMAND_QUEUE_DESC queueDesc = {};
		queueDesc.Type = d3d12QueueType;
		queueDesc.Priority = D3D12_COMMAND_QUEUE_PRIORITY_NORMAL;
		queueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
		queueDesc.NodeMask = 0;

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

	void D3D12DeviceQueue::WaitForQueue()
	{
	}

	void D3D12DeviceQueue::Execute(const DeviceQueueExecuteInfo& executeInfo)
	{
		if (executeInfo.commandBuffers.empty())
		{
			return;
		}

		Vector<ID3D12CommandList*> cmdLists(executeInfo.commandBuffers.size());

		for (size_t i = 0; auto & cmdBuffer : executeInfo.commandBuffers)
		{
			cmdLists[i] = cmdBuffer->GetHandle<ID3D12CommandList*>();
			i++;
		}

		m_commandQueue->ExecuteCommandLists(static_cast<UINT>(cmdLists.size()), cmdLists.data());

		for (const auto& cmdBuffer : executeInfo.commandBuffers)
		{
			auto currentFenceData = cmdBuffer->As<D3D12CommandBuffer>()->GetCurrentSemaphore();

			auto val = currentFenceData->IncrementAndGetValue();
			m_commandQueue->Signal(currentFenceData->GetHandle<ID3D12Fence*>(), val);

			//RHILog::Log(LogSeverity::Trace, "Signaling value {}", val);
		}
	}
}
