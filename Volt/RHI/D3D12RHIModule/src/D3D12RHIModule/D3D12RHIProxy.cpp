#include "dxpch.h"
#include "D3D12RHIProxy.h"

#include "D3D12RHIModule/Graphics/D3D12Swapchain.h"
#include "D3D12RHIModule/Graphics/D3D12DeviceQueue.h"
#include "D3D12RHIModule/Graphics/D3D12GraphicsContext.h"
#include "D3D12RHIModule/Graphics/D3D12PhysicalGraphicsDevice.h"
#include "D3D12RHIModule/Graphics/D3D12GraphicsDevice.h"

#include "D3D12RHIModule/Descriptors/D3D12DescriptorTable.h"
#include "D3D12RHIModule/Descriptors/D3D12BindlessDescriptorTable.h"

#include "D3D12RHIModule/Buffers/D3D12UniformBuffer.h"
#include "D3D12RHIModule/Buffers/D3D12StorageBuffer.h"
#include "D3D12RHIModule/Buffers/D3D12VertexBuffer.h"
#include "D3D12RHIModule/Buffers/D3D12IndexBuffer.h"
#include "D3D12RHIModule/Buffers/D3D12BufferView.h"
#include "D3D12RHIModule/Buffers/D3D12CommandBuffer.h"

#include "D3D12RHIModule/ImGui/D3D12ImGuiImplementation.h"

#include "D3D12RHIModule/Shader/D3D12ShaderCompiler.h"
#include "D3D12RHIModule/Shader/D3D12Shader.h"

#include "D3D12RHIModule/Pipelines/D3D12RenderPipeline.h"
#include "D3D12RHIModule/Pipelines/D3D12ComputePipeline.h"

#include "D3D12RHIModule/Memory/D3D12TransientHeap.h"
#include "D3D12RHIModule/Memory/D3D12TransientAllocator.h"
#include "D3D12RHIModule/Memory/D3D12DefaultAllocator.h"

#include "D3D12RHIModule/Images/D3D12SamplerState.h"
#include "D3D12RHIModule/Images/D3D12ImageView.h"
#include "D3D12RHIModule/Images/D3D12Image2D.h"
#include <RHIModule/Images/Image3D.h>

#include "D3D12RHIModule/Synchronization/D3D12Semaphore.h"
#include <RHIModule/Synchronization/Fence.h>
#include <RHIModule/Synchronization/Event.h>

namespace Volt::RHI
{
	D3D12RHIProxy::D3D12RHIProxy()
	{
		s_instance = this;
	}
	
	RefPtr<BufferView> D3D12RHIProxy::CreateBufferView(const BufferViewSpecification& specification) const
	{
		return RefPtr<D3D12BufferView>::Create(specification);
	}
	
	RefPtr<CommandBuffer> D3D12RHIProxy::CreateCommandBuffer(const uint32_t count, QueueType queueType) const
	{
		return RefPtr<D3D12CommandBuffer>::Create(count, queueType);
	}
	
	RefPtr<IndexBuffer> D3D12RHIProxy::CreateIndexBuffer(std::span<const uint32_t> indices) const
	{
		return RefPtr<D3D12IndexBuffer>::Create(indices);
	}
	
	RefPtr<VertexBuffer> D3D12RHIProxy::CreateVertexBuffer(const void* data, const uint32_t size, const uint32_t stride) const
	{
		return RefPtr<D3D12VertexBuffer>::Create(data, size, stride);
	}
	
	RefPtr<StorageBuffer> D3D12RHIProxy::CreateStorageBuffer(uint32_t count, uint64_t elementSize, std::string_view name, BufferUsage bufferUsage, MemoryUsage memoryUsage, RefPtr<Allocator> allocator) const
	{
		return RefPtr<D3D12StorageBuffer>::Create(count, elementSize, name, bufferUsage, memoryUsage, allocator);
	}

	RefPtr<UniformBuffer> D3D12RHIProxy::CreateUniformBuffer(const uint32_t size, const void* data, const uint32_t count, std::string_view name) const
	{
		return RefPtr<D3D12UniformBuffer>::Create(size, data, count, name);
	}
	
	RefPtr<DescriptorTable> D3D12RHIProxy::CreateDescriptorTable(const DescriptorTableCreateInfo& createInfo) const
	{
		return RefPtr<D3D12DescriptorTable>::Create(createInfo);
	}

	RefPtr<BindlessDescriptorTable> D3D12RHIProxy::CreateBindlessDescriptorTable(const uint64_t framesInFlight) const
	{
		return RefPtr<D3D12BindlessDescriptorTable>::Create(framesInFlight);
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
	
	RefPtr<Image2D> D3D12RHIProxy::CreateImage2D(const ImageSpecification& specification, const void* data, RefPtr<Allocator> allocator) const
	{
		return RefPtr<D3D12Image2D>::Create(specification, data, allocator);
	}
	
	RefPtr<Image2D> D3D12RHIProxy::CreateImage2D(const SwapchainImageSpecification& specification) const
	{
		return RefPtr<D3D12Image2D>::Create(specification);
	}

	RefPtr<Image3D> D3D12RHIProxy::CreateImage3D(const ImageSpecification& specification, const void* data, RefPtr<Allocator> allocator) const
	{
		return RefPtr<Image3D>();
	}
	
	RefPtr<ImageView> D3D12RHIProxy::CreateImageView(const ImageViewSpecification& specification) const
	{
		return RefPtr<D3D12ImageView>::Create(specification);
	}
	
	RefPtr<SamplerState> D3D12RHIProxy::CreateSamplerState(const SamplerStateCreateInfo& createInfo) const
	{
		return RefPtr<D3D12SamplerState>::Create(createInfo);
	}
	
	RefPtr<DefaultAllocator> D3D12RHIProxy::CreateDefaultAllocator() const
	{
		return RefPtr<D3D12DefaultAllocator>::Create();
	}
	
	RefPtr<TransientAllocator> D3D12RHIProxy::CreateTransientAllocator() const
	{
		return RefPtr<D3D12TransientAllocator>::Create();
	}
	
	RefPtr<TransientHeap> D3D12RHIProxy::CreateTransientHeap(const TransientHeapCreateInfo& createInfo) const
	{
		return RefPtr<D3D12TransientHeap>::Create(createInfo);
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
		return RefPtr<D3D12ImGuiImplementation>::Create(createInfo);
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
