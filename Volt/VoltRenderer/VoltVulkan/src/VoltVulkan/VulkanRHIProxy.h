#pragma once

#include "VoltVulkan/Core.h"

#include <VoltRHI/RHIProxy.h>

namespace Volt::RHI
{
	class VulkanRHIProxy : public RHIProxy
	{
	public:
		VulkanRHIProxy();
		~VulkanRHIProxy() override = default;

		Ref<BufferView> CreateBufferView(const BufferViewSpecification& specification) const override;

		Ref<CommandBuffer> CreateCommandBuffer(const uint32_t count, QueueType queueType) const override;
		Ref<CommandBuffer> CreateCommandBuffer(Weak<Swapchain> swapchain) const override;

		Ref<IndexBuffer> CreateIndexBuffer(std::span<const uint32_t> indices) const override;
		Ref<VertexBuffer> CreateVertexBuffer(const uint32_t size, const void* data) const override;

		Ref<StorageBuffer> CreateStorageBuffer(const uint32_t count, const size_t elementSize, std::string_view name, const BufferUsage bufferUsage, const MemoryUsage memoryUsage) const override;
		Ref<StorageBuffer> CreateStorageBuffer(const size_t size, std::string_view name, const BufferUsage bufferUsage, const MemoryUsage memoryUsage) const override;
		Ref<StorageBuffer> CreateStorageBuffer(const size_t size, Ref<Allocator> customAllocator, std::string_view name, const BufferUsage bufferUsage, const MemoryUsage memoryUsage) const override;
		Ref<UniformBuffer> CreateUniformBuffer(const uint32_t size, const void* data) const override;

		Ref<DescriptorTable> CreateDescriptorTable(const DescriptorTableCreateInfo& createInfo) const override;

		Ref<DeviceQueue> CreateDeviceQueue(const DeviceQueueCreateInfo& createInfo) const override;
		Ref<GraphicsContext> CreateGraphicsContext(const GraphicsContextCreateInfo& createInfo) const override;
		Ref<GraphicsDevice> CreateGraphicsDevice(const GraphicsDeviceCreateInfo& createInfo) const override;
		Ref<PhysicalGraphicsDevice> CreatePhysicalGraphicsDevice(const PhysicalDeviceCreateInfo& createInfo) const override;
		Ref<Swapchain> CreateSwapchain(GLFWwindow* window) const override;

		Ref<Image2D> CreateImage2D(const ImageSpecification& specification, const void* data) const override;
		Ref<Image2D> CreateImage2D(const ImageSpecification& specification, Ref<Allocator> customAllocator, const void* data) const override;
		Ref<Image2D> CreateImage2D(const SwapchainImageSpecification& specification) const override;

		Ref<ImageView> CreateImageView(const ImageViewSpecification& specification) const override;
		Ref<SamplerState> CreateSamplerState(const SamplerStateCreateInfo& createInfo) const override;

		Scope<DefaultAllocator> CreateDefaultAllocator() const override;
		Ref<TransientAllocator> CreateTransientAllocator() const override;
		Ref<TransientHeap> CreateTransientHeap(const TransientHeapCreateInfo& createInfo) const override;

		Ref<RenderPipeline> CreateRenderPipeline(const RenderPipelineCreateInfo& createInfo) const override;
		Ref<ComputePipeline> CreateComputePipeline(Ref<Shader> shader, bool useGlobalResources) const override;

		Ref<Shader> CreateShader(const ShaderSpecification& specification) const override;
		Ref<ShaderCompiler> CreateShaderCompiler(const ShaderCompilerCreateInfo& createInfo) const override;

		Ref<Event> CreateEvent(const EventCreateInfo& createInfo) const override;
		Ref<Fence> CreateFence(const FenceCreateInfo& createInfo) const override;
		Ref<Semaphore> CreateSemaphore(const SemaphoreCreateInfo& createInfo) const override;
	
		Ref<ImGuiImplementation> CreateImGuiImplementation(const ImGuiCreateInfo& createInfo) const override;

		void SetRHICallbackInfo(const RHICallbackInfo& callbackInfo) override;
		void DestroyResource(std::function<void()>&& function) override;
		void RequestApplicationClose() override;

	private:
		RHICallbackInfo m_callbackInfo;
	};

	VTVK_API Ref<RHIProxy> CreateVulkanRHIProxy();
}
