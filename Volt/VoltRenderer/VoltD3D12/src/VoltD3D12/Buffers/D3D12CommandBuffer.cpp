#include "dxpch.h"
#include "D3D12CommandBuffer.h"

#include "VoltRHI/Graphics/GraphicsDevice.h"
#include "VoltD3D12/Graphics/D3D12DeviceQueue.h"
#include "VoltD3D12/Graphics/D3D12GraphicsDevice.h"
#include "VoltD3D12/Images/D3D12Image2D.h"
#include "VoltD3D12/Images/D3D12ImageView.h"
#include "VoltD3D12/Pipelines/D3D12RenderPipeline.h"

namespace Volt::RHI
{
	D3D12CommandBuffer::D3D12CommandBuffer(const uint32_t count, QueueType queueType, bool swapchainTarget) : CommandBuffer(queueType)
	{
		Create(count, queueType, swapchainTarget);
	}

	D3D12CommandBuffer::~D3D12CommandBuffer()
	{
	}

	void* D3D12CommandBuffer::GetHandleImpl() const
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

	void D3D12CommandBuffer::WaitForLastFence()
	{
	}

	void D3D12CommandBuffer::WaitForFences()
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

	void D3D12CommandBuffer::DrawIndexedIndirect(Ref<StorageBuffer> commandsBuffer, const size_t offset, const uint32_t drawCount, const uint32_t stride)
	{
	}

	void D3D12CommandBuffer::DrawIndirect(Ref<StorageBuffer> commandsBuffer, const size_t offset, const uint32_t drawCount, const uint32_t stride)
	{
	}

	void D3D12CommandBuffer::DrawIndirectCount(Ref<StorageBuffer> commandsBuffer, const size_t offset, Ref<StorageBuffer> countBuffer, const size_t countBufferOffset, const uint32_t maxDrawCount, const uint32_t stride)
	{
	}

	void D3D12CommandBuffer::DrawIndexedIndirectCount(Ref<StorageBuffer> commandsBuffer, const size_t offset, Ref<StorageBuffer> countBuffer, const size_t countBufferOffset, const uint32_t maxDrawCount, const uint32_t stride)
	{
	}

	void D3D12CommandBuffer::Dispatch(const uint32_t groupCountX, const uint32_t groupCountY, const uint32_t groupCountZ)
	{
	}

	void D3D12CommandBuffer::SetViewports(const std::vector<Viewport>& viewports)
	{
		GetCommandData().commandList->RSSetViewports(static_cast<uint32_t>(viewports.size()), reinterpret_cast<const D3D12_VIEWPORT*>(viewports.data()));
	}

	void D3D12CommandBuffer::SetScissors(const std::vector<Rect2D>& scissors)
	{
		GetCommandData().commandList->RSSetScissorRects(static_cast<uint32_t>(scissors.size()), reinterpret_cast<const D3D12_RECT*>(scissors.data()));
	}

	void D3D12CommandBuffer::BindPipeline(Ref<RenderPipeline> pipeline)
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

	void D3D12CommandBuffer::BindPipeline(Ref<ComputePipeline> pipeline)
	{

	}

	void D3D12CommandBuffer::BindVertexBuffers(const std::vector<Ref<VertexBuffer>>& vertexBuffers, const uint32_t firstBinding)
	{
	}

	void D3D12CommandBuffer::BindIndexBuffer(Ref<IndexBuffer> indexBuffer)
	{
	}

	void D3D12CommandBuffer::BindDescriptorTable(Ref<DescriptorTable> descriptorTable)
	{
	}

	void D3D12CommandBuffer::BeginRendering(const RenderingInfo& renderingInfo)
	{

		auto& cmdData = GetCommandData();

		std::vector<CD3DX12_CPU_DESCRIPTOR_HANDLE> rtvViews;
		rtvViews.reserve(renderingInfo.colorAttachments.size());

		CD3DX12_CPU_DESCRIPTOR_HANDLE* dsvView = nullptr;

		for (auto& attachment : renderingInfo.colorAttachments)
		{
			auto view = attachment.view;
			auto viewHandle = view->GetHandle<CD3DX12_CPU_DESCRIPTOR_HANDLE*>();
			
			if (attachment.clearMode == ClearMode::Clear)
			{
				cmdData.commandList->ClearRenderTargetView(*viewHandle, attachment.clearColor.data(), 0, nullptr);
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

	void D3D12CommandBuffer::CopyImageToBackBuffer(Ref<Image2D> srcImage)
	{
	}

	void D3D12CommandBuffer::ClearImage(Ref<Image2D> image, std::array<float, 4> clearColor)
	{
	}

	void D3D12CommandBuffer::CopyBufferRegion(Ref<Allocation> srcResource, const size_t srcOffset, Ref<Allocation> dstResource, const size_t dstOffset, const size_t size)
	{
	}

	void D3D12CommandBuffer::CopyBufferToImage(Ref<Allocation> srcBuffer, Ref<Image2D> dstImage, const uint32_t width, const uint32_t height, const uint32_t mip)
	{
	}

	const uint32_t D3D12CommandBuffer::GetCurrentIndex() const
	{
		return m_currentCommandBufferIndex;
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