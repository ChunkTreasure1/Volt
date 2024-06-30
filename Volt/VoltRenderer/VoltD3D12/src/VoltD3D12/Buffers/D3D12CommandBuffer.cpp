#include "dxpch.h"
#include "D3D12CommandBuffer.h"

#include "VoltD3D12/Graphics/D3D12DeviceQueue.h"
#include "VoltD3D12/Graphics/D3D12GraphicsDevice.h"
#include "VoltD3D12/Images/D3D12Image2D.h"
#include "VoltD3D12/Images/D3D12ImageView.h"
#include "VoltD3D12/Pipelines/D3D12RenderPipeline.h"
#include "VoltD3D12/Shader/D3D12Shader.h"
#include "VoltD3D12/Descriptors/D3D12DescriptorTable.h"

#include <VoltRHI/Graphics/GraphicsDevice.h>
#include <VoltRHI/Memory/Allocation.h>
#include <VoltRHI/Descriptors/DescriptorTable.h>
#include <VoltRHI/Pipelines/ComputePipeline.h>
#include <VoltRHI/Buffers/IndexBuffer.h>
#include <VoltRHI/Buffers/StorageBuffer.h>
#include <VoltRHI/Synchronization/Semaphore.h>

namespace Volt::RHI
{ 
	D3D12CommandBuffer::D3D12CommandBuffer(const uint32_t count, QueueType queueType) 
		: m_queueType(queueType), m_commandListCount(count)
	{
		m_currentCommandListIndex = m_commandListCount - 1;
		Invalidate();
	}

	D3D12CommandBuffer::D3D12CommandBuffer(WeakPtr<Swapchain> swapchain)
		: m_queueType(QueueType::Graphics), m_commandListCount(1), m_swapchainTarget(swapchain)
	{
		m_currentCommandListIndex = m_commandListCount - 1;
		m_isSwapchainTarget = true;
		Invalidate();
	}

	D3D12CommandBuffer::~D3D12CommandBuffer()
	{
		Release();
	}

	void* D3D12CommandBuffer::GetHandleImpl() const
	{
		return m_commandLists.at(m_currentCommandListIndex).commandList.Get();
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
			
			SemaphoreCreateInfo info{};
			info.initialValue = 0;
			cmdListData.fence = Semaphore::Create(info);
		}
	}

	void D3D12CommandBuffer::Release()
	{
		WaitForFences();
	}

	const uint32_t D3D12CommandBuffer::GetCurrentCommandListIndex() const
	{
		return m_currentCommandListIndex;
	}

	void D3D12CommandBuffer::Begin()
	{
		VT_PROFILE_FUNCTION();

		m_currentCommandListIndex = (m_currentCommandListIndex + 1) % m_commandListCount;
		auto& currentCommandList = m_commandLists.at(m_currentCommandListIndex);

		if (!m_isSwapchainTarget)
		{
			currentCommandList.fence->Wait();
		}

		currentCommandList.commandAllocator->Reset();
		currentCommandList.commandList->Reset(currentCommandList.commandAllocator.Get(), nullptr); // #TODO_Ivar: Huh?
	}

	void D3D12CommandBuffer::RestartAfterFlush()
	{
	}

	void D3D12CommandBuffer::End()
	{
		m_commandLists.at(m_currentCommandListIndex).commandList->Close();
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

		const auto& cmdList = m_commandLists.at(m_currentCommandListIndex);
		cmdList.fence->Wait();
	}

	void D3D12CommandBuffer::WaitForLastFence()
	{
	}

	void D3D12CommandBuffer::WaitForFences()
	{
		for (auto& cmdList : m_commandLists)
		{
			cmdList.fence->Wait();
		}
	}

	void D3D12CommandBuffer::SetEvent(WeakPtr<Event> event)
	{
	}

	void D3D12CommandBuffer::Draw(const uint32_t vertexCount, const uint32_t instanceCount, const uint32_t firstVertex, const uint32_t firstInstance)
	{
		auto& commandData = m_commandLists.at(m_currentCommandListIndex);
		commandData.commandList->DrawInstanced(vertexCount, instanceCount, firstVertex, firstInstance);
	}

	void D3D12CommandBuffer::DrawIndexed(const uint32_t indexCount, const uint32_t instanceCount, const uint32_t firstIndex, const uint32_t vertexOffset, const uint32_t firstInstance)
	{
		auto& commandData = m_commandLists.at(m_currentCommandListIndex);
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
		auto& commandData = m_commandLists.at(m_currentCommandListIndex);
		commandData.commandList->Dispatch(groupCountX, groupCountY, groupCountZ);
	}

	void D3D12CommandBuffer::DispatchIndirect(WeakPtr<StorageBuffer> commandsBuffer, const size_t offset)
	{
	}

	void D3D12CommandBuffer::SetViewports(const StackVector<Viewport, MAX_VIEWPORT_COUNT>& viewports)
	{
		auto& commandData = m_commandLists.at(m_currentCommandListIndex);
		commandData.commandList->RSSetViewports(static_cast<uint32_t>(viewports.Size()), reinterpret_cast<const D3D12_VIEWPORT*>(viewports.Data()));
	}

	void D3D12CommandBuffer::SetScissors(const StackVector<Rect2D, MAX_VIEWPORT_COUNT>& scissors)
	{
		auto& commandData = m_commandLists.at(m_currentCommandListIndex);
		commandData.commandList->RSSetScissorRects(static_cast<uint32_t>(scissors.Size()), reinterpret_cast<const D3D12_RECT*>(scissors.Data()));
	}

	void D3D12CommandBuffer::BindPipeline(WeakPtr<RenderPipeline> pipeline)
	{
		m_currentComputePipeline.Reset();

		if (!pipeline)
		{
			m_currentRenderPipeline.Reset();
			return;
		}

		m_currentRenderPipeline = pipeline;

		//auto d3d12RenderPipline = pipeline->As<D3D12RenderPipeline>();
		//GetCommandData().commandList->SetGraphicsRootSignature(d3d12RenderPipline->GetRoot());
		//GetCommandData().commandList->SetPipelineState(d3d12RenderPipline->GetPSO());
		//
		//D3D12_PRIMITIVE_TOPOLOGY topologyType = {};
		//
		//switch (d3d12RenderPipline->GetTopology())
		//{
		//	case Topology::LineList: topologyType = D3D10_PRIMITIVE_TOPOLOGY_LINELIST; break;
		//	case Topology::PatchList: topologyType = D3D10_PRIMITIVE_TOPOLOGY_UNDEFINED; break;
		//	case Topology::TriangleList: topologyType = D3D10_PRIMITIVE_TOPOLOGY_TRIANGLELIST; break;
		//	case Topology::PointList: topologyType = D3D10_PRIMITIVE_TOPOLOGY_POINTLIST; break;
		//	case Topology::TriangleStrip: topologyType = D3D10_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP; break;
		//}
		//
		//GetCommandData().commandList->IASetPrimitiveTopology(topologyType);
	}

	void D3D12CommandBuffer::BindPipeline(WeakPtr<ComputePipeline> pipeline)
	{
		m_currentRenderPipeline.Reset();

		if (!pipeline)
		{
			m_currentComputePipeline.Reset();
			return;
		}

		m_currentComputePipeline = pipeline;

		const uint32_t index = GetCurrentCommandListIndex();
		auto& cmdList = m_commandLists.at(index).commandList;

		D3D12Shader& d3d12Shader = m_currentComputePipeline->GetShader()->AsRef<D3D12Shader>();

		cmdList->SetComputeRootSignature(d3d12Shader.GetRootSignature().Get());
		cmdList->SetPipelineState(m_currentComputePipeline->GetHandle<ID3D12PipelineState*>());
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
		VT_PROFILE_FUNCTION();
		descriptorTable->AsRef<D3D12DescriptorTable>().Bind(*this);
	}

	void D3D12CommandBuffer::BeginRendering(const RenderingInfo& renderingInfo)
	{
		auto& cmdData = m_commandLists.at(m_currentCommandListIndex);

		std::vector<D3D12_CPU_DESCRIPTOR_HANDLE> rtvViews;
		rtvViews.reserve(renderingInfo.colorAttachments.Size());

		D3D12_CPU_DESCRIPTOR_HANDLE* dsvView = nullptr;

		for (auto& attachment : renderingInfo.colorAttachments)
		{
			auto view = attachment.view;
			auto viewHandle = D3D12_CPU_DESCRIPTOR_HANDLE(view.As<D3D12ImageView>()->GetRTVDSVDescriptor().GetCPUPointer());
			
			if (attachment.clearMode == ClearMode::Clear)
			{
				cmdData.commandList->ClearRenderTargetView(viewHandle, attachment.clearColor.float32, 0, nullptr);
			}
			rtvViews.emplace_back(viewHandle);
		}

		if (renderingInfo.depthAttachmentInfo.view)
		{
			auto view = renderingInfo.depthAttachmentInfo.view;
			auto viewHandle = D3D12_CPU_DESCRIPTOR_HANDLE(view.As<D3D12ImageView>()->GetRTVDSVDescriptor().GetCPUPointer());

			if (renderingInfo.depthAttachmentInfo.clearMode == ClearMode::Clear)
			{

				cmdData.commandList->ClearDepthStencilView(viewHandle, D3D12_CLEAR_FLAG_DEPTH, 1, 0, 0, nullptr);
			}

			dsvView = &viewHandle;
		}
		
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
		return m_currentCommandListIndex;
	}

	const QueueType D3D12CommandBuffer::GetQueueType() const
	{
		return m_queueType;
	}
}
