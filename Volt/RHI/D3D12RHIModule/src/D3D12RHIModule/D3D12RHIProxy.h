#pragma once

#include "D3D12RHIModule/Core.h"
#include <RHIModule/RHIProxy.h>

namespace Volt::RHI
{
	class D3D12RHIProxy : public RHIProxy
	{
	public:
		D3D12RHIProxy();
		~D3D12RHIProxy() override = default;

		RefPtr<BufferView> CreateBufferView(const BufferViewSpecification& specification) const override;

		RefPtr<CommandBuffer> CreateCommandBuffer(const uint32_t count, QueueType queueType) const override;

		RefPtr<IndexBuffer> CreateIndexBuffer(std::span<const uint32_t> indices) const override;
		RefPtr<VertexBuffer> CreateVertexBuffer(const void* data, const uint32_t size, const uint32_t stride) const override;

		RefPtr<StorageBuffer> CreateStorageBuffer(uint32_t count, uint64_t elementSize, std::string_view name, BufferUsage bufferUsage, MemoryUsage memoryUsage, RefPtr<Allocator> allocator) const override;
		RefPtr<UniformBuffer> CreateUniformBuffer(const uint32_t size, const void* data, const uint32_t count, std::string_view name) const override;

		RefPtr<DescriptorTable> CreateDescriptorTable(const DescriptorTableCreateInfo& createInfo) const override;
		RefPtr<BindlessDescriptorTable> CreateBindlessDescriptorTable(const uint64_t framesInFlight) const override;

		RefPtr<DeviceQueue> CreateDeviceQueue(const DeviceQueueCreateInfo& createInfo) const override;
		RefPtr<GraphicsContext> CreateGraphicsContext(const GraphicsContextCreateInfo& createInfo) const override;
		RefPtr<GraphicsDevice> CreateGraphicsDevice(const GraphicsDeviceCreateInfo& createInfo) const override;
		RefPtr<PhysicalGraphicsDevice> CreatePhysicalGraphicsDevice(const PhysicalDeviceCreateInfo& createInfo) const override;
		RefPtr<Swapchain> CreateSwapchain(GLFWwindow* window) const override;

		RefPtr<Image2D> CreateImage2D(const ImageSpecification& specification, const void* data, RefPtr<Allocator> allocator) const override;
		RefPtr<Image2D> CreateImage2D(const SwapchainImageSpecification& specification) const override;
		RefPtr<Image3D> CreateImage3D(const ImageSpecification& specification, const void* data, RefPtr<Allocator> allocator) const override;

		RefPtr<ImageView> CreateImageView(const ImageViewSpecification& specification) const override;
		RefPtr<SamplerState> CreateSamplerState(const SamplerStateCreateInfo& createInfo) const override;

		RefPtr<DefaultAllocator> CreateDefaultAllocator() const override;
		RefPtr<TransientAllocator> CreateTransientAllocator() const override;
		RefPtr<TransientHeap> CreateTransientHeap(const TransientHeapCreateInfo& createInfo) const override;

		RefPtr<RenderPipeline> CreateRenderPipeline(const RenderPipelineCreateInfo& createInfo) const override;
		RefPtr<ComputePipeline> CreateComputePipeline(RefPtr<Shader> shader, bool useGlobalResources) const override;

		RefPtr<Shader> CreateShader(const ShaderSpecification& specification) const override;
		RefPtr<ShaderCompiler> CreateShaderCompiler(const ShaderCompilerCreateInfo& createInfo) const override;

		RefPtr<Event> CreateEvent(const EventCreateInfo& createInfo) const override;
		RefPtr<Fence> CreateFence(const FenceCreateInfo& createInfo) const override;
		RefPtr<Semaphore> CreateSemaphore(const SemaphoreCreateInfo& createInfo) const override;

		RefPtr<ImGuiImplementation> CreateImGuiImplementation(const ImGuiCreateInfo& createInfo) const override;

		void SetRHICallbackInfo(const RHICallbackInfo& callbackInfo) override;
		void DestroyResource(std::function<void()>&& function) override;
		void RequestApplicationClose() override;

	private:
		RHICallbackInfo m_callbackInfo;
	};

	VTDX_API RefPtr<RHIProxy> CreateD3D12RHIProxy();
}
