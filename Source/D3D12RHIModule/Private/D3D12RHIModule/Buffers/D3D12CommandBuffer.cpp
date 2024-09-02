#include "dxpch.h"
#include "D3D12RHIModule/Buffers/D3D12CommandBuffer.h"

#include "D3D12RHIModule/Graphics/D3D12DeviceQueue.h"
#include "D3D12RHIModule/Graphics/D3D12GraphicsDevice.h"
#include "D3D12RHIModule/Graphics/D3D12GraphicsContext.h"
#include "D3D12RHIModule/Images/D3D12Image.h"
#include "D3D12RHIModule/Images/D3D12ImageView.h"
#include "D3D12RHIModule/Pipelines/D3D12RenderPipeline.h"
#include "D3D12RHIModule/Shader/D3D12Shader.h"
#include "D3D12RHIModule/Descriptors/D3D12DescriptorTable.h"
#include "D3D12RHIModule/Descriptors/D3D12BindlessDescriptorTable.h"
#include "D3D12RHIModule/Descriptors/CPUDescriptorHeapManager.h"
#include "D3D12RHIModule/Descriptors/D3D12DescriptorHeap.h"
#include "D3D12RHIModule/Buffers/D3D12BufferView.h"
#include "D3D12RHIModule/Buffers/D3D12StorageBuffer.h"
#include "D3D12RHIModule/Buffers/CommandSignatureCache.h"

#include <RHIModule/Graphics/GraphicsDevice.h>
#include <RHIModule/Memory/Allocation.h>
#include <RHIModule/Descriptors/DescriptorTable.h>
#include <RHIModule/Pipelines/ComputePipeline.h>
#include <RHIModule/Buffers/IndexBuffer.h>
#include <RHIModule/Buffers/VertexBuffer.h>
#include <RHIModule/Synchronization/Semaphore.h>
#include <RHIModule/Synchronization/Fence.h>
#include <RHIModule/Memory/MemoryUtility.h>
#include <RHIModule/Images/ImageUtility.h>
#include <RHIModule/RHIProxy.h>

#include <CoreUtilities/EnumUtils.h>

#include <pix.h>

namespace Volt::RHI
{
	namespace Utility
	{
		inline D3D12_BARRIER_SYNC GetD3D12BarrierSyncFromBarrierStage(const BarrierStage barrierStage)
		{
			D3D12_BARRIER_SYNC result = D3D12_BARRIER_SYNC_NONE;

			if (EnumValueContainsFlag(barrierStage, BarrierStage::All))
			{
				result |= D3D12_BARRIER_SYNC_ALL;
			}

			if (EnumValueContainsFlag(barrierStage, BarrierStage::IndexInput))
			{
				result |= D3D12_BARRIER_SYNC_INDEX_INPUT;
			}

			if (EnumValueContainsFlag(barrierStage, BarrierStage::VertexInput))
			{
				result |= D3D12_BARRIER_SYNC_VERTEX_SHADING;
			}

			if (EnumValueContainsFlag(barrierStage, BarrierStage::VertexShader))
			{
				result |= D3D12_BARRIER_SYNC_VERTEX_SHADING;
			}

			if (EnumValueContainsFlag(barrierStage, BarrierStage::PixelShader))
			{
				result |= D3D12_BARRIER_SYNC_PIXEL_SHADING;
			}

			if (EnumValueContainsFlag(barrierStage, BarrierStage::DepthStencil))
			{
				result |= D3D12_BARRIER_SYNC_DEPTH_STENCIL;
			}

			if (EnumValueContainsFlag(barrierStage, BarrierStage::RenderTarget))
			{
				result |= D3D12_BARRIER_SYNC_RENDER_TARGET;
			}

			if (EnumValueContainsFlag(barrierStage, BarrierStage::ComputeShader))
			{
				result |= D3D12_BARRIER_SYNC_COMPUTE_SHADING;
			}

			if (EnumValueContainsFlag(barrierStage, BarrierStage::RayTracing))
			{
				result |= D3D12_BARRIER_SYNC_RAYTRACING;
			}

			if (EnumValueContainsFlag(barrierStage, BarrierStage::Copy))
			{
				result |= D3D12_BARRIER_SYNC_COPY;
			}

			if (EnumValueContainsFlag(barrierStage, BarrierStage::Resolve))
			{
				result |= D3D12_BARRIER_SYNC_RESOLVE;
			}

			if (EnumValueContainsFlag(barrierStage, BarrierStage::DrawIndirect))
			{
				result |= D3D12_BARRIER_SYNC_EXECUTE_INDIRECT;
			}

			if (EnumValueContainsFlag(barrierStage, BarrierStage::AllGraphics))
			{
				result |= D3D12_BARRIER_SYNC_ALL_SHADING;
			}

			if (EnumValueContainsFlag(barrierStage, BarrierStage::VideoDecode))
			{
				result |= D3D12_BARRIER_SYNC_VIDEO_DECODE;
			}

			if (EnumValueContainsFlag(barrierStage, BarrierStage::VideoEncode))
			{
				result |= D3D12_BARRIER_SYNC_VIDEO_ENCODE;
			}

			return result;
		}

		inline D3D12_BARRIER_ACCESS GetD3D12BarrierAccessFromBarrierAccess(const BarrierAccess barrierAccess)
		{
			D3D12_BARRIER_ACCESS result = D3D12_BARRIER_ACCESS_COMMON;

			if (barrierAccess == BarrierAccess::None)
			{
				result |= D3D12_BARRIER_ACCESS_NO_ACCESS;
			}

			if (EnumValueContainsFlag(barrierAccess, BarrierAccess::VertexBuffer))
			{
				result |= D3D12_BARRIER_ACCESS_VERTEX_BUFFER;
			}

			if (EnumValueContainsFlag(barrierAccess, BarrierAccess::UniformBuffer))
			{
				result |= D3D12_BARRIER_ACCESS_CONSTANT_BUFFER;
			}

			if (EnumValueContainsFlag(barrierAccess, BarrierAccess::IndexBuffer))
			{
				result |= D3D12_BARRIER_ACCESS_INDEX_BUFFER;
			}

			if (EnumValueContainsFlag(barrierAccess, BarrierAccess::RenderTarget))
			{
				result |= D3D12_BARRIER_ACCESS_RENDER_TARGET;
			}

			if (EnumValueContainsFlag(barrierAccess, BarrierAccess::ShaderWrite))
			{
				result |= D3D12_BARRIER_ACCESS_UNORDERED_ACCESS;
			}

			if (EnumValueContainsFlag(barrierAccess, BarrierAccess::DepthStencilWrite))
			{
				result |= D3D12_BARRIER_ACCESS_DEPTH_STENCIL_WRITE;
			}

			if (EnumValueContainsFlag(barrierAccess, BarrierAccess::DepthStencilRead))
			{
				result |= D3D12_BARRIER_ACCESS_DEPTH_STENCIL_READ;
			}

			if (EnumValueContainsFlag(barrierAccess, BarrierAccess::ShaderRead))
			{
				result |= D3D12_BARRIER_ACCESS_SHADER_RESOURCE;
			}

			if (EnumValueContainsFlag(barrierAccess, BarrierAccess::IndirectArgument))
			{
				result |= D3D12_BARRIER_ACCESS_INDIRECT_ARGUMENT;
			}

			if (EnumValueContainsFlag(barrierAccess, BarrierAccess::CopyDest))
			{
				result |= D3D12_BARRIER_ACCESS_COPY_DEST;
			}

			if (EnumValueContainsFlag(barrierAccess, BarrierAccess::CopySource))
			{
				result |= D3D12_BARRIER_ACCESS_COPY_SOURCE;
			}

			if (EnumValueContainsFlag(barrierAccess, BarrierAccess::ResolveDest))
			{
				result |= D3D12_BARRIER_ACCESS_RESOLVE_DEST;
			}

			if (EnumValueContainsFlag(barrierAccess, BarrierAccess::ResolveSource))
			{
				result |= D3D12_BARRIER_ACCESS_RESOLVE_SOURCE;
			}

			if (EnumValueContainsFlag(barrierAccess, BarrierAccess::VideoEncodeRead))
			{
				result |= D3D12_BARRIER_ACCESS_VIDEO_ENCODE_READ;
			}

			if (EnumValueContainsFlag(barrierAccess, BarrierAccess::VideoEncodeWrite))
			{
				result |= D3D12_BARRIER_ACCESS_VIDEO_ENCODE_WRITE;
			}

			if (EnumValueContainsFlag(barrierAccess, BarrierAccess::VideoDecodeRead))
			{
				result |= D3D12_BARRIER_ACCESS_VIDEO_DECODE_READ;
			}

			if (EnumValueContainsFlag(barrierAccess, BarrierAccess::VideoDecodeWrite))
			{
				result |= D3D12_BARRIER_ACCESS_VIDEO_DECODE_WRITE;
			}

			return result;
		}

		inline D3D12_BARRIER_LAYOUT GetD3D12BarrierLayoutFromImageLayout(const ImageLayout layout)
		{
			switch (layout)
			{
				case ImageLayout::Undefined: return D3D12_BARRIER_LAYOUT_UNDEFINED;
				case ImageLayout::Present: return D3D12_BARRIER_LAYOUT_PRESENT;
				case ImageLayout::RenderTarget: return D3D12_BARRIER_LAYOUT_RENDER_TARGET;
				case ImageLayout::ShaderWrite: return D3D12_BARRIER_LAYOUT_UNORDERED_ACCESS;
				case ImageLayout::DepthStencilWrite: return D3D12_BARRIER_LAYOUT_DEPTH_STENCIL_WRITE;
				case ImageLayout::DepthStencilRead: return D3D12_BARRIER_LAYOUT_DEPTH_STENCIL_READ;
				case ImageLayout::ShaderRead: return D3D12_BARRIER_LAYOUT_SHADER_RESOURCE;
				case ImageLayout::CopySource: return D3D12_BARRIER_LAYOUT_COPY_SOURCE;
				case ImageLayout::CopyDest: return D3D12_BARRIER_LAYOUT_COPY_DEST;
				case ImageLayout::ResolveSource: return D3D12_BARRIER_LAYOUT_RESOLVE_SOURCE;
				case ImageLayout::ResolveDest: return D3D12_BARRIER_LAYOUT_RESOLVE_DEST;
				case ImageLayout::VideoDecodeRead: return D3D12_BARRIER_LAYOUT_VIDEO_ENCODE_READ;
				case ImageLayout::VideoDecodeWrite: return D3D12_BARRIER_LAYOUT_VIDEO_ENCODE_WRITE;
				case ImageLayout::VideoEncodeRead: return D3D12_BARRIER_LAYOUT_VIDEO_DECODE_READ;
				case ImageLayout::VideoEncodeWrite: return D3D12_BARRIER_LAYOUT_VIDEO_DECODE_READ;
			}

			VT_ENSURE(false);
			return D3D12_BARRIER_LAYOUT_UNDEFINED;
		}
	}

	D3D12CommandBuffer::D3D12CommandBuffer(QueueType queueType)
		: m_queueType(queueType)
	{
		Invalidate();
	}

	D3D12CommandBuffer::~D3D12CommandBuffer()
	{
		Release();
	}

	void* D3D12CommandBuffer::GetHandleImpl() const
	{
		return m_commandListData.commandList.Get();
	}

	void D3D12CommandBuffer::Invalidate()
	{
		auto device = GraphicsContext::GetDevice();
		ID3D12Device2* devicePtr = device->GetHandle<ID3D12Device2*>();

		auto d3d12QueueType = GetD3D12QueueType(m_queueType);

		VT_D3D12_CHECK(devicePtr->CreateCommandAllocator(d3d12QueueType, VT_D3D12_ID(m_commandListData.commandAllocator)));
		VT_D3D12_CHECK(devicePtr->CreateCommandList(0, d3d12QueueType, m_commandListData.commandAllocator.Get(), nullptr, VT_D3D12_ID(m_commandListData.commandList)));
		m_commandListData.commandList->Close();

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

		m_commandListData.commandList->SetName(name.c_str());

		SemaphoreCreateInfo info{};
		info.initialValue = 0;
		m_commandListData.fence = Semaphore::Create(info);

		// Temp descriptors
		{
			DescriptorHeapSpecification spec{};
			spec.descriptorType = D3D12DescriptorType::CBV_SRV_UAV;
			spec.maxDescriptorCount = 500;
			spec.supportsGPUDescriptors = true;
			m_descriptorHeap = CreateScope<D3D12DescriptorHeap>(spec);
		}
	}

	void D3D12CommandBuffer::Release()
	{
		WaitForFence();

		m_descriptorHeap = nullptr;
	}

	void D3D12CommandBuffer::BindPipelineInternal()
	{
		if (!m_pipelineNeedsToBeBound)
		{
			return;
		}

		auto& cmdList = m_commandListData.commandList;

		if (m_currentComputePipeline)
		{
			D3D12Shader& d3d12Shader = m_currentComputePipeline->GetShader()->AsRef<D3D12Shader>();

			cmdList->SetComputeRootSignature(d3d12Shader.GetRootSignature().Get());
			cmdList->SetPipelineState(m_currentComputePipeline->GetHandle<ID3D12PipelineState*>());
		}
		else
		{
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

		m_pipelineNeedsToBeBound = false;
	}

	D3D12DescriptorPointer D3D12CommandBuffer::CreateTempDescriptorPointer()
	{
		D3D12DescriptorPointer ptr = m_descriptorHeap->Allocate();
		m_allocatedDescriptors.emplace_back(ptr);
		return ptr;
	}

	void D3D12CommandBuffer::Begin()
	{
		VT_PROFILE_FUNCTION();

		m_commandListData.fence->Wait();

		for (const auto& descriptor : m_allocatedDescriptors)
		{
			m_descriptorHeap->Free(descriptor);
		}

		VT_D3D12_CHECK(m_commandListData.commandAllocator->Reset());
		VT_D3D12_CHECK(m_commandListData.commandList->Reset(m_commandListData.commandAllocator.Get(), nullptr));

		BeginMarker("CommandBuffer", { 1.f, 1.f, 1.f, 1.f });
	}

	void D3D12CommandBuffer::End()
	{
		VT_PROFILE_FUNCTION();

		EndMarker();
		m_commandListData.commandList->Close();
	}

	void D3D12CommandBuffer::Flush(RefPtr<Fence> fence)
	{
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

		m_commandListData.fence->Wait();
	}

	void D3D12CommandBuffer::WaitForFence()
	{
		VT_PROFILE_FUNCTION();

		auto device = GraphicsContext::GetDevice();
		auto queue = device->GetDeviceQueue(m_queueType)->GetHandle<ID3D12CommandQueue*>();

		queue->Signal(m_commandListData.fence->GetHandle<ID3D12Fence*>(), m_commandListData.fence->IncrementAndGetValue());
		m_commandListData.fence->Wait();
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

		m_commandListData.commandList->DrawInstanced(vertexCount, instanceCount, firstVertex, firstInstance);
	}

	void D3D12CommandBuffer::DrawIndexed(const uint32_t indexCount, const uint32_t instanceCount, const uint32_t firstIndex, const uint32_t vertexOffset, const uint32_t firstInstance)
	{
		VT_PROFILE_FUNCTION();

#ifdef VT_ENABLE_COMMAND_BUFFER_VALIDATION
		VT_ENSURE(m_currentRenderPipeline != nullptr);
#endif

		m_commandListData.commandList->DrawIndexedInstanced(indexCount, instanceCount, firstIndex, vertexOffset, firstInstance);
	}

	void D3D12CommandBuffer::DrawIndexedIndirect(WeakPtr<StorageBuffer> commandsBuffer, const size_t offset, const uint32_t drawCount, const uint32_t stride)
	{
		VT_PROFILE_FUNCTION();

#ifdef VT_ENABLE_COMMAND_BUFFER_VALIDATION
		VT_ENSURE(m_currentRenderPipeline != nullptr);
#endif

		ComPtr<ID3D12CommandSignature> signature = CommandSignatureCache::Get().GetOrCreateCommandSignature(CommandSignatureType::DrawIndexed, stride);
		m_commandListData.commandList->ExecuteIndirect(signature.Get(), drawCount, commandsBuffer->GetHandle<ID3D12Resource*>(), offset, nullptr, 0);
	}

	void D3D12CommandBuffer::DrawIndirect(WeakPtr<StorageBuffer> commandsBuffer, const size_t offset, const uint32_t drawCount, const uint32_t stride)
	{
		VT_PROFILE_FUNCTION();

#ifdef VT_ENABLE_COMMAND_BUFFER_VALIDATION
		VT_ENSURE(m_currentRenderPipeline != nullptr);
#endif

		ComPtr<ID3D12CommandSignature> signature = CommandSignatureCache::Get().GetOrCreateCommandSignature(CommandSignatureType::Draw, stride);
		m_commandListData.commandList->ExecuteIndirect(signature.Get(), drawCount, commandsBuffer->GetHandle<ID3D12Resource*>(), offset, nullptr, 0);
	}

	void D3D12CommandBuffer::DrawIndirectCount(WeakPtr<StorageBuffer> commandsBuffer, const size_t offset, WeakPtr<StorageBuffer> countBuffer, const size_t countBufferOffset, const uint32_t maxDrawCount, const uint32_t stride)
	{
		VT_PROFILE_FUNCTION();

#ifdef VT_ENABLE_COMMAND_BUFFER_VALIDATION
		VT_ENSURE(m_currentRenderPipeline != nullptr);
#endif

		ComPtr<ID3D12CommandSignature> signature = CommandSignatureCache::Get().GetOrCreateCommandSignature(CommandSignatureType::Draw, stride);
		m_commandListData.commandList->ExecuteIndirect(signature.Get(), maxDrawCount, commandsBuffer->GetHandle<ID3D12Resource*>(), offset, countBuffer->GetHandle<ID3D12Resource*>(), countBufferOffset);
	}

	void D3D12CommandBuffer::DrawIndexedIndirectCount(WeakPtr<StorageBuffer> commandsBuffer, const size_t offset, WeakPtr<StorageBuffer> countBuffer, const size_t countBufferOffset, const uint32_t maxDrawCount, const uint32_t stride)
	{
		VT_PROFILE_FUNCTION();

#ifdef VT_ENABLE_COMMAND_BUFFER_VALIDATION
		VT_ENSURE(m_currentRenderPipeline != nullptr);
#endif

		ComPtr<ID3D12CommandSignature> signature = CommandSignatureCache::Get().GetOrCreateCommandSignature(CommandSignatureType::DrawIndexed, stride);
		m_commandListData.commandList->ExecuteIndirect(signature.Get(), maxDrawCount, commandsBuffer->GetHandle<ID3D12Resource*>(), offset, countBuffer->GetHandle<ID3D12Resource*>(), countBufferOffset);
	}

	void D3D12CommandBuffer::DispatchMeshTasks(const uint32_t groupCountX, const uint32_t groupCountY, const uint32_t groupCountZ)
	{
		VT_PROFILE_FUNCTION();

#ifdef VT_ENABLE_COMMAND_BUFFER_VALIDATION
		VT_ENSURE(m_currentRenderPipeline != nullptr);
#endif

		m_commandListData.commandList->DispatchMesh(groupCountX, groupCountY, groupCountZ);
	}

	void D3D12CommandBuffer::DispatchMeshTasksIndirect(WeakPtr<StorageBuffer> commandsBuffer, const size_t offset, const uint32_t drawCount, const uint32_t stride)
	{
		VT_PROFILE_FUNCTION();

#ifdef VT_ENABLE_COMMAND_BUFFER_VALIDATION
		VT_ENSURE(m_currentRenderPipeline != nullptr);
#endif

		ComPtr<ID3D12CommandSignature> signature = CommandSignatureCache::Get().GetOrCreateCommandSignature(CommandSignatureType::DispatchMesh, stride);
		m_commandListData.commandList->ExecuteIndirect(signature.Get(), drawCount, commandsBuffer->GetHandle<ID3D12Resource*>(), offset, nullptr, 0);
	}

	void D3D12CommandBuffer::DispatchMeshTasksIndirectCount(WeakPtr<StorageBuffer> commandsBuffer, const size_t offset, WeakPtr<StorageBuffer> countBuffer, const size_t countBufferOffset, const uint32_t maxDrawCount, const uint32_t stride)
	{
		VT_PROFILE_FUNCTION();

#ifdef VT_ENABLE_COMMAND_BUFFER_VALIDATION
		VT_ENSURE(m_currentRenderPipeline != nullptr);
#endif

		ComPtr<ID3D12CommandSignature> signature = CommandSignatureCache::Get().GetOrCreateCommandSignature(CommandSignatureType::DispatchMesh, stride);
		m_commandListData.commandList->ExecuteIndirect(signature.Get(), maxDrawCount, commandsBuffer->GetHandle<ID3D12Resource*>(), offset, countBuffer->GetHandle<ID3D12Resource*>(), countBufferOffset);
	}

	void D3D12CommandBuffer::Dispatch(const uint32_t groupCountX, const uint32_t groupCountY, const uint32_t groupCountZ)
	{
		VT_PROFILE_FUNCTION();

#ifdef VT_ENABLE_COMMAND_BUFFER_VALIDATION
		VT_ENSURE(m_currentComputePipeline != nullptr);
#endif

		if (groupCountX == 0 || groupCountY == 0 || groupCountZ == 0)
		{
			return;
		}

		m_commandListData.commandList->Dispatch(groupCountX, groupCountY, groupCountZ);
	}

	void D3D12CommandBuffer::DispatchIndirect(WeakPtr<StorageBuffer> commandsBuffer, const size_t offset)
	{
		VT_PROFILE_FUNCTION();

#ifdef VT_ENABLE_COMMAND_BUFFER_VALIDATION
		VT_ENSURE(m_currentComputePipeline != nullptr);
#endif

		ComPtr<ID3D12CommandSignature> signature = CommandSignatureCache::Get().GetOrCreateCommandSignature(CommandSignatureType::Dispatch, sizeof(IndirectDispatchCommand));
		m_commandListData.commandList->ExecuteIndirect(signature.Get(), 1, commandsBuffer->GetHandle<ID3D12Resource*>(), offset, nullptr, 0);
	}

	void D3D12CommandBuffer::SetViewports(const StackVector<Viewport, MAX_VIEWPORT_COUNT>& viewports)
	{
		VT_PROFILE_FUNCTION();

		m_commandListData.commandList->RSSetViewports(static_cast<uint32_t>(viewports.Size()), reinterpret_cast<const D3D12_VIEWPORT*>(viewports.Data()));
	}

	void D3D12CommandBuffer::SetScissors(const StackVector<Rect2D, MAX_VIEWPORT_COUNT>& scissors)
	{
		VT_PROFILE_FUNCTION();

		m_commandListData.commandList->RSSetScissorRects(static_cast<uint32_t>(scissors.Size()), reinterpret_cast<const D3D12_RECT*>(scissors.Data()));
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
		m_pipelineNeedsToBeBound = true;
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
		m_pipelineNeedsToBeBound = true;
	}

	void D3D12CommandBuffer::BindVertexBuffers(const StackVector<WeakPtr<VertexBuffer>, RHI::MAX_VERTEX_BUFFER_COUNT>& vertexBuffers, const uint32_t firstBinding)
	{
		VT_PROFILE_FUNCTION();

		StackVector<D3D12_VERTEX_BUFFER_VIEW, RHI::MAX_VERTEX_BUFFER_COUNT> views;

		for (const auto& buffer : vertexBuffers)
		{
			auto& newView = views.EmplaceBack();
			newView.BufferLocation = buffer->GetHandle<ID3D12Resource*>()->GetGPUVirtualAddress();
			newView.SizeInBytes = static_cast<uint32_t>(buffer->GetByteSize());
			newView.StrideInBytes = buffer->GetStride();
		}

		m_commandListData.commandList->IASetVertexBuffers(firstBinding, static_cast<uint32_t>(views.Size()), views.Data());
	}

	void D3D12CommandBuffer::BindVertexBuffers(const StackVector<WeakPtr<StorageBuffer>, RHI::MAX_VERTEX_BUFFER_COUNT>& vertexBuffers, const uint32_t firstBinding)
	{
		VT_PROFILE_FUNCTION();

		StackVector<D3D12_VERTEX_BUFFER_VIEW, RHI::MAX_VERTEX_BUFFER_COUNT> views;

		for (const auto& buffer : vertexBuffers)
		{
			auto& newView = views.EmplaceBack();
			newView.BufferLocation = buffer->GetHandle<ID3D12Resource*>()->GetGPUVirtualAddress();
			newView.SizeInBytes = static_cast<uint32_t>(buffer->GetByteSize());
			newView.StrideInBytes = static_cast<uint32_t>(buffer->GetElementSize());
		}

		m_commandListData.commandList->IASetVertexBuffers(firstBinding, static_cast<uint32_t>(views.Size()), views.Data());
	}

	void D3D12CommandBuffer::BindIndexBuffer(WeakPtr<IndexBuffer> indexBuffer)
	{
		VT_PROFILE_FUNCTION();

		D3D12_INDEX_BUFFER_VIEW view{};
		view.BufferLocation = indexBuffer->GetHandle<ID3D12Resource*>()->GetGPUVirtualAddress();
		view.Format = DXGI_FORMAT_R32_UINT;
		view.SizeInBytes = static_cast<uint32_t>(indexBuffer->GetByteSize());

		m_commandListData.commandList->IASetIndexBuffer(&view);
	}

	void D3D12CommandBuffer::BindIndexBuffer(WeakPtr<StorageBuffer> indexBuffer)
	{
		VT_PROFILE_FUNCTION();

		D3D12_INDEX_BUFFER_VIEW view{};
		view.BufferLocation = indexBuffer->GetHandle<ID3D12Resource*>()->GetGPUVirtualAddress();
		view.Format = DXGI_FORMAT_R32_UINT;
		view.SizeInBytes = static_cast<uint32_t>(indexBuffer->GetByteSize());

		m_commandListData.commandList->IASetIndexBuffer(&view);
	}

	void D3D12CommandBuffer::BindDescriptorTable(WeakPtr<DescriptorTable> descriptorTable)
	{
		VT_PROFILE_FUNCTION();
		descriptorTable->AsRef<D3D12DescriptorTable>().Bind(*this);
		BindPipelineInternal();
		descriptorTable->AsRef<D3D12DescriptorTable>().SetRootParameters(*this);
	}

	void D3D12CommandBuffer::BindDescriptorTable(WeakPtr<BindlessDescriptorTable> descriptorTable)
	{
		VT_PROFILE_FUNCTION();
		descriptorTable->AsRef<D3D12BindlessDescriptorTable>().Bind(*this);
		BindPipelineInternal();
		descriptorTable->AsRef<D3D12BindlessDescriptorTable>().SetRootParameters(*this);
	}

	void D3D12CommandBuffer::BeginRendering(const RenderingInfo& renderingInfo)
	{
		VT_PROFILE_FUNCTION();

		Vector<D3D12_CPU_DESCRIPTOR_HANDLE> rtvViews;
		rtvViews.reserve(renderingInfo.colorAttachments.Size());

		D3D12_CPU_DESCRIPTOR_HANDLE* dsvView = nullptr;

		for (auto& attachment : renderingInfo.colorAttachments)
		{
			auto view = attachment.view;
			auto viewHandle = D3D12_CPU_DESCRIPTOR_HANDLE(view.As<D3D12ImageView>()->GetRTVDSVDescriptor().GetCPUPointer());

			if (attachment.clearMode == ClearMode::Clear)
			{
				m_commandListData.commandList->ClearRenderTargetView(viewHandle, attachment.clearColor.float32, 0, nullptr);
			}
			rtvViews.emplace_back(viewHandle);
		}

		if (renderingInfo.depthAttachmentInfo.view)
		{
			auto view = renderingInfo.depthAttachmentInfo.view;
			auto viewHandle = D3D12_CPU_DESCRIPTOR_HANDLE(view.As<D3D12ImageView>()->GetRTVDSVDescriptor().GetCPUPointer());

			if (renderingInfo.depthAttachmentInfo.clearMode == ClearMode::Clear)
			{

				m_commandListData.commandList->ClearDepthStencilView(viewHandle, D3D12_CLEAR_FLAG_DEPTH, 1, 0, 0, nullptr);
			}

			dsvView = &viewHandle;
		}

		m_commandListData.commandList->OMSetRenderTargets(static_cast<UINT>(rtvViews.size()), rtvViews.data(), false, dsvView);
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

		const uint32_t numValues = size / sizeof(uint32_t);
		const uint32_t numOffsetValues = offset / sizeof(uint32_t);

		if (m_currentRenderPipeline)
		{
			m_commandListData.commandList->SetGraphicsRoot32BitConstants(m_currentRenderPipeline->GetShader()->As<D3D12Shader>()->GetPushConstantsRootParameterIndex(), numValues, data, numOffsetValues);
		}
		else
		{
			m_commandListData.commandList->SetComputeRoot32BitConstants(m_currentRenderPipeline->GetShader()->As<D3D12Shader>()->GetPushConstantsRootParameterIndex(), numValues, data, numOffsetValues);
		}
	}

	inline void AddGlobalBarrier(const GlobalBarrier& barrierInfo, D3D12_GLOBAL_BARRIER& barrier)
	{
		barrier.SyncBefore = Utility::GetD3D12BarrierSyncFromBarrierStage(barrierInfo.srcStage);
		barrier.AccessBefore = Utility::GetD3D12BarrierAccessFromBarrierAccess(barrierInfo.srcAccess);
		barrier.SyncAfter = Utility::GetD3D12BarrierSyncFromBarrierStage(barrierInfo.dstStage);
		barrier.AccessAfter = Utility::GetD3D12BarrierAccessFromBarrierAccess(barrierInfo.dstAccess);
	}

	inline void AddBufferBarrier(const BufferBarrier& barrierInfo, D3D12_BUFFER_BARRIER& barrier)
	{
		VT_ENSURE(barrierInfo.resource != nullptr);

		if (barrierInfo.srcStage == BarrierStage::Clear)
		{
			barrier.SyncBefore = D3D12_BARRIER_SYNC_CLEAR_UNORDERED_ACCESS_VIEW;
			barrier.AccessBefore = D3D12_BARRIER_ACCESS_UNORDERED_ACCESS;
		}
		else
		{
			barrier.SyncBefore = Utility::GetD3D12BarrierSyncFromBarrierStage(barrierInfo.srcStage);
			barrier.AccessBefore = Utility::GetD3D12BarrierAccessFromBarrierAccess(barrierInfo.srcAccess);
		}

		if (barrierInfo.dstStage == BarrierStage::Clear)
		{
			barrier.SyncAfter = D3D12_BARRIER_SYNC_CLEAR_UNORDERED_ACCESS_VIEW;
			barrier.AccessAfter = D3D12_BARRIER_ACCESS_UNORDERED_ACCESS;
		}
		else
		{
			barrier.SyncAfter = Utility::GetD3D12BarrierSyncFromBarrierStage(barrierInfo.dstStage);
			barrier.AccessAfter = Utility::GetD3D12BarrierAccessFromBarrierAccess(barrierInfo.dstAccess);
		}

		barrier.Offset = 0;
		barrier.Size = barrierInfo.size;
		barrier.pResource = barrierInfo.resource->GetHandle<ID3D12Resource*>();

		GraphicsContext::GetResourceStateTracker()->TransitionResource(barrierInfo.resource, barrierInfo.dstStage, barrierInfo.dstAccess);
	}

	inline void AddImageBarrier(const ImageBarrier& barrierInfo, D3D12_TEXTURE_BARRIER& barrier)
	{
		VT_ENSURE(barrierInfo.resource != nullptr);

		if (barrierInfo.srcStage == BarrierStage::Clear)
		{
			if (barrierInfo.resource->GetType() == ResourceType::Image2D)
			{
				const auto aspectMask = barrierInfo.resource->As<Image>()->GetImageAspect();
				if (EnumValueContainsFlag(aspectMask, ImageAspect::Depth) || EnumValueContainsFlag(aspectMask, ImageAspect::Stencil))
				{
					barrier.SyncBefore = D3D12_BARRIER_SYNC_DEPTH_STENCIL;
				}
				else
				{
					barrier.SyncBefore = D3D12_BARRIER_SYNC_RENDER_TARGET;
				}
			}
		}
		else
		{
			barrier.SyncBefore = Utility::GetD3D12BarrierSyncFromBarrierStage(barrierInfo.srcStage);
		}

		if (barrierInfo.dstStage == BarrierStage::Clear)
		{
			if (barrierInfo.resource->GetType() == ResourceType::Image2D)
			{
				const auto aspectMask = barrierInfo.resource->As<Image>()->GetImageAspect();
				if (EnumValueContainsFlag(aspectMask, ImageAspect::Depth) || EnumValueContainsFlag(aspectMask, ImageAspect::Stencil))
				{
					barrier.SyncAfter = D3D12_BARRIER_SYNC_DEPTH_STENCIL;
				}
				else
				{
					barrier.SyncAfter = D3D12_BARRIER_SYNC_RENDER_TARGET;
				}
			}
		}

		barrier.AccessBefore = Utility::GetD3D12BarrierAccessFromBarrierAccess(barrierInfo.srcAccess);
		barrier.SyncAfter = Utility::GetD3D12BarrierSyncFromBarrierStage(barrierInfo.dstStage);
		barrier.AccessAfter = Utility::GetD3D12BarrierAccessFromBarrierAccess(barrierInfo.dstAccess);
		barrier.LayoutBefore = Utility::GetD3D12BarrierLayoutFromImageLayout(barrierInfo.srcLayout);
		barrier.LayoutAfter = Utility::GetD3D12BarrierLayoutFromImageLayout(barrierInfo.dstLayout);
		barrier.pResource = barrierInfo.resource->GetHandle<ID3D12Resource*>();
		barrier.Flags = D3D12_TEXTURE_BARRIER_FLAG_NONE;

		auto levelCount = barrierInfo.subResource.levelCount;
		if (levelCount == ALL_MIPS)
		{
			if (barrierInfo.resource->GetType() == ResourceType::Image2D)
			{
				levelCount = barrierInfo.resource->As<Image>()->GetMipCount();
			}
		}

		auto layerCount = barrierInfo.subResource.layerCount;
		if (layerCount == ALL_LAYERS)
		{
			if (barrierInfo.resource->GetType() == ResourceType::Image2D)
			{
				layerCount = barrierInfo.resource->As<Image>()->GetLayerCount();
			}
		}

		barrier.Subresources.NumMipLevels = levelCount;
		barrier.Subresources.NumArraySlices = layerCount;
		barrier.Subresources.FirstArraySlice = barrierInfo.subResource.baseArrayLayer;
		barrier.Subresources.IndexOrFirstMipLevel = barrierInfo.subResource.baseMipLevel;
		barrier.Subresources.FirstPlane = 0;
		barrier.Subresources.NumPlanes = 0;

		GraphicsContext::GetResourceStateTracker()->TransitionResource(barrierInfo.resource, barrierInfo.dstStage, barrierInfo.dstAccess, barrierInfo.dstLayout);
	}

	void D3D12CommandBuffer::ResourceBarrier(const Vector<ResourceBarrierInfo>& resourceBarriers)
	{
		VT_PROFILE_FUNCTION();

		Vector<D3D12_GLOBAL_BARRIER> globalBarriers;
		Vector<D3D12_TEXTURE_BARRIER> textureBarriers;
		Vector<D3D12_BUFFER_BARRIER> bufferBarriers;

		for (const auto& resourceBarrier : resourceBarriers)
		{
			VT_ENSURE(resourceBarrier.type != BarrierType::None);

			switch (resourceBarrier.type)
			{
				case BarrierType::Global:
					AddGlobalBarrier(resourceBarrier.globalBarrier(), globalBarriers.emplace_back());
					break;

				case BarrierType::Buffer:
					AddBufferBarrier(resourceBarrier.bufferBarrier(), bufferBarriers.emplace_back());
					break;

				case BarrierType::Image:
					AddImageBarrier(resourceBarrier.imageBarrier(), textureBarriers.emplace_back());
					break;
			}
		}

		Vector<D3D12_BARRIER_GROUP> barrierGroups{};

		if (!globalBarriers.empty())
		{
			auto& group = barrierGroups.emplace_back();
			group.Type = D3D12_BARRIER_TYPE_GLOBAL;
			group.NumBarriers = static_cast<uint32_t>(globalBarriers.size());
			group.pGlobalBarriers = globalBarriers.data();
		}

		if (!textureBarriers.empty())
		{
			auto& group = barrierGroups.emplace_back();
			group.Type = D3D12_BARRIER_TYPE_TEXTURE;
			group.NumBarriers = static_cast<uint32_t>(textureBarriers.size());
			group.pTextureBarriers = textureBarriers.data();
		}

		if (!bufferBarriers.empty())
		{
			auto& group = barrierGroups.emplace_back();
			group.Type = D3D12_BARRIER_TYPE_BUFFER;
			group.NumBarriers = static_cast<uint32_t>(bufferBarriers.size());
			group.pBufferBarriers = bufferBarriers.data();
		}

		if (!barrierGroups.empty())
		{
			m_commandListData.commandList->Barrier(static_cast<uint32_t>(barrierGroups.size()), barrierGroups.data());
		}
	}

	void D3D12CommandBuffer::BeginMarker(std::string_view markerLabel, const std::array<float, 4>& markerColor)
	{
		uint32_t color = PIX_COLOR(static_cast<BYTE>(markerColor[0] * 255.f), static_cast<BYTE>(markerColor[1] * 255.f), static_cast<BYTE>(markerColor[2] * 255.f));
		PIXBeginEvent(m_commandListData.commandList.Get(), color, markerLabel.data());
	}

	void D3D12CommandBuffer::EndMarker()
	{
		PIXEndEvent(m_commandListData.commandList.Get());
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

	void D3D12CommandBuffer::ClearImage(WeakPtr<Image> image, std::array<float, 4> clearColor)
	{
		VT_PROFILE_FUNCTION();

		auto imageView = image->GetView();
		auto& d3d12View = imageView->AsRef<D3D12ImageView>();

		D3D12_RECT rect{};
		rect.top = 0;
		rect.left = 0;
		rect.bottom = image->GetHeight();
		rect.right = image->GetWidth();

		if ((image->GetImageAspect() & ImageAspect::Depth) != ImageAspect::None)
		{
			m_commandListData.commandList->ClearDepthStencilView(D3D12_CPU_DESCRIPTOR_HANDLE(d3d12View.GetRTVDSVDescriptor().GetCPUPointer()), D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL, clearColor[0], static_cast<uint8_t>(clearColor[1]), 1, &rect);
		}
		else
		{
			m_commandListData.commandList->ClearRenderTargetView(D3D12_CPU_DESCRIPTOR_HANDLE(d3d12View.GetRTVDSVDescriptor().GetCPUPointer()), clearColor.data(), 1, &rect);
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

		m_commandListData.commandList->SetDescriptorHeaps(1, &heap);
		m_commandListData.commandList->ClearUnorderedAccessViewUint(D3D12_GPU_DESCRIPTOR_HANDLE(tempDescriptor.GetGPUPointer()), D3D12_CPU_DESCRIPTOR_HANDLE(srcDescriptor.GetCPUPointer()), buffer->GetHandle<ID3D12Resource*>(), values, 0, nullptr);
	}

	void D3D12CommandBuffer::UpdateBuffer(WeakPtr<StorageBuffer> dstBuffer, const size_t dstOffset, const size_t dataSize, const void* data)
	{
	}

	void D3D12CommandBuffer::CopyBufferRegion(WeakPtr<Allocation> srcResource, const size_t srcOffset, WeakPtr<Allocation> dstResource, const size_t dstOffset, const size_t size)
	{
		VT_PROFILE_FUNCTION();

		m_commandListData.commandList->CopyBufferRegion(dstResource->GetResourceHandle<ID3D12Resource*>(), dstOffset, srcResource->GetResourceHandle<ID3D12Resource*>(), srcOffset, size);
	}

	void D3D12CommandBuffer::CopyBufferToImage(WeakPtr<Allocation> srcBuffer, WeakPtr<Image> dstImage, const uint32_t width, const uint32_t height, const uint32_t depth, const uint32_t mip)
	{
	}

	void D3D12CommandBuffer::CopyImageToBuffer(WeakPtr<Image> srcImage, WeakPtr<Allocation> dstBuffer, const size_t dstOffset, const uint32_t width, const uint32_t height, const uint32_t depth, const uint32_t mip)
	{
	}

	void D3D12CommandBuffer::CopyImage(WeakPtr<Image> srcImage, WeakPtr<Image> dstImage, const uint32_t width, const uint32_t height, const uint32_t depth)
	{
	}

	void D3D12CommandBuffer::UploadTextureData(WeakPtr<Image> dstImage, const ImageCopyData& copyData)
	{
		Vector<D3D12_SUBRESOURCE_DATA> subResources;
		subResources.reserve(copyData.copySubData.size());

		for (const auto& subData : copyData.copySubData)
		{
			auto& newSubResource = subResources.emplace_back();
			newSubResource.pData = subData.data;
			newSubResource.RowPitch = subData.rowPitch;
			newSubResource.SlicePitch = subData.slicePitch;
		}

		ID3D12Resource* d3d12Image = dstImage->GetHandle<ID3D12Resource*>();

		const uint32_t subResourceCount = static_cast<uint32_t>(subResources.size());
		const uint64_t requiredSize = GetRequiredIntermediateSize(d3d12Image, 0, subResourceCount);
		RefPtr<Allocation> stagingAlloc = GraphicsContext::GetDefaultAllocator()->CreateBuffer(requiredSize, BufferUsage::TransferSrc, MemoryUsage::CPUToGPU);
		UpdateSubresources(m_commandListData.commandList.Get(), d3d12Image, stagingAlloc->GetResourceHandle<ID3D12Resource*>(), 0, 0, subResourceCount, subResources.data());

		RHIProxy::GetInstance().DestroyResource([stagingAlloc]()
		{
			GraphicsContext::GetDefaultAllocator()->DestroyBuffer(stagingAlloc);
		});
	}

	const QueueType D3D12CommandBuffer::GetQueueType() const
	{
		return m_queueType;
	}
	const WeakPtr<Fence> D3D12CommandBuffer::GetFence() const
	{
		return WeakPtr<Fence>();
	}
}
