#include "dxpch.h"
#include "D3D12CommandBuffer.h"

#include "VoltRHI/Graphics/GraphicsDevice.h"
#include "VoltRHI/Memory/Allocation.h"
#include "VoltRHI/Descriptors/DescriptorTable.h"
#include "VoltRHI/Pipelines/ComputePipeline.h"
#include "VoltRHI/Buffers/IndexBuffer.h"
#include "VoltRHI/Buffers/StorageBuffer.h"

#include "VoltRHI/Synchronization/Event.h"

#include "VoltD3D12/Graphics/D3D12DeviceQueue.h"
#include "VoltD3D12/Graphics/D3D12GraphicsDevice.h"
#include "VoltD3D12/Images/D3D12Image2D.h"
#include "VoltD3D12/Images/D3D12ImageView.h"
#include "VoltD3D12/Pipelines/D3D12RenderPipeline.h"

namespace Volt::RHI
{ 
	D3D12CommandBuffer::D3D12CommandBuffer(const uint32_t count, QueueType queueType) 
		: m_queueType(queueType), m_commandListCount(count)
	{
		Invalidate();
		Create(count, queueType, false);
	}

	D3D12CommandBuffer::D3D12CommandBuffer(WeakPtr<Swapchain> swapchain)
		: m_queueType(QueueType::Graphics), m_commandListCount(1), m_swapchainTarget(swapchain)
	{
		m_isSwapchainTarget = true;
		Invalidate();
	}

	D3D12CommandBuffer::~D3D12CommandBuffer()
	{
	}

	void* D3D12CommandBuffer::GetHandleImpl() const
	{
		return nullptr;
	}

	void D3D12CommandBuffer::Invalidate()
	{
		auto device = GraphicsContext::GetDevice();
		ID3D12Device2* devicePtr = device->GetHandle<ID3D12Device2*>();

		auto d3d12QueueType = GetD3D12QueueType(m_queueType);

		m_commandLists.resize(m_commandListCount);

		for (uint32_t i = 0; i < m_commandListCount; i++)
		{
			auto& cmdListData = m_commandLists.at(i);

			VT_D3D12_CHECK(devicePtr->CreateCommandAllocator(d3d12QueueType, VT_D3D12_ID(cmdListData.commandAllocator)));
			VT_D3D12_CHECK(devicePtr->CreateCommandList(0, d3d12QueueType, cmdListData.commandAllocator.Get(), nullptr, VT_D3D12_ID(cmdListData.commandList)));
			cmdListData.commandList->Close();

			std::wstring name = L"Command List - ";

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

			name += L", Index " + std::to_wstring(i);
			cmdListData.commandList->SetName(name.c_str());
		}
	}

	void D3D12CommandBuffer::Release()
	{
	}

	void D3D12CommandBuffer::Begin()
	{
		IncrementIndex();
		auto& commandData = GetCommandData();
		GetFenceData().Wait();

		commandData.commandAllocator->Reset();
		commandData.commandList->Reset(commandData.commandAllocator, nullptr);
	}

	void D3D12CommandBuffer::RestartAfterFlush()
	{
	}

	void D3D12CommandBuffer::End()
	{
		auto& commandData = GetCommandData();
		commandData.commandList->Close();
	}

	void D3D12CommandBuffer::Execute()
	{
		auto device = GraphicsContext::GetDevice();
		device->GetDeviceQueue(m_queueType)->Execute({ { this } });
	}

	void D3D12CommandBuffer::ExecuteAndWait()
	{
		auto device = GraphicsContext::GetDevice();
		device->GetDeviceQueue(m_queueType)->Execute({ { this } });

		auto& commandData = GetCommandData();
		auto& fenceData = GetFenceData();

		fenceData.Wait();
		commandData.commandAllocator->Reset();
	}

	void D3D12CommandBuffer::WaitForLastFence()
	{
	}

	void D3D12CommandBuffer::WaitForFences()
	{
	}

	void D3D12CommandBuffer::SetEvent(WeakPtr<Event> event)
	{
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

	void D3D12CommandBuffer::DrawIndexedIndirect(WeakPtr<StorageBuffer> commandsBuffer, const size_t offset, const uint32_t drawCount, const uint32_t stride)
	{
	}

	void D3D12CommandBuffer::DrawIndirect(WeakPtr<StorageBuffer> commandsBuffer, const size_t offset, const uint32_t drawCount, const uint32_t stride)
	{
	}

	void D3D12CommandBuffer::DrawIndirectCount(WeakPtr<StorageBuffer> commandsBuffer, const size_t offset, WeakPtr<StorageBuffer> countBuffer, const size_t countBufferOffset, const uint32_t maxDrawCount, const uint32_t stride)
	{
	}

	void D3D12CommandBuffer::DrawIndexedIndirectCount(WeakPtr<StorageBuffer> commandsBuffer, const size_t offset, WeakPtr<StorageBuffer> countBuffer, const size_t countBufferOffset, const uint32_t maxDrawCount, const uint32_t stride)
	{
	}

	void D3D12CommandBuffer::DispatchMeshTasks(const uint32_t groupCountX, const uint32_t groupCountY, const uint32_t groupCountZ)
	{
	}

	void D3D12CommandBuffer::DispatchMeshTasksIndirect(WeakPtr<StorageBuffer> commandsBuffer, const size_t offset, const uint32_t drawCount, const uint32_t stride)
	{
	}

	void D3D12CommandBuffer::DispatchMeshTasksIndirectCount(WeakPtr<StorageBuffer> commandsBuffer, const size_t offset, WeakPtr<StorageBuffer> countBuffer, const size_t countBufferOffset, const uint32_t maxDrawCount, const uint32_t stride)
	{
	}

	void D3D12CommandBuffer::Dispatch(const uint32_t groupCountX, const uint32_t groupCountY, const uint32_t groupCountZ)
	{
	}

	void D3D12CommandBuffer::DispatchIndirect(WeakPtr<StorageBuffer> commandsBuffer, const size_t offset)
	{
	}

	void D3D12CommandBuffer::SetViewports(const StackVector<Viewport, MAX_VIEWPORT_COUNT>& viewports)
	{
		GetCommandData().commandList->RSSetViewports(static_cast<uint32_t>(viewports.Size()), reinterpret_cast<const D3D12_VIEWPORT*>(viewports.Data()));
	}

	void D3D12CommandBuffer::SetScissors(const StackVector<Rect2D, MAX_VIEWPORT_COUNT>& scissors)
	{
		GetCommandData().commandList->RSSetScissorRects(static_cast<uint32_t>(scissors.Size()), reinterpret_cast<const D3D12_RECT*>(scissors.Data()));
	}

	void D3D12CommandBuffer::BindPipeline(WeakPtr<RenderPipeline> pipeline)
	{
		auto d3d12RenderPipline = pipeline->As<D3D12RenderPipeline>();
		GetCommandData().commandList->SetGraphicsRootSignature(d3d12RenderPipline->GetRoot());
		GetCommandData().commandList->SetPipelineState(d3d12RenderPipline->GetPSO());

		D3D12_PRIMITIVE_TOPOLOGY topologyType = {};

		switch (d3d12RenderPipline->GetTopology())
		{
			case Topology::LineList: topologyType = D3D10_PRIMITIVE_TOPOLOGY_LINELIST; break;
			case Topology::PatchList: topologyType = D3D10_PRIMITIVE_TOPOLOGY_UNDEFINED; break;
			case Topology::TriangleList: topologyType = D3D10_PRIMITIVE_TOPOLOGY_TRIANGLELIST; break;
			case Topology::PointList: topologyType = D3D10_PRIMITIVE_TOPOLOGY_POINTLIST; break;
			case Topology::TriangleStrip: topologyType = D3D10_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP; break;
		}

		GetCommandData().commandList->IASetPrimitiveTopology(topologyType);
	}

	void D3D12CommandBuffer::BindPipeline(WeakPtr<ComputePipeline> pipeline)
	{

	}

	void D3D12CommandBuffer::BindVertexBuffers(const StackVector<WeakPtr<VertexBuffer>, RHI::MAX_VERTEX_BUFFER_COUNT>& vertexBuffers, const uint32_t firstBinding)
	{
	}

	void D3D12CommandBuffer::BindVertexBuffers(const StackVector<WeakPtr<StorageBuffer>, RHI::MAX_VERTEX_BUFFER_COUNT>& vertexBuffers, const uint32_t firstBinding)
	{
	}

	void D3D12CommandBuffer::BindIndexBuffer(WeakPtr<IndexBuffer> indexBuffer)
	{
	}

	void D3D12CommandBuffer::BindIndexBuffer(WeakPtr<StorageBuffer> indexBuffer)
	{
	}

	void D3D12CommandBuffer::BindDescriptorTable(WeakPtr<DescriptorTable> descriptorTable)
	{
	}

	void D3D12CommandBuffer::BeginRendering(const RenderingInfo& renderingInfo)
	{

		auto& cmdData = GetCommandData();

		std::vector<CD3DX12_CPU_DESCRIPTOR_HANDLE> rtvViews;
		rtvViews.reserve(renderingInfo.colorAttachments.Size());

		CD3DX12_CPU_DESCRIPTOR_HANDLE* dsvView = nullptr;

		for (auto& attachment : renderingInfo.colorAttachments)
		{
			auto view = attachment.view;
			auto viewHandle = view->GetHandle<CD3DX12_CPU_DESCRIPTOR_HANDLE*>();
			
			if (attachment.clearMode == ClearMode::Clear)
			{
				cmdData.commandList->ClearRenderTargetView(*viewHandle, attachment.clearColor.float32, 0, nullptr);
			}
			rtvViews.emplace_back(*viewHandle);
		}

		if (renderingInfo.depthAttachmentInfo.view)
		{
			auto view = renderingInfo.depthAttachmentInfo.view;
			auto viewHandle = view->GetHandle<CD3DX12_CPU_DESCRIPTOR_HANDLE*>();

			if (renderingInfo.depthAttachmentInfo.clearMode == ClearMode::Clear)
			{

				cmdData.commandList->ClearDepthStencilView(*viewHandle, D3D12_CLEAR_FLAG_DEPTH, 1, 0, 0, nullptr);
			}

			dsvView = viewHandle;
		}
		
		m_amountOfTargetsbound = static_cast<uint32_t>(rtvViews.size());

		cmdData.commandList->OMSetRenderTargets(static_cast<UINT>(rtvViews.size()), rtvViews.data(), false, dsvView);
	}

	void D3D12CommandBuffer::EndRendering()
	{
	}

	void D3D12CommandBuffer::PushConstants(const void* data, const uint32_t size, const uint32_t offset)
	{
	}

	void D3D12CommandBuffer::ResourceBarrier(const std::vector<ResourceBarrierInfo>& resourceBarriers)
	{
	}

	void D3D12CommandBuffer::BeginMarker(std::string_view markerLabel, const std::array<float, 4>& markerColor)
	{
	}

	void D3D12CommandBuffer::EndMarker()
	{
	}

	const uint32_t D3D12CommandBuffer::BeginTimestamp()
	{
		return 0;
	}

	void D3D12CommandBuffer::EndTimestamp(uint32_t timestampIndex)
	{
	}

	const float D3D12CommandBuffer::GetExecutionTime(uint32_t timestampIndex) const
	{
		return 0.0f;
	}

	void D3D12CommandBuffer::ClearImage(WeakPtr<Image2D> image, std::array<float, 4> clearColor)
	{
	}

	void D3D12CommandBuffer::ClearBuffer(WeakPtr<StorageBuffer> buffer, const uint32_t value)
	{
	}

	void D3D12CommandBuffer::UpdateBuffer(WeakPtr<StorageBuffer> dstBuffer, const size_t dstOffset, const size_t dataSize, const void* data)
	{
	}

	void D3D12CommandBuffer::CopyBufferRegion(WeakPtr<Allocation> srcResource, const size_t srcOffset, WeakPtr<Allocation> dstResource, const size_t dstOffset, const size_t size)
	{
	}

	void D3D12CommandBuffer::CopyBufferToImage(WeakPtr<Allocation> srcBuffer, WeakPtr<Image2D> dstImage, const uint32_t width, const uint32_t height, const uint32_t mip)
	{
	}

	void D3D12CommandBuffer::CopyImageToBuffer(WeakPtr<Image2D> srcImage, WeakPtr<Allocation> dstBuffer, const size_t dstOffset, const uint32_t width, const uint32_t height, const uint32_t mip)
	{
	}

	void D3D12CommandBuffer::CopyImage(WeakPtr<Image2D> srcImage, WeakPtr<Image2D> dstImage, const uint32_t width, const uint32_t height)
	{
	}

	const uint32_t D3D12CommandBuffer::GetCurrentIndex() const
	{
		return m_currentCommandBufferIndex;
	}

	const QueueType D3D12CommandBuffer::GetQueueType() const
	{
		return m_queueType;
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

		//auto d3d12deviceQueue = deviceQueue->As<D3D12DeviceQueue>();
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
