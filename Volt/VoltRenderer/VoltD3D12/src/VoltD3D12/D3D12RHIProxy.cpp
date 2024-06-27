#include "dxpch.h"
#include "D3D12RHIProxy.h"

#include "VoltD3D12/Graphics/D3D12Swapchain.h"
#include "VoltD3D12/Graphics/D3D12DeviceQueue.h"
#include "VoltD3D12/Graphics/D3D12GraphicsContext.h"
#include "VoltD3D12/Graphics/D3D12PhysicalGraphicsDevice.h"
#include "VoltD3D12/Graphics/D3D12GraphicsDevice.h"

#include "VoltD3D12/Descriptors/D3D12DescriptorTable.h"

#include <VoltRHI/Buffers/UniformBuffer.h>
#include <VoltRHI/Buffers/StorageBuffer.h>
#include <VoltRHI/Buffers/VertexBuffer.h>
#include <VoltRHI/Buffers/IndexBuffer.h>
#include <VoltRHI/Buffers/BufferView.h>
#include "VoltD3D12/Buffers/D3D12CommandBuffer.h"

#include "VoltD3D12/ImGui/D3D12ImGuiImplementation.h"

#include "VoltD3D12/Shader/D3D12ShaderCompiler.h"
#include "VoltD3D12/Shader/D3D12Shader.h"

#include "VoltD3D12/Pipelines/D3D12RenderPipeline.h"
#include "VoltD3D12/Pipelines/D3D12ComputePipeline.h"

#include <VoltRHI/Memory/TransientHeap.h>
#include "VoltD3D12/Memory/D3D12DefaultAllocator.h"

#include <VoltRHI/Images/SamplerState.h>
#include "VoltD3D12/Images/D3D12ImageView.h"
#include "VoltD3D12/Images/D3D12Image2D.h"

#include "VoltD3D12/Synchronization/D3D12Semaphore.h"
#include <VoltRHI/Synchronization/Fence.h>
#include <VoltRHI/Synchronization/Event.h>

namespace Volt::RHI
{
	D3D12RHIProxy::D3D12RHIProxy()
	{
		s_instance = this;
	}
	
	RefPtr<BufferView> D3D12RHIProxy::CreateBufferView(const BufferViewSpecification& specification) const
	{
		return RefPtr<BufferView>();
	}
	
	RefPtr<CommandBuffer> D3D12RHIProxy::CreateCommandBuffer(const uint32_t count, QueueType queueType) const
	{
		return RefPtr<D3D12CommandBuffer>::Create(count, queueType);
	}
	
	RefPtr<CommandBuffer> D3D12RHIProxy::CreateCommandBuffer(WeakPtr<Swapchain> swapchain) const
	{
		return RefPtr<D3D12CommandBuffer>::Create(swapchain);
	}
	
	RefPtr<IndexBuffer> D3D12RHIProxy::CreateIndexBuffer(std::span<const uint32_t> indices) const
	{
		return RefPtr<IndexBuffer>();
	}
	
	RefPtr<VertexBuffer> D3D12RHIProxy::CreateVertexBuffer(const uint32_t size, const void* data) const
	{
		return RefPtr<VertexBuffer>();
	}
	
	RefPtr<StorageBuffer> D3D12RHIProxy::CreateStorageBuffer(const uint32_t count, const size_t elementSize, std::string_view name, const BufferUsage bufferUsage, const MemoryUsage memoryUsage) const
	{
		return RefPtr<StorageBuffer>();
	}
	
	RefPtr<StorageBuffer> D3D12RHIProxy::CreateStorageBuffer(const size_t size, std::string_view name, const BufferUsage bufferUsage, const MemoryUsage memoryUsage) const
	{
		return RefPtr<StorageBuffer>();
	}
	
	RefPtr<StorageBuffer> D3D12RHIProxy::CreateStorageBuffer(const size_t size, RefPtr<Allocator> customAllocator, std::string_view name, const BufferUsage bufferUsage, const MemoryUsage memoryUsage) const
	{
		return RefPtr<StorageBuffer>();
	}
	
	RefPtr<UniformBuffer> D3D12RHIProxy::CreateUniformBuffer(const uint32_t size, const void* data) const
	{
		return RefPtr<UniformBuffer>();
	}
	
	RefPtr<DescriptorTable> D3D12RHIProxy::CreateDescriptorTable(const DescriptorTableCreateInfo& createInfo) const
	{
		return RefPtr<D3D12DescriptorTable>::Create(createInfo);
	}
	
	RefPtr<DeviceQueue> D3D12RHIProxy::CreateDeviceQueue(const DeviceQueueCreateInfo& createInfo) const
	{
		return RefPtr<DeviceQueue>();
	}
	
	RefPtr<GraphicsContext> D3D12RHIProxy::CreateGraphicsContext(const GraphicsContextCreateInfo& createInfo) const
	{
		return RefPtr<D3D12GraphicsContext>::Create(createInfo);
	}
	
	RefPtr<GraphicsDevice> D3D12RHIProxy::CreateGraphicsDevice(const GraphicsDeviceCreateInfo& createInfo) const
	{
		return RefPtr<D3D12GraphicsDevice>::Create(createInfo);
	}
	
	RefPtr<PhysicalGraphicsDevice> D3D12RHIProxy::CreatePhysicalGraphicsDevice(const PhysicalDeviceCreateInfo& createInfo) const
	{
		return RefPtr<D3D12PhysicalGraphicsDevice>::Create(createInfo);
	}
	
	RefPtr<Swapchain> D3D12RHIProxy::CreateSwapchain(GLFWwindow* window) const
	{
		return RefPtr<D3D12Swapchain>::Create(window);
	}
	
	RefPtr<Image2D> D3D12RHIProxy::CreateImage2D(const ImageSpecification& specification, const void* data) const
	{
		return RefPtr<D3D12Image2D>::Create(specification, data);
	}
	
	RefPtr<Image2D> D3D12RHIProxy::CreateImage2D(const ImageSpecification& specification, RefPtr<Allocator> customAllocator, const void* data) const
	{
		return RefPtr<D3D12Image2D>::Create(specification, customAllocator, data);
	}
	
	RefPtr<Image2D> D3D12RHIProxy::CreateImage2D(const SwapchainImageSpecification& specification) const
	{
		return RefPtr<D3D12Image2D>::Create(specification);
	}
	
	RefPtr<ImageView> D3D12RHIProxy::CreateImageView(const ImageViewSpecification& specification) const
	{
		return RefPtr<D3D12ImageView>::Create(specification);
	}
	
	RefPtr<SamplerState> D3D12RHIProxy::CreateSamplerState(const SamplerStateCreateInfo& createInfo) const
	{
		return RefPtr<SamplerState>();
	}
	
	Scope<DefaultAllocator> D3D12RHIProxy::CreateDefaultAllocator() const
	{
		return CreateScope<D3D12DefaultAllocator>();
	}
	
	RefPtr<TransientAllocator> D3D12RHIProxy::CreateTransientAllocator() const
	{
		return RefPtr<TransientAllocator>();
	}
	
	RefPtr<TransientHeap> D3D12RHIProxy::CreateTransientHeap(const TransientHeapCreateInfo& createInfo) const
	{
		return RefPtr<TransientHeap>();
	}
	
	RefPtr<RenderPipeline> D3D12RHIProxy::CreateRenderPipeline(const RenderPipelineCreateInfo& createInfo) const
	{
		return RefPtr<D3D12RenderPipeline>::Create(createInfo);
	}
	
	RefPtr<ComputePipeline> D3D12RHIProxy::CreateComputePipeline(RefPtr<Shader> shader, bool useGlobalResources) const
	{
		return RefPtr<D3D12ComputePipeline>::Create(shader, useGlobalResources);
	}
	
	RefPtr<Shader> D3D12RHIProxy::CreateShader(const ShaderSpecification& specification) const
	{
		return RefPtr<D3D12Shader>::Create(specification);
	}
	
	RefPtr<ShaderCompiler> D3D12RHIProxy::CreateShaderCompiler(const ShaderCompilerCreateInfo& createInfo) const
	{
		return RefPtr<D3D12ShaderCompiler>::Create(createInfo);
	}
	
	RefPtr<Event> D3D12RHIProxy::CreateEvent(const EventCreateInfo& createInfo) const
	{
		return RefPtr<Event>();
	}
	
	RefPtr<Fence> D3D12RHIProxy::CreateFence(const FenceCreateInfo& createInfo) const
	{
		return RefPtr<Fence>();
	}
	
	RefPtr<Semaphore> D3D12RHIProxy::CreateSemaphore(const SemaphoreCreateInfo& createInfo) const
	{
		return RefPtr<D3D12Semaphore>::Create(createInfo);
	}
	
	RefPtr<ImGuiImplementation> D3D12RHIProxy::CreateImGuiImplementation(const ImGuiCreateInfo& createInfo) const
	{
		return RefPtr<ImGuiImplementation>();
	}

	void D3D12RHIProxy::SetRHICallbackInfo(const RHICallbackInfo& callbackInfo)
	{
		m_callbackInfo = callbackInfo;
	}

	void D3D12RHIProxy::DestroyResource(std::function<void()>&& function)
	{
		if (m_callbackInfo.resourceManagementInfo.resourceDeletionCallback)
		{
			m_callbackInfo.resourceManagementInfo.resourceDeletionCallback(std::move(function));
		}
		else
		{
			function();
		}
	}

	void D3D12RHIProxy::RequestApplicationClose()
	{
		if (m_callbackInfo.requestCloseEventCallback)
		{
			m_callbackInfo.requestCloseEventCallback();
		}
	}

	RefPtr<RHIProxy> CreateD3D12RHIProxy()
	{
		return RefPtr<D3D12RHIProxy>::Create();
	}
}
