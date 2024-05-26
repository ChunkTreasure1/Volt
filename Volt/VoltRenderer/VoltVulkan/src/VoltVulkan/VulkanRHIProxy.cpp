#include "vkpch.h"
#include "VulkanRHIProxy.h"

#include "VoltVulkan/Buffers/VulkanBufferView.h"
#include "VoltVulkan/Buffers/VulkanCommandBuffer.h"
#include "VoltVulkan/Buffers/VulkanIndexBuffer.h"
#include "VoltVulkan/Buffers/VulkanVertexBuffer.h"
#include "VoltVulkan/Buffers/VulkanUniformBuffer.h"
#include "VoltVulkan/Buffers/VulkanStorageBuffer.h"

#include "VoltVulkan/Descriptors/VulkanDescriptorTable.h"

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

	Ref<BufferView> VulkanRHIProxy::CreateBufferView(const BufferViewSpecification& specification) const
	{
		return CreateRef<VulkanBufferView>(specification);
	}

	Ref<CommandBuffer> VulkanRHIProxy::CreateCommandBuffer(const uint32_t count, QueueType queueType) const
	{
		return CreateRef<VulkanCommandBuffer>(count, queueType);
	}

	Ref<CommandBuffer> VulkanRHIProxy::CreateCommandBuffer(Weak<Swapchain> swapchain) const
	{
		return CreateRef<VulkanCommandBuffer>(swapchain);
	}

	Ref<IndexBuffer> VulkanRHIProxy::CreateIndexBuffer(std::span<const uint32_t> indices) const
	{
		return CreateRef<VulkanIndexBuffer>(indices);
	}

	Ref<VertexBuffer> VulkanRHIProxy::CreateVertexBuffer(const uint32_t size, const void* data) const
	{
		return CreateRef<VulkanVertexBuffer>(size, data);
	}

	Ref<StorageBuffer> VulkanRHIProxy::CreateStorageBuffer(const uint32_t count, const size_t elementSize, std::string_view name, const BufferUsage bufferUsage, const MemoryUsage memoryUsage) const
	{
		return CreateRef<VulkanStorageBuffer>(count, elementSize, name, bufferUsage, memoryUsage);
	}

	Ref<StorageBuffer> VulkanRHIProxy::CreateStorageBuffer(const size_t size, std::string_view name, const BufferUsage bufferUsage, const MemoryUsage memoryUsage) const
	{
		return CreateRef<VulkanStorageBuffer>(size, name, bufferUsage, memoryUsage);
	}

	Ref<StorageBuffer> VulkanRHIProxy::CreateStorageBuffer(const size_t size, Ref<Allocator> customAllocator, std::string_view name, const BufferUsage bufferUsage, const MemoryUsage memoryUsage) const
	{
		return CreateRef<VulkanStorageBuffer>(size, customAllocator, name, bufferUsage, memoryUsage);
	}

	Ref<UniformBuffer> VulkanRHIProxy::CreateUniformBuffer(const uint32_t size, const void* data) const
	{
		return CreateRef<VulkanUniformBuffer>(size, data);
	}

	Ref<DescriptorTable> VulkanRHIProxy::CreateDescriptorTable(const DescriptorTableCreateInfo& createInfo) const
	{
		return CreateRef<VulkanDescriptorTable>(createInfo);
	}

	Ref<DeviceQueue> VulkanRHIProxy::CreateDeviceQueue(const DeviceQueueCreateInfo& createInfo) const
	{
		return CreateRef<VulkanDeviceQueue>(createInfo);
	}

	Ref<GraphicsContext> VulkanRHIProxy::CreateGraphicsContext(const GraphicsContextCreateInfo& createInfo) const
	{
		return CreateRef<VulkanGraphicsContext>(createInfo);
	}

	Ref<GraphicsDevice> VulkanRHIProxy::CreateGraphicsDevice(const GraphicsDeviceCreateInfo& createInfo) const
	{
		return CreateRef<VulkanGraphicsDevice>(createInfo);
	}

	Ref<PhysicalGraphicsDevice> VulkanRHIProxy::CreatePhysicalGraphicsDevice(const PhysicalDeviceCreateInfo& createInfo) const
	{
		return CreateRef<VulkanPhysicalGraphicsDevice>(createInfo);
	}

	Ref<Swapchain> VulkanRHIProxy::CreateSwapchain(GLFWwindow* window) const
	{
		return CreateRef<VulkanSwapchain>(window);
	}

	Ref<Image2D> VulkanRHIProxy::CreateImage2D(const ImageSpecification& specification, const void* data) const
	{
		return CreateRef<VulkanImage2D>(specification, data);
	}

	Ref<Image2D> VulkanRHIProxy::CreateImage2D(const ImageSpecification& specification, Ref<Allocator> customAllocator, const void* data) const
	{
		return CreateRef<VulkanImage2D>(specification, customAllocator, data);
	}

	Ref<Image2D> VulkanRHIProxy::CreateImage2D(const SwapchainImageSpecification& specification) const
	{
		return CreateRef<VulkanImage2D>(specification);
	}

	Ref<ImageView> VulkanRHIProxy::CreateImageView(const ImageViewSpecification& specification) const
	{
		return CreateRef<VulkanImageView>(specification);
	}

	Ref<SamplerState> VulkanRHIProxy::CreateSamplerState(const SamplerStateCreateInfo& createInfo) const
	{
		return CreateRef<VulkanSamplerState>(createInfo);
	}

	Scope<DefaultAllocator> VulkanRHIProxy::CreateDefaultAllocator() const
	{
		return CreateScope<VulkanDefaultAllocator>();
	}

	Ref<TransientAllocator> VulkanRHIProxy::CreateTransientAllocator() const
	{
		return CreateRef<VulkanTransientAllocator>();
	}

	Ref<TransientHeap> VulkanRHIProxy::CreateTransientHeap(const TransientHeapCreateInfo& createInfo) const
	{
		return CreateRef<VulkanTransientHeap>(createInfo);
	}

	Ref<RenderPipeline> VulkanRHIProxy::CreateRenderPipeline(const RenderPipelineCreateInfo& createInfo) const
	{
		return CreateRef<VulkanRenderPipeline>(createInfo);
	}

	Ref<ComputePipeline> VulkanRHIProxy::CreateComputePipeline(Ref<Shader> shader, bool useGlobalResources) const
	{
		return CreateRef<VulkanComputePipeline>(shader, useGlobalResources);
	}

	Ref<Shader> VulkanRHIProxy::CreateShader(const ShaderSpecification& specification) const
	{
		return CreateRef<VulkanShader>(specification);
	}

	Ref<ShaderCompiler> VulkanRHIProxy::CreateShaderCompiler(const ShaderCompilerCreateInfo& createInfo) const
	{
		return CreateRef<VulkanShaderCompiler>(createInfo);
	}

	Ref<Event> VulkanRHIProxy::CreateEvent(const EventCreateInfo& createInfo) const
	{
		return CreateRef<VulkanEvent>(createInfo);
	}

	Ref<Fence> VulkanRHIProxy::CreateFence(const FenceCreateInfo& createInfo) const
	{
		return CreateRef<VulkanFence>(createInfo);
	}

	Ref<Semaphore> VulkanRHIProxy::CreateSemaphore(const SemaphoreCreateInfo& createInfo) const
	{
		return CreateRef<VulkanSemaphore>(createInfo);
	}

	Ref<ImGuiImplementation> VulkanRHIProxy::CreateImGuiImplementation(const ImGuiCreateInfo& createInfo) const
	{
		return CreateRef<VulkanImGuiImplementation>(createInfo);
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

	Ref<RHIProxy> CreateVulkanRHIProxy()
	{
		return CreateRef<VulkanRHIProxy>();
	}
}
