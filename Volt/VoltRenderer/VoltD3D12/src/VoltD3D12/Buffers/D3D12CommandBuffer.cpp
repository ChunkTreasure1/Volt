#include "dxpch.h"
#include "D3D12CommandBuffer.h"

#include "VoltRHI/Graphics/GraphicsDevice.h"

#include "VoltD3D12/Graphics/D3D12DeviceQueue.h"
#include <VoltD3D12/Graphics/D3D12GraphicsDevice.h>

namespace Volt::RHI
{
	D3D12CommandBuffer::D3D12CommandBuffer(const uint32_t count, QueueType queueType, bool swapchainTarget) : CommandBuffer(queueType)
	{
		Create(count, queueType, swapchainTarget);
	}

	D3D12CommandBuffer::~D3D12CommandBuffer()
	{
	}

	void* D3D12CommandBuffer::GetHandleImpl()
	{
		return nullptr;
	}

	void D3D12CommandBuffer::Begin()
	{
		IncrementIndex();
		auto& commandData = GetCommandData();

		GetFenceData().Wait();

		commandData.commandAllocator->Reset();
		commandData.commandList->Reset(commandData.commandAllocator, nullptr);
	}

	void D3D12CommandBuffer::End()
	{
		auto& commandData = GetCommandData();
		commandData.commandList->Close();
	}

	void D3D12CommandBuffer::Execute()
	{
		auto device = GraphicsContext::GetDevice();
		device->GetDeviceQueue(m_queueType)->Execute({ shared_from_this()->As<D3D12CommandBuffer>()});
	}

	void D3D12CommandBuffer::ExecuteAndWait()
	{
		auto device = GraphicsContext::GetDevice();
		device->GetDeviceQueue(m_queueType)->Execute({ shared_from_this()->As<D3D12CommandBuffer>() });

		auto& commandData = GetCommandData();
		auto& fenceData = GetFenceData();

		fenceData.Wait();
		commandData.commandAllocator->Reset();
	}

	void D3D12CommandBuffer::Draw(const uint32_t vertexCount, const uint32_t instanceCount, const uint32_t firstVertex, const uint32_t firstInstance)
	{
		auto& commandData = GetCommandData();

		commandData.commandList->DrawInstanced(vertexCount, instanceCount, firstVertex, firstInstance);
	}

	void D3D12CommandBuffer::DrawIndexed(const uint32_t indexCount, const uint32_t instanceCount, const uint32_t firstIndex, const uint32_t vertexOffset, const uint32_t firstInstance)
	{
		auto& commandData = GetCommandData();

		commandData.commandList->DrawIndexedInstanced(indexCount, instanceCount, firstIndex, vertexOffset, firstInstance);
	}

	void D3D12CommandBuffer::SetViewports(const std::vector<Viewport>& viewports)
	{
	}

	void D3D12CommandBuffer::SetScissors(const std::vector<Rect2D>& scissors)
	{
	}

	void D3D12CommandBuffer::Create(const uint32_t count, QueueType queueType, bool swapchainTarget)
	{
		m_isSwapchainTarget = swapchainTarget;
		m_queueType = queueType;
		m_perInternalBufferData.resize(count);

		auto device = GraphicsContext::GetDevice();

		if (!device)
		{
			VT_RHI_DEBUGBREAK();
			return;
		}

		auto deviceQueue = device->GetDeviceQueue(queueType);

		if (!deviceQueue)
		{
			VT_RHI_DEBUGBREAK();
			return;
		}

		auto d3d12deviceQueue = deviceQueue->As<D3D12DeviceQueue>();
		auto d3d12device = device->As<D3D12GraphicsDevice>()->GetHandle<ID3D12Device2*>();

		auto d3d12QueueType = GetD3D12QueueType(m_queueType);

		for (size_t i = 0; i < count; ++i)
		{
			auto& internalData = m_perInternalBufferData[i];

			auto& commandData = internalData.first;
			auto& fenceData = internalData.second;

			VT_D3D12_CHECK(d3d12device->CreateCommandAllocator(d3d12QueueType, VT_D3D12_ID(commandData.commandAllocator)));

			VT_D3D12_CHECK(d3d12device->CreateCommandList(0, d3d12QueueType, commandData.commandAllocator, nullptr, VT_D3D12_ID(commandData.commandList)));

			commandData.commandList->Close();

			std::wstring name;
			name = L"CommandList - TYPE [";

			switch (m_queueType)
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

			name += L"] - Flight ID [" + std::to_wstring(i) + L"]";

			commandData.commandList->SetName(name.c_str());

			fenceData.Create(queueType);
			fenceData.Signal();
		}
	}

	void D3D12CommandBuffer::IncrementIndex()
	{
		m_currentCommandBufferIndex = (m_currentCommandBufferIndex + 1) % static_cast<uint32_t>(m_perInternalBufferData.size());
	}

	D3D12Fence& D3D12CommandBuffer::GetFenceData()
	{
		return m_perInternalBufferData[m_currentCommandBufferIndex].second;
	}

	D3D12CommandData& D3D12CommandBuffer::GetCommandData()
	{
		return m_perInternalBufferData[m_currentCommandBufferIndex].first;
	}
	
}