#include "dxpch.h"
#include "D3D12CommandBuffer.h"

#include "VoltD3D12/Graphics/D3D12DeviceQueue.h"
#include "VoltD3D12/Graphics/D3D12GraphicsDevice.h"
#include "VoltD3D12/Graphics/D3D12GraphicsContext.h"
#include "VoltD3D12/Images/D3D12Image2D.h"
#include "VoltD3D12/Images/D3D12ImageView.h"
#include "VoltD3D12/Pipelines/D3D12RenderPipeline.h"
#include "VoltD3D12/Shader/D3D12Shader.h"
#include "VoltD3D12/Descriptors/D3D12DescriptorTable.h"
#include "VoltD3D12/Descriptors/CPUDescriptorHeapManager.h"
#include "VoltD3D12/Descriptors/D3D12DescriptorHeap.h"
#include "VoltD3D12/Buffers/D3D12BufferView.h"
#include "VoltD3D12/Buffers/D3D12StorageBuffer.h"
#include "VoltD3D12/Buffers/CommandSignatureCache.h"

#include <VoltRHI/Graphics/GraphicsDevice.h>
#include <VoltRHI/Memory/Allocation.h>
#include <VoltRHI/Descriptors/DescriptorTable.h>
#include <VoltRHI/Pipelines/ComputePipeline.h>
#include <VoltRHI/Buffers/IndexBuffer.h>
#include <VoltRHI/Buffers/VertexBuffer.h>
#include <VoltRHI/Synchronization/Semaphore.h>
#include <VoltRHI/Memory/MemoryUtility.h>
#include <VoltRHI/Images/ImageUtility.h>
#include <VoltRHI/RHIProxy.h>

#include "VoltD3D12/Common/d3dx12.h"

#include <CoreUtilities/EnumUtils.h>

#include <pix.h>

namespace Volt::RHI
{ 
	namespace Utility
	{
		D3D12_RESOURCE_STATES GetResourceStateFromLayout(const ImageLayout layout, const BarrierStage barrierStage)
		{
			D3D12_RESOURCE_STATES result = D3D12_RESOURCE_STATE_COMMON;

			if (EnumValueContainsFlag(layout, ImageLayout::Present))
			{
				result |= D3D12_RESOURCE_STATE_PRESENT;
			}

			if (EnumValueContainsFlag(layout, ImageLayout::RenderTarget))
			{
				result |= D3D12_RESOURCE_STATE_RENDER_TARGET;
			}

			if (EnumValueContainsFlag(layout, ImageLayout::ShaderWrite))
			{
				result |= D3D12_RESOURCE_STATE_UNORDERED_ACCESS;
			}

			if (EnumValueContainsFlag(layout, ImageLayout::DepthStencilWrite))
			{
				result |= D3D12_RESOURCE_STATE_DEPTH_WRITE;
			}

			if (EnumValueContainsFlag(layout, ImageLayout::DepthStencilRead))
			{
				result |= D3D12_RESOURCE_STATE_DEPTH_READ;
			}

			if (EnumValueContainsFlag(layout, ImageLayout::ShaderRead))
			{
				if (EnumValueContainsFlag(barrierStage, BarrierStage::PixelShader))
				{
					result |= D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE;
				}

				if (EnumValueContainsFlag(barrierStage, BarrierStage::VertexShader) ||
					EnumValueContainsFlag(barrierStage, BarrierStage::ComputeShader))
				{
					result |= D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE;
				}

				if (result == D3D12_RESOURCE_STATE_COMMON)
				{
					result = D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE;
				}
			}

			if (EnumValueContainsFlag(layout, ImageLayout::TransferSource))
			{
				result |= D3D12_RESOURCE_STATE_COPY_SOURCE;
			}

			if (EnumValueContainsFlag(layout, ImageLayout::TransferDestination))
			{
				result |= D3D12_RESOURCE_STATE_COPY_DEST;
			}

			if (EnumValueContainsFlag(layout, ImageLayout::ResolveSource))
			{
				result |= D3D12_RESOURCE_STATE_RESOLVE_SOURCE;
			}

			if (EnumValueContainsFlag(layout, ImageLayout::ResolveDestination))
			{
				result |= D3D12_RESOURCE_STATE_RESOLVE_DEST;
			}

			if (EnumValueContainsFlag(layout, ImageLayout::VideoDecodeRead))
			{
				result |= D3D12_RESOURCE_STATE_VIDEO_DECODE_READ;
			}

			if (EnumValueContainsFlag(layout, ImageLayout::VideoDecodeWrite))
			{
				result |= D3D12_RESOURCE_STATE_VIDEO_DECODE_WRITE;
			}

			if (EnumValueContainsFlag(layout, ImageLayout::VideoEncodeRead))
			{
				result |= D3D12_RESOURCE_STATE_VIDEO_ENCODE_READ;
			}

			if (EnumValueContainsFlag(layout, ImageLayout::VideoEncodeWrite))
			{
				result |= D3D12_RESOURCE_STATE_VIDEO_ENCODE_WRITE;
			}

			return result;
		}

		D3D12_RESOURCE_STATES GetResourceStateFromStage(const BarrierStage barrierSync, const BarrierAccess barrierAccess, WeakPtr<RHIResource> resource)
		{
			bool isUploadBuffer = false;

			if (resource->GetType() == ResourceType::StorageBuffer)
			{
				isUploadBuffer = (resource->As<D3D12StorageBuffer>()->GetMemoryUsage() & MemoryUsage::CPUToGPU) != MemoryUsage::None;
			}

			D3D12_RESOURCE_STATES result = D3D12_RESOURCE_STATE_COMMON;

			if (EnumValueContainsFlag(barrierSync, BarrierStage::Draw))
			{
				result |= D3D12_RESOURCE_STATE_COMMON;
			}

			if (EnumValueContainsFlag(barrierSync, BarrierStage::IndexInput))
			{
				result |= D3D12_RESOURCE_STATE_INDEX_BUFFER;
			}

			if (EnumValueContainsFlag(barrierSync, BarrierStage::VertexShader))
			{
				if (EnumValueContainsFlag(barrierAccess, BarrierAccess::VertexBuffer))
				{
					result |= D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER;
				}
				else if (EnumValueContainsFlag(barrierAccess, BarrierAccess::ShaderRead))
				{
					result |= D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE;
				}
			}

			if (EnumValueContainsFlag(barrierSync, BarrierStage::PixelShader))
			{
				if (EnumValueContainsFlag(barrierAccess, BarrierAccess::ShaderWrite))
				{
					result |= D3D12_RESOURCE_STATE_UNORDERED_ACCESS;
				}
				else
				{
					result |= D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE;
				}
			}

			if (EnumValueContainsFlag(barrierSync, BarrierStage::DepthStencil))
			{
				if (EnumValueContainsFlag(barrierAccess, BarrierAccess::DepthStencilRead))
				{
					result |= D3D12_RESOURCE_STATE_DEPTH_READ;
				}
				else if (EnumValueContainsFlag(barrierAccess, BarrierAccess::DepthStencilWrite))
				{
					result |= D3D12_RESOURCE_STATE_DEPTH_WRITE;
				}
			}

			if (EnumValueContainsFlag(barrierSync, BarrierStage::RenderTarget))
			{
				result |= D3D12_RESOURCE_STATE_RENDER_TARGET;
			}

			if (EnumValueContainsFlag(barrierSync, BarrierStage::ComputeShader))
			{
				if (EnumValueContainsFlag(barrierAccess, BarrierAccess::ShaderWrite))
				{
					result |= D3D12_RESOURCE_STATE_UNORDERED_ACCESS;
				}
				else
				{
					result |= D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE;
				}
			}

			if (EnumValueContainsFlag(barrierSync, BarrierStage::RayTracing))
			{
				result |= D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE;
			}

			if (EnumValueContainsFlag(barrierSync, BarrierStage::Copy))
			{
				if (EnumValueContainsFlag(barrierAccess, BarrierAccess::TransferSource))
				{
					if (isUploadBuffer)
					{
						result |= D3D12_RESOURCE_STATE_GENERIC_READ;
					}
					else
					{
						result |= D3D12_RESOURCE_STATE_COPY_SOURCE;
					}
				}
				else
				{
					result |= D3D12_RESOURCE_STATE_COPY_DEST;
				}
			}

			if (EnumValueContainsFlag(barrierSync, BarrierStage::Clear))
			{
				result |= D3D12_RESOURCE_STATE_UNORDERED_ACCESS;
			}

			if (EnumValueContainsFlag(barrierSync, BarrierStage::Resolve))
			{
				result |= D3D12_RESOURCE_STATE_RESOLVE_DEST;
			}

			if (EnumValueContainsFlag(barrierSync, BarrierStage::Indirect))
			{
				result |= D3D12_RESOURCE_STATE_INDIRECT_ARGUMENT;
			}

			if (EnumValueContainsFlag(barrierSync, BarrierStage::AllGraphics))
			{
				result |= D3D12_RESOURCE_STATE_COMMON;
			}

			if (EnumValueContainsFlag(barrierSync, BarrierStage::VideoDecode))
			{
				result |= D3D12_RESOURCE_STATE_VIDEO_DECODE_WRITE;
			}

			if (EnumValueContainsFlag(barrierSync, BarrierStage::BuildAccelerationStructure))
			{
				result |= D3D12_RESOURCE_STATE_RAYTRACING_ACCELERATION_STRUCTURE;
			}

			if (EnumValueContainsFlag(barrierSync, BarrierStage::All))
			{
				result |= D3D12_RESOURCE_STATE_COMMON;
			}

			return result;
		}
	}

	D3D12CommandBuffer::D3D12CommandBuffer(const uint32_t count, QueueType queueType) 
		: m_queueType(queueType), m_commandListCount(count)
	{
		m_currentCommandListIndex = m_commandListCount - 1;
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

		// Temp descriptors
		{
			m_allocatedDescriptors.resize(m_commandListCount);

			DescriptorHeapSpecification spec{};
			spec.descriptorType = D3D12DescriptorType::CBV_SRV_UAV;
			spec.maxDescriptorCount = 500;
			spec.supportsGPUDescriptors = true;
			m_descriptorHeap = CreateScope<D3D12DescriptorHeap>(spec);
		}
	}

	void D3D12CommandBuffer::Release()
	{
		WaitForFences();

		m_descriptorHeap = nullptr;
		m_commandLists.clear();
	}

	const uint32_t D3D12CommandBuffer::GetCurrentCommandListIndex() const
	{
		return m_currentCommandListIndex;
	}

	D3D12DescriptorPointer D3D12CommandBuffer::CreateTempDescriptorPointer()
	{
		D3D12DescriptorPointer ptr = m_descriptorHeap->Allocate(m_currentCommandListIndex);
		m_allocatedDescriptors.at(m_currentCommandListIndex).emplace_back(ptr);
		return ptr;
	}

	void D3D12CommandBuffer::Begin()
	{
		VT_PROFILE_FUNCTION();

		m_currentCommandListIndex = (m_currentCommandListIndex + 1) % m_commandListCount;
		auto& currentCommandList = m_commandLists.at(m_currentCommandListIndex);

		currentCommandList.fence->Wait();

		for (const auto& descriptor : m_allocatedDescriptors.at(m_currentCommandListIndex))
		{
			m_descriptorHeap->Free(descriptor);
		}

		VT_D3D12_CHECK(currentCommandList.commandAllocator->Reset());
		VT_D3D12_CHECK(currentCommandList.commandList->Reset(currentCommandList.commandAllocator.Get(), nullptr)); // #TODO_Ivar: Huh?
	}

	void D3D12CommandBuffer::RestartAfterFlush()
	{
	}

	void D3D12CommandBuffer::End()
	{
		VT_PROFILE_FUNCTION();

		m_commandLists.at(m_currentCommandListIndex).commandList->Close();
	}

	void D3D12CommandBuffer::Execute()
	{
		VT_PROFILE_FUNCTION();

		auto device = GraphicsContext::GetDevice();
		device->GetDeviceQueue(m_queueType)->Execute({ { this } });
	}

	void D3D12CommandBuffer::ExecuteAndWait()
	{
		VT_PROFILE_FUNCTION();

		auto device = GraphicsContext::GetDevice();
		device->GetDeviceQueue(m_queueType)->Execute({ { this } });

		const auto& cmdList = m_commandLists.at(m_currentCommandListIndex);
		cmdList.fence->Wait();
	}

	void D3D12CommandBuffer::WaitForFences()
	{
		VT_PROFILE_FUNCTION();

		auto device = GraphicsContext::GetDevice();
		auto queue = device->GetDeviceQueue(m_queueType)->GetHandle<ID3D12CommandQueue*>();

		for (auto& cmdList : m_commandLists)
		{
			queue->Signal(cmdList.fence->GetHandle<ID3D12Fence*>(), cmdList.fence->IncrementAndGetValue());
			cmdList.fence->Wait();
		}
	}

	void D3D12CommandBuffer::SetEvent(WeakPtr<Event> event)
	{
	}

	void D3D12CommandBuffer::Draw(const uint32_t vertexCount, const uint32_t instanceCount, const uint32_t firstVertex, const uint32_t firstInstance)
	{
		VT_PROFILE_FUNCTION();

#ifdef VT_ENABLE_COMMAND_BUFFER_VALIDATION
		VT_ENSURE(m_currentRenderPipeline != nullptr);
#endif

		auto& commandData = m_commandLists.at(m_currentCommandListIndex);
		commandData.commandList->DrawInstanced(vertexCount, instanceCount, firstVertex, firstInstance);
	}

	void D3D12CommandBuffer::DrawIndexed(const uint32_t indexCount, const uint32_t instanceCount, const uint32_t firstIndex, const uint32_t vertexOffset, const uint32_t firstInstance)
	{
		VT_PROFILE_FUNCTION();

#ifdef VT_ENABLE_COMMAND_BUFFER_VALIDATION
		VT_ENSURE(m_currentRenderPipeline != nullptr);
#endif

		auto& commandData = m_commandLists.at(m_currentCommandListIndex);
		commandData.commandList->DrawIndexedInstanced(indexCount, instanceCount, firstIndex, vertexOffset, firstInstance);
	}

	void D3D12CommandBuffer::DrawIndexedIndirect(WeakPtr<StorageBuffer> commandsBuffer, const size_t offset, const uint32_t drawCount, const uint32_t stride)
	{
		VT_PROFILE_FUNCTION();

#ifdef VT_ENABLE_COMMAND_BUFFER_VALIDATION
		VT_ENSURE(m_currentRenderPipeline != nullptr);
#endif

		ComPtr<ID3D12CommandSignature> signature = CommandSignatureCache::Get().GetOrCreateCommandSignature(CommandSignatureType::DrawIndexed, stride);
		auto& commandData = m_commandLists.at(m_currentCommandListIndex);
		commandData.commandList->ExecuteIndirect(signature.Get(), drawCount, commandsBuffer->GetHandle<ID3D12Resource*>(), offset, nullptr, 0);
	}

	void D3D12CommandBuffer::DrawIndirect(WeakPtr<StorageBuffer> commandsBuffer, const size_t offset, const uint32_t drawCount, const uint32_t stride)
	{
		VT_PROFILE_FUNCTION();

#ifdef VT_ENABLE_COMMAND_BUFFER_VALIDATION
		VT_ENSURE(m_currentRenderPipeline != nullptr);
#endif

		ComPtr<ID3D12CommandSignature> signature = CommandSignatureCache::Get().GetOrCreateCommandSignature(CommandSignatureType::Draw, stride);
		auto& commandData = m_commandLists.at(m_currentCommandListIndex);
		commandData.commandList->ExecuteIndirect(signature.Get(), drawCount, commandsBuffer->GetHandle<ID3D12Resource*>(), offset, nullptr, 0);
	}

	void D3D12CommandBuffer::DrawIndirectCount(WeakPtr<StorageBuffer> commandsBuffer, const size_t offset, WeakPtr<StorageBuffer> countBuffer, const size_t countBufferOffset, const uint32_t maxDrawCount, const uint32_t stride)
	{
		VT_PROFILE_FUNCTION();

#ifdef VT_ENABLE_COMMAND_BUFFER_VALIDATION
		VT_ENSURE(m_currentRenderPipeline != nullptr);
#endif

		ComPtr<ID3D12CommandSignature> signature = CommandSignatureCache::Get().GetOrCreateCommandSignature(CommandSignatureType::Draw, stride);
		auto& commandData = m_commandLists.at(m_currentCommandListIndex);
		commandData.commandList->ExecuteIndirect(signature.Get(), maxDrawCount, commandsBuffer->GetHandle<ID3D12Resource*>(), offset, countBuffer->GetHandle<ID3D12Resource*>(), countBufferOffset);
	}

	void D3D12CommandBuffer::DrawIndexedIndirectCount(WeakPtr<StorageBuffer> commandsBuffer, const size_t offset, WeakPtr<StorageBuffer> countBuffer, const size_t countBufferOffset, const uint32_t maxDrawCount, const uint32_t stride)
	{
		VT_PROFILE_FUNCTION();

#ifdef VT_ENABLE_COMMAND_BUFFER_VALIDATION
		VT_ENSURE(m_currentRenderPipeline != nullptr);
#endif

		ComPtr<ID3D12CommandSignature> signature = CommandSignatureCache::Get().GetOrCreateCommandSignature(CommandSignatureType::DrawIndexed, stride);
		auto& commandData = m_commandLists.at(m_currentCommandListIndex);
		commandData.commandList->ExecuteIndirect(signature.Get(), maxDrawCount, commandsBuffer->GetHandle<ID3D12Resource*>(), offset, countBuffer->GetHandle<ID3D12Resource*>(), countBufferOffset);
	}

	void D3D12CommandBuffer::DispatchMeshTasks(const uint32_t groupCountX, const uint32_t groupCountY, const uint32_t groupCountZ)
	{
		VT_PROFILE_FUNCTION();

#ifdef VT_ENABLE_COMMAND_BUFFER_VALIDATION
		VT_ENSURE(m_currentRenderPipeline != nullptr);
#endif

		auto& commandData = m_commandLists.at(m_currentCommandListIndex);
		commandData.commandList->DispatchMesh(groupCountX, groupCountY, groupCountZ);
	}

	void D3D12CommandBuffer::DispatchMeshTasksIndirect(WeakPtr<StorageBuffer> commandsBuffer, const size_t offset, const uint32_t drawCount, const uint32_t stride)
	{
		VT_PROFILE_FUNCTION();

#ifdef VT_ENABLE_COMMAND_BUFFER_VALIDATION
		VT_ENSURE(m_currentRenderPipeline != nullptr);
#endif

		ComPtr<ID3D12CommandSignature> signature = CommandSignatureCache::Get().GetOrCreateCommandSignature(CommandSignatureType::DispatchMesh, stride);
		auto& commandData = m_commandLists.at(m_currentCommandListIndex);
		commandData.commandList->ExecuteIndirect(signature.Get(), drawCount, commandsBuffer->GetHandle<ID3D12Resource*>(), offset, nullptr, 0);
	}

	void D3D12CommandBuffer::DispatchMeshTasksIndirectCount(WeakPtr<StorageBuffer> commandsBuffer, const size_t offset, WeakPtr<StorageBuffer> countBuffer, const size_t countBufferOffset, const uint32_t maxDrawCount, const uint32_t stride)
	{
		VT_PROFILE_FUNCTION();

#ifdef VT_ENABLE_COMMAND_BUFFER_VALIDATION
		VT_ENSURE(m_currentRenderPipeline != nullptr);
#endif

		ComPtr<ID3D12CommandSignature> signature = CommandSignatureCache::Get().GetOrCreateCommandSignature(CommandSignatureType::DispatchMesh, stride);
		auto& commandData = m_commandLists.at(m_currentCommandListIndex);
		commandData.commandList->ExecuteIndirect(signature.Get(), maxDrawCount, commandsBuffer->GetHandle<ID3D12Resource*>(), offset, countBuffer->GetHandle<ID3D12Resource*>(), countBufferOffset);
	}

	void D3D12CommandBuffer::Dispatch(const uint32_t groupCountX, const uint32_t groupCountY, const uint32_t groupCountZ)
	{
		VT_PROFILE_FUNCTION();

#ifdef VT_ENABLE_COMMAND_BUFFER_VALIDATION
		VT_ENSURE(m_currentComputePipeline != nullptr);
#endif

		auto& commandData = m_commandLists.at(m_currentCommandListIndex);
		commandData.commandList->Dispatch(groupCountX, groupCountY, groupCountZ);
	}

	void D3D12CommandBuffer::DispatchIndirect(WeakPtr<StorageBuffer> commandsBuffer, const size_t offset)
	{
		VT_PROFILE_FUNCTION();

#ifdef VT_ENABLE_COMMAND_BUFFER_VALIDATION
		VT_ENSURE(m_currentComputePipeline != nullptr);
#endif

		ComPtr<ID3D12CommandSignature> signature = CommandSignatureCache::Get().GetOrCreateCommandSignature(CommandSignatureType::Dispatch, sizeof(IndirectDispatchCommand));
		auto& commandData = m_commandLists.at(m_currentCommandListIndex);
		commandData.commandList->ExecuteIndirect(signature.Get(), 1, commandsBuffer->GetHandle<ID3D12Resource*>(), offset, nullptr, 0);
	}

	void D3D12CommandBuffer::SetViewports(const StackVector<Viewport, MAX_VIEWPORT_COUNT>& viewports)
	{
		VT_PROFILE_FUNCTION();

		auto& commandData = m_commandLists.at(m_currentCommandListIndex);
		commandData.commandList->RSSetViewports(static_cast<uint32_t>(viewports.Size()), reinterpret_cast<const D3D12_VIEWPORT*>(viewports.Data()));
	}

	void D3D12CommandBuffer::SetScissors(const StackVector<Rect2D, MAX_VIEWPORT_COUNT>& scissors)
	{
		VT_PROFILE_FUNCTION();

		auto& commandData = m_commandLists.at(m_currentCommandListIndex);
		commandData.commandList->RSSetScissorRects(static_cast<uint32_t>(scissors.Size()), reinterpret_cast<const D3D12_RECT*>(scissors.Data()));
	}

	void D3D12CommandBuffer::BindPipeline(WeakPtr<RenderPipeline> pipeline)
	{
		VT_PROFILE_FUNCTION();

		m_currentComputePipeline.Reset();

		if (!pipeline)
		{
			m_currentRenderPipeline.Reset();
			return;
		}

		m_currentRenderPipeline = pipeline;
		auto cmdList = m_commandLists.at(m_currentCommandListIndex).commandList;
		
		D3D12Shader& d3d12Shader = m_currentRenderPipeline->GetShader()->AsRef<D3D12Shader>();
		D3D12RenderPipeline& d3d12Pipeline = m_currentRenderPipeline->AsRef<D3D12RenderPipeline>();

		cmdList->SetGraphicsRootSignature(d3d12Shader.GetRootSignature().Get());
		cmdList->SetPipelineState(m_currentRenderPipeline->GetHandle<ID3D12PipelineState*>());

		D3D12_PRIMITIVE_TOPOLOGY topology{};
		
		switch (d3d12Pipeline.GetTopology())
		{
			case Topology::TriangleList: topology = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST; break;
			case Topology::LineList: topology = D3D_PRIMITIVE_TOPOLOGY_LINELIST; break;
			case Topology::TriangleStrip: topology = D3D_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP; break;
			case Topology::PointList: topology = D3D_PRIMITIVE_TOPOLOGY_POINTLIST; break;
		}

		cmdList->IASetPrimitiveTopology(topology);
	}

	void D3D12CommandBuffer::BindPipeline(WeakPtr<ComputePipeline> pipeline)
	{
		VT_PROFILE_FUNCTION();

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
		VT_PROFILE_FUNCTION();

		StackVector<D3D12_VERTEX_BUFFER_VIEW, RHI::MAX_VERTEX_BUFFER_COUNT> views;

		for (const auto& buffer : vertexBuffers)
		{
			auto& newView = views.Emplace();
			newView.BufferLocation = buffer->GetHandle<ID3D12Resource*>()->GetGPUVirtualAddress();
			newView.SizeInBytes = static_cast<uint32_t>(buffer->GetByteSize());
			newView.StrideInBytes = buffer->GetStride();
		}

		auto& commandData = m_commandLists.at(m_currentCommandListIndex);
		commandData.commandList->IASetVertexBuffers(firstBinding, static_cast<uint32_t>(views.Size()), views.Data());
	}

	void D3D12CommandBuffer::BindVertexBuffers(const StackVector<WeakPtr<StorageBuffer>, RHI::MAX_VERTEX_BUFFER_COUNT>& vertexBuffers, const uint32_t firstBinding)
	{
		VT_PROFILE_FUNCTION();

		StackVector<D3D12_VERTEX_BUFFER_VIEW, RHI::MAX_VERTEX_BUFFER_COUNT> views;

		for (const auto& buffer : vertexBuffers)
		{
			auto& newView = views.Emplace();
			newView.BufferLocation = buffer->GetHandle<ID3D12Resource*>()->GetGPUVirtualAddress();
			newView.SizeInBytes = static_cast<uint32_t>(buffer->GetByteSize());
			newView.StrideInBytes = static_cast<uint32_t>(buffer->GetElementSize());
		}

		auto& commandData = m_commandLists.at(m_currentCommandListIndex);
		commandData.commandList->IASetVertexBuffers(firstBinding, static_cast<uint32_t>(views.Size()), views.Data());
	}

	void D3D12CommandBuffer::BindIndexBuffer(WeakPtr<IndexBuffer> indexBuffer)
	{
		VT_PROFILE_FUNCTION();
		auto& commandData = m_commandLists.at(m_currentCommandListIndex);

		D3D12_INDEX_BUFFER_VIEW view{};
		view.BufferLocation = indexBuffer->GetHandle<ID3D12Resource*>()->GetGPUVirtualAddress();
		view.Format = DXGI_FORMAT_R32_UINT;
		view.SizeInBytes = static_cast<uint32_t>(indexBuffer->GetByteSize());

		commandData.commandList->IASetIndexBuffer(&view);
	}

	void D3D12CommandBuffer::BindIndexBuffer(WeakPtr<StorageBuffer> indexBuffer)
	{
		VT_PROFILE_FUNCTION();

		D3D12_INDEX_BUFFER_VIEW view{};
		view.BufferLocation = indexBuffer->GetHandle<ID3D12Resource*>()->GetGPUVirtualAddress();
		view.Format = DXGI_FORMAT_R32_UINT;
		view.SizeInBytes = static_cast<uint32_t>(indexBuffer->GetByteSize());

		auto& commandData = m_commandLists.at(m_currentCommandListIndex);
		commandData.commandList->IASetIndexBuffer(&view);
	}

	void D3D12CommandBuffer::BindDescriptorTable(WeakPtr<DescriptorTable> descriptorTable)
	{
		VT_PROFILE_FUNCTION();
		descriptorTable->AsRef<D3D12DescriptorTable>().Bind(*this);
	}

	void D3D12CommandBuffer::BeginRendering(const RenderingInfo& renderingInfo)
	{
		VT_PROFILE_FUNCTION();

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
#ifdef VT_ENABLE_COMMAND_BUFFER_VALIDATION
		VT_ENSURE(m_currentComputePipeline != nullptr || m_currentRenderPipeline != nullptr);

		if (m_currentRenderPipeline)
		{
			VT_ENSURE(m_currentRenderPipeline->GetShader()->GetConstantsBuffer().IsValid());
		}
		else
		{
			VT_ENSURE(m_currentComputePipeline->GetShader()->GetConstantsBuffer().IsValid());
		}

		VT_ENSURE(size % 4 == 0 && offset % 4 == 0);
#endif

		auto& cmdData = m_commandLists.at(m_currentCommandListIndex);

		const uint32_t numValues = size / sizeof(uint32_t);
		const uint32_t numOffsetValues = offset / sizeof(uint32_t);

		// Root parameter index will always be 0 for push constants.
		if (m_currentRenderPipeline)
		{
			cmdData.commandList->SetGraphicsRoot32BitConstants(0, numValues, data, numOffsetValues);
		}
		else
		{
			cmdData.commandList->SetComputeRoot32BitConstants(0, numValues, data, numOffsetValues);
		}
	}

	void D3D12CommandBuffer::ResourceBarrier(const std::vector<ResourceBarrierInfo>& resourceBarriers)
	{
		VT_PROFILE_FUNCTION();

		std::vector<D3D12_RESOURCE_BARRIER> barriers{};
		barriers.reserve(resourceBarriers.size());

		for (const auto& voltBarrier : resourceBarriers)
		{
			D3D12_RESOURCE_BARRIER newBarrier{};

			switch (voltBarrier.type)
			{
				case BarrierType::Global:
				{
					newBarrier.Type = D3D12_RESOURCE_BARRIER_TYPE_UAV;
					newBarrier.UAV.pResource = nullptr;
					break;
				}

				case BarrierType::Buffer:
				{
					const auto& bufferBarrier = voltBarrier.bufferBarrier();

					newBarrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
					newBarrier.Transition.pResource = voltBarrier.bufferBarrier().resource->GetHandle<ID3D12Resource*>();
					newBarrier.Transition.Subresource = 0;
					newBarrier.Transition.StateBefore = Utility::GetResourceStateFromStage(bufferBarrier.srcStage, bufferBarrier.srcAccess, voltBarrier.bufferBarrier().resource);
					newBarrier.Transition.StateAfter = Utility::GetResourceStateFromStage(bufferBarrier.dstStage, bufferBarrier.dstAccess, voltBarrier.bufferBarrier().resource);

					if (newBarrier.Transition.StateBefore == newBarrier.Transition.StateAfter)
					{
						continue;
					}
					break;
				}

				case BarrierType::Image:
				{
					const auto& imageBarrier = voltBarrier.imageBarrier();

					newBarrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
					newBarrier.Transition.pResource = imageBarrier.resource->GetHandle<ID3D12Resource*>();
					newBarrier.Transition.Subresource = D3D12CalcSubresource(imageBarrier.subResource.baseMipLevel, imageBarrier.subResource.baseArrayLayer, 0, imageBarrier.subResource.levelCount, imageBarrier.subResource.layerCount);
					newBarrier.Transition.StateBefore = Utility::GetResourceStateFromLayout(imageBarrier.srcLayout, imageBarrier.srcStage);
					newBarrier.Transition.StateAfter = Utility::GetResourceStateFromLayout(imageBarrier.dstLayout, imageBarrier.dstStage);

					if (newBarrier.Transition.StateBefore == newBarrier.Transition.StateAfter)
					{
						continue;
					}

					imageBarrier.resource->As<D3D12Image2D>()->SetCurrentImageLayout(imageBarrier.dstLayout);
					break;
				}
			}

			barriers.emplace_back(newBarrier);
		}

		if (!barriers.empty())
		{
			auto& cmdData = m_commandLists.at(m_currentCommandListIndex);
			cmdData.commandList->ResourceBarrier(static_cast<uint32_t>(barriers.size()), barriers.data());
		}
	}

	void D3D12CommandBuffer::BeginMarker(std::string_view markerLabel, const std::array<float, 4>& markerColor)
	{
		auto& cmdData = m_commandLists.at(m_currentCommandListIndex);
		uint32_t color = PIX_COLOR(static_cast<BYTE>(markerColor[0] * 255.f), static_cast<BYTE>(markerColor[1] * 255.f), static_cast<BYTE>(markerColor[2] * 255.f));
		PIXBeginEvent(cmdData.commandList.Get(), color, markerLabel.data());
	}

	void D3D12CommandBuffer::EndMarker()
	{
		auto& cmdData = m_commandLists.at(m_currentCommandListIndex);
		PIXEndEvent(cmdData.commandList.Get());
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
		VT_PROFILE_FUNCTION();

		auto& cmdData = m_commandLists.at(m_currentCommandListIndex);
		auto imageView = image->GetView();
		auto& d3d12View = imageView->AsRef<D3D12ImageView>();

		D3D12_RECT rect{};
		rect.top = 0;
		rect.left = 0;
		rect.bottom = image->GetHeight();
		rect.right = image->GetWidth();

		if ((image->GetImageAspect() & ImageAspect::Depth) != ImageAspect::None)
		{
			cmdData.commandList->ClearDepthStencilView(D3D12_CPU_DESCRIPTOR_HANDLE(d3d12View.GetRTVDSVDescriptor().GetCPUPointer()), D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL, clearColor[0], static_cast<uint8_t>(clearColor[1]), 1, &rect);
		}
		else
		{
			cmdData.commandList->ClearRenderTargetView(D3D12_CPU_DESCRIPTOR_HANDLE(d3d12View.GetRTVDSVDescriptor().GetCPUPointer()), clearColor.data(), 1, &rect);
		}
	}

	void D3D12CommandBuffer::ClearBuffer(WeakPtr<StorageBuffer> buffer, const uint32_t value)
	{
		VT_PROFILE_FUNCTION();

		auto view = buffer->GetView().As<D3D12BufferView>();
		ID3D12Device2* devicePtr = GraphicsContext::GetDevice()->GetHandle<ID3D12Device2*>();

		D3D12DescriptorPointer srcDescriptor = view->GetUAVDescriptor();
		VT_ENSURE(srcDescriptor.IsValid());

		D3D12DescriptorPointer tempDescriptor = CreateTempDescriptorPointer();
		VT_ENSURE(tempDescriptor.IsValid());

		devicePtr->CopyDescriptorsSimple(1, D3D12_CPU_DESCRIPTOR_HANDLE(tempDescriptor.GetCPUPointer()), D3D12_CPU_DESCRIPTOR_HANDLE(srcDescriptor.GetCPUPointer()), D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

		const uint32_t values[4] = { value, value, value, value };
		ID3D12DescriptorHeap* heap = m_descriptorHeap->GetHeap().Get();

		auto& cmdData = m_commandLists.at(m_currentCommandListIndex);
		cmdData.commandList->SetDescriptorHeaps(1, &heap);
		cmdData.commandList->ClearUnorderedAccessViewUint(D3D12_GPU_DESCRIPTOR_HANDLE(tempDescriptor.GetGPUPointer()), D3D12_CPU_DESCRIPTOR_HANDLE(srcDescriptor.GetCPUPointer()), buffer->GetHandle<ID3D12Resource*>(), values, 0, nullptr);
	}

	void D3D12CommandBuffer::UpdateBuffer(WeakPtr<StorageBuffer> dstBuffer, const size_t dstOffset, const size_t dataSize, const void* data)
	{
	}

	void D3D12CommandBuffer::CopyBufferRegion(WeakPtr<Allocation> srcResource, const size_t srcOffset, WeakPtr<Allocation> dstResource, const size_t dstOffset, const size_t size)
	{
		VT_PROFILE_FUNCTION();

		auto& cmdData = m_commandLists.at(m_currentCommandListIndex);
		cmdData.commandList->CopyBufferRegion(dstResource->GetResourceHandle<ID3D12Resource*>(), dstOffset, srcResource->GetResourceHandle<ID3D12Resource*>(), srcOffset, size);
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

	void D3D12CommandBuffer::UploadTextureData(WeakPtr<Image2D> dstImage, const ImageCopyData& copyData)
	{
		std::vector<D3D12_SUBRESOURCE_DATA> subResources;
		subResources.reserve(copyData.copySubData.size());

		for (const auto& subData : copyData.copySubData)
		{
			auto& newSubResource = subResources.emplace_back();
			newSubResource.pData = subData.data;
			newSubResource.RowPitch = subData.rowPitch;
			newSubResource.SlicePitch = subData.slicePitch;
		}

		auto& cmdData = m_commandLists.at(m_currentCommandListIndex);

		ID3D12Resource* d3d12Image = dstImage->GetHandle<ID3D12Resource*>();

		const uint32_t subResourceCount = static_cast<uint32_t>(subResources.size());
		const uint64_t requiredSize = GetRequiredIntermediateSize(d3d12Image, 0, subResourceCount);
		RefPtr<Allocation> stagingAlloc = GraphicsContext::GetDefaultAllocator()->CreateBuffer(requiredSize, BufferUsage::TransferSrc, MemoryUsage::CPUToGPU);
		UpdateSubresources(cmdData.commandList.Get(), d3d12Image, stagingAlloc->GetResourceHandle<ID3D12Resource*>(), 0, 0, subResourceCount, subResources.data());

		RHIProxy::GetInstance().DestroyResource([stagingAlloc]() 
		{
			GraphicsContext::GetDefaultAllocator()->DestroyBuffer(stagingAlloc);
		});
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
