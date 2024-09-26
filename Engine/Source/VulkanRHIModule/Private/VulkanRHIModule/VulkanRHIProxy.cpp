#include "vkpch.h"
#include "VulkanRHIProxy.h"

#include "VulkanRHIModule/Buffers/VulkanBufferView.h"
#include "VulkanRHIModule/Buffers/VulkanCommandBuffer.h"
#include "VulkanRHIModule/Buffers/VulkanIndexBuffer.h"
#include "VulkanRHIModule/Buffers/VulkanVertexBuffer.h"
#include "VulkanRHIModule/Buffers/VulkanUniformBuffer.h"
#include "VulkanRHIModule/Buffers/VulkanStorageBuffer.h"

#include "VulkanRHIModule/Descriptors/VulkanDescriptorTable.h"
#include "VulkanRHIModule/Descriptors/VulkanBindlessDescriptorTable.h"

#include "VulkanRHIModule/Graphics/VulkanDeviceQueue.h"
#include "VulkanRHIModule/Graphics/VulkanGraphicsContext.h"
#include "VulkanRHIModule/Graphics/VulkanGraphicsDevice.h"
#include "VulkanRHIModule/Graphics/VulkanPhysicalGraphicsDevice.h"
#include "VulkanRHIModule/Graphics/VulkanSwapchain.h"

#include "VulkanRHIModule/Images/VulkanImage.h"
#include "VulkanRHIModule/Images/VulkanImageView.h"
#include "VulkanRHIModule/Images/VulkanSamplerState.h"

#include "VulkanRHIModule/Memory/VulkanDefaultAllocator.h"
#include "VulkanRHIModule/Memory/VulkanTransientAllocator.h"
#include "VulkanRHIModule/Memory/VulkanTransientHeap.h"

#include "VulkanRHIModule/Pipelines/VulkanRenderPipeline.h"
#include "VulkanRHIModule/Pipelines/VulkanComputePipeline.h"

#include "VulkanRHIModule/Shader/VulkanShader.h"
#include "VulkanRHIModule/Shader/VulkanShaderCompiler.h"

#include "VulkanRHIModule/Synchronization/VulkanEvent.h"
#include "VulkanRHIModule/Synchronization/VulkanFence.h"
#include "VulkanRHIModule/Synchronization/VulkanSemaphore.h"

#include "VulkanRHIModule/ImGui/VulkanImGuiImplementation.h"

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

	RefPtr<CommandBuffer> VulkanRHIProxy::CreateCommandBuffer(QueueType queueType) const
	{
		return RefPtr<VulkanCommandBuffer>::Create(queueType);
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

	RefPtr<Image> VulkanRHIProxy::CreateImage(const ImageSpecification& specification, const void* data, RefPtr<Allocator> allocator) const
	{
		return RefPtr<VulkanImage>::Create(specification, data, allocator);
	}

	RefPtr<Image> VulkanRHIProxy::CreateImage(const SwapchainImageSpecification& specification) const
	{
		return RefPtr<VulkanImage>::Create(specification);
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
