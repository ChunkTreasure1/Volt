#include "vkpch.h"
#include "VulkanRHIProxy.h"

#include "VoltVulkan/Buffers/VulkanBufferView.h"
#include "VoltVulkan/Buffers/VulkanCommandBuffer.h"
#include "VoltVulkan/Buffers/VulkanIndexBuffer.h"
#include "VoltVulkan/Buffers/VulkanVertexBuffer.h"
#include "VoltVulkan/Buffers/VulkanUniformBuffer.h"
#include "VoltVulkan/Buffers/VulkanStorageBuffer.h"

#include "VoltVulkan/Descriptors/VulkanDescriptorTable.h"
#include "VoltVulkan/Descriptors/VulkanBindlessDescriptorTable.h"

#include "VoltVulkan/Graphics/VulkanDeviceQueue.h"
#include "VoltVulkan/Graphics/VulkanGraphicsContext.h"
#include "VoltVulkan/Graphics/VulkanGraphicsDevice.h"
#include "VoltVulkan/Graphics/VulkanPhysicalGraphicsDevice.h"
#include "VoltVulkan/Graphics/VulkanSwapchain.h"

#include "VoltVulkan/Images/VulkanImage2D.h"
#include "VoltVulkan/Images/VulkanImageView.h"
#include "VoltVulkan/Images/VulkanSamplerState.h"

#include "VoltVulkan/Memory/VulkanDefaultAllocator.h"
#include "VoltVulkan/Memory/VulkanTransientAllocator.h"
#include "VoltVulkan/Memory/VulkanTransientHeap.h"

#include "VoltVulkan/Pipelines/VulkanRenderPipeline.h"
#include "VoltVulkan/Pipelines/VulkanComputePipeline.h"

#include "VoltVulkan/Shader/VulkanShader.h"
#include "VoltVulkan/Shader/VulkanShaderCompiler.h"

#include "VoltVulkan/Synchronization/VulkanEvent.h"
#include "VoltVulkan/Synchronization/VulkanFence.h"
#include "VoltVulkan/Synchronization/VulkanSemaphore.h"

#include "VoltVulkan/ImGui/VulkanImGuiImplementation.h"

namespace Volt::RHI
{
	VulkanRHIProxy::VulkanRHIProxy()
	{
		s_instance = this;
	}

	RefPtr<BufferView> VulkanRHIProxy::CreateBufferView(const BufferViewSpecification& specification) const
	{
		return RefPtr<VulkanBufferView>::Create(specification);
	}

	RefPtr<CommandBuffer> VulkanRHIProxy::CreateCommandBuffer(const uint32_t count, QueueType queueType) const
	{
		return RefPtr<VulkanCommandBuffer>::Create(count, queueType);
	}

	RefPtr<IndexBuffer> VulkanRHIProxy::CreateIndexBuffer(std::span<const uint32_t> indices) const
	{
		return RefPtr<VulkanIndexBuffer>::Create(indices);
	}

	RefPtr<VertexBuffer> VulkanRHIProxy::CreateVertexBuffer(const void* data, const uint32_t size, const uint32_t stride) const
	{
		return RefPtr<VulkanVertexBuffer>::Create(data, size, stride);
	}

	RefPtr<StorageBuffer> VulkanRHIProxy::CreateStorageBuffer(uint32_t count, uint64_t elementSize, std::string_view name, BufferUsage bufferUsage, MemoryUsage memoryUsage, RefPtr<Allocator> allocator) const
	{
		return RefPtr<VulkanStorageBuffer>::Create(count, elementSize, name, bufferUsage, memoryUsage, allocator);
	}

	RefPtr<UniformBuffer> VulkanRHIProxy::CreateUniformBuffer(const uint32_t size, const void* data, const uint32_t count, std::string_view name) const
	{
		return RefPtr<VulkanUniformBuffer>::Create(size, data, count, name);
	}

	RefPtr<DescriptorTable> VulkanRHIProxy::CreateDescriptorTable(const DescriptorTableCreateInfo& createInfo) const
	{
		return RefPtr<VulkanDescriptorTable>::Create(createInfo);
	}

	RefPtr<BindlessDescriptorTable> VulkanRHIProxy::CreateBindlessDescriptorTable(const uint64_t framesInFlight) const
	{
		return RefPtr<VulkanBindlessDescriptorTable>::Create(framesInFlight);
	}

	RefPtr<DeviceQueue> VulkanRHIProxy::CreateDeviceQueue(const DeviceQueueCreateInfo& createInfo) const
	{
		return RefPtr<VulkanDeviceQueue>::Create(createInfo);
	}

	RefPtr<GraphicsContext> VulkanRHIProxy::CreateGraphicsContext(const GraphicsContextCreateInfo& createInfo) const
	{
		return RefPtr<VulkanGraphicsContext>::Create(createInfo);
	}

	RefPtr<GraphicsDevice> VulkanRHIProxy::CreateGraphicsDevice(const GraphicsDeviceCreateInfo& createInfo) const
	{
		return RefPtr<VulkanGraphicsDevice>::Create(createInfo);
	}

	RefPtr<PhysicalGraphicsDevice> VulkanRHIProxy::CreatePhysicalGraphicsDevice(const PhysicalDeviceCreateInfo& createInfo) const
	{
		return RefPtr<VulkanPhysicalGraphicsDevice>::Create(createInfo);
	}

	RefPtr<Swapchain> VulkanRHIProxy::CreateSwapchain(GLFWwindow* window) const
	{
		return RefPtr<VulkanSwapchain>::Create(window);
	}

	RefPtr<Image2D> VulkanRHIProxy::CreateImage2D(const ImageSpecification& specification, const void* data, RefPtr<Allocator> allocator) const
	{
		return RefPtr<VulkanImage2D>::Create(specification, data, allocator);
	}

	RefPtr<Image2D> VulkanRHIProxy::CreateImage2D(const SwapchainImageSpecification& specification) const
	{
		return RefPtr<VulkanImage2D>::Create(specification);
	}

	RefPtr<ImageView> VulkanRHIProxy::CreateImageView(const ImageViewSpecification& specification) const
	{
		return RefPtr<VulkanImageView>::Create(specification);
	}

	RefPtr<SamplerState> VulkanRHIProxy::CreateSamplerState(const SamplerStateCreateInfo& createInfo) const
	{
		return RefPtr<VulkanSamplerState>::Create(createInfo);
	}

	RefPtr<DefaultAllocator> VulkanRHIProxy::CreateDefaultAllocator() const
	{
		return RefPtr<VulkanDefaultAllocator>::Create();
	}

	RefPtr<TransientAllocator> VulkanRHIProxy::CreateTransientAllocator() const
	{
		return RefPtr<VulkanTransientAllocator>::Create();
	}

	RefPtr<TransientHeap> VulkanRHIProxy::CreateTransientHeap(const TransientHeapCreateInfo& createInfo) const
	{
		return RefPtr<VulkanTransientHeap>::Create(createInfo);
	}

	RefPtr<RenderPipeline> VulkanRHIProxy::CreateRenderPipeline(const RenderPipelineCreateInfo& createInfo) const
	{
		return RefPtr<VulkanRenderPipeline>::Create(createInfo);
	}

	RefPtr<ComputePipeline> VulkanRHIProxy::CreateComputePipeline(RefPtr<Shader> shader, bool useGlobalResources) const
	{
		return RefPtr<VulkanComputePipeline>::Create(shader, useGlobalResources);
	}

	RefPtr<Shader> VulkanRHIProxy::CreateShader(const ShaderSpecification& specification) const
	{
		return RefPtr<VulkanShader>::Create(specification);
	}

	RefPtr<ShaderCompiler> VulkanRHIProxy::CreateShaderCompiler(const ShaderCompilerCreateInfo& createInfo) const
	{
		return RefPtr<VulkanShaderCompiler>::Create(createInfo);
	}

	RefPtr<Event> VulkanRHIProxy::CreateEvent(const EventCreateInfo& createInfo) const
	{
		return RefPtr<VulkanEvent>::Create(createInfo);
	}

	RefPtr<Fence> VulkanRHIProxy::CreateFence(const FenceCreateInfo& createInfo) const
	{
		return RefPtr<VulkanFence>::Create(createInfo);
	}

	RefPtr<Semaphore> VulkanRHIProxy::CreateSemaphore(const SemaphoreCreateInfo& createInfo) const
	{
		return RefPtr<VulkanSemaphore>::Create(createInfo);
	}

	RefPtr<ImGuiImplementation> VulkanRHIProxy::CreateImGuiImplementation(const ImGuiCreateInfo& createInfo) const
	{
		return RefPtr<VulkanImGuiImplementation>::Create(createInfo);
	}

	void VulkanRHIProxy::SetRHICallbackInfo(const RHICallbackInfo& callbackInfo)
	{
		m_callbackInfo = callbackInfo;
	}

	void VulkanRHIProxy::DestroyResource(std::function<void()>&& function)
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

	void VulkanRHIProxy::RequestApplicationClose()
	{
		if (m_callbackInfo.requestCloseEventCallback)
		{
			m_callbackInfo.requestCloseEventCallback();
		}
	}

	RefPtr<RHIProxy> CreateVulkanRHIProxy()
	{
		return RefPtr<VulkanRHIProxy>::Create();
	}
}
