#pragma once

#include "VoltRHI/Core/RHICommon.h"
#include "VoltRHI/RHILog.h"

#include <CoreUtilities/Core.h>

#include <span>

struct GLFWwindow;

namespace Volt::RHI
{
	class StorageBuffer;
	class UniformBuffer;

	class VertexBuffer;
	class IndexBuffer;
	class BufferView;

	class CommandBuffer;

	class DescriptorTable;
	class DeviceQueue;
	class GraphicsContext;
	class GraphicsDevice;
	class PhysicalGraphicsDevice;
	class Swapchain;

	class Allocator;
	class DefaultAllocator;
	class TransientAllocator;
	class TransientHeap;

	class Image2D;
	class ImageView;
	class SamplerState;

	class RenderPipeline;
	class ComputePipeline;

	class Shader;
	class ShaderCompiler;

	class Event;
	class Fence;
	class Semaphore;

	class ImGuiImplementation;

	struct BufferViewSpecification;
	struct DescriptorTableCreateInfo;
	struct DeviceQueueCreateInfo;
	struct GraphicsContextCreateInfo;
	struct GraphicsDeviceCreateInfo;
	struct PhysicalDeviceCreateInfo;
	struct ImageSpecification;
	struct SwapchainImageSpecification;
	struct ImageViewSpecification;
	struct SamplerStateCreateInfo;
	struct TransientHeapCreateInfo;
	struct RenderPipelineCreateInfo;
	struct ShaderSpecification;
	struct ShaderCompilerCreateInfo;
	struct EventCreateInfo;
	struct FenceCreateInfo;
	struct SemaphoreCreateInfo;
	struct ImGuiCreateInfo;

	struct RHICallbackInfo
	{
		ResourceManagementInfo resourceManagementInfo;
		std::function<void()> requestCloseEventCallback;
	};

	class VTRHI_API RHIProxy
	{
	public:
		virtual ~RHIProxy();

		void SetLogInfo(const LogInfo& logInfo);

		virtual Ref<BufferView> CreateBufferView(const BufferViewSpecification& specification) const = 0;

		virtual Ref<CommandBuffer> CreateCommandBuffer(const uint32_t count, QueueType queueType) const = 0;
		virtual Ref<CommandBuffer> CreateCommandBuffer(Weak<Swapchain> swapchain) const = 0;

		virtual Ref<IndexBuffer> CreateIndexBuffer(std::span<const uint32_t> indices) const = 0;
		virtual Ref<VertexBuffer> CreateVertexBuffer(const uint32_t size, const void* data) const = 0;

		virtual Ref<StorageBuffer> CreateStorageBuffer(const uint32_t count, const size_t elementSize, std::string_view name, const BufferUsage bufferUsage, const MemoryUsage memoryUsage) const = 0;
		virtual Ref<StorageBuffer> CreateStorageBuffer(const size_t size, std::string_view name, const BufferUsage bufferUsage, const MemoryUsage memoryUsage) const = 0;
		virtual Ref<StorageBuffer> CreateStorageBuffer(const size_t size, Ref<Allocator> customAllocator, std::string_view name, const BufferUsage bufferUsage, const MemoryUsage memoryUsage) const = 0;
		virtual Ref<UniformBuffer> CreateUniformBuffer(const uint32_t size, const void* data) const = 0;

		virtual Ref<DescriptorTable> CreateDescriptorTable(const DescriptorTableCreateInfo& createInfo) const = 0;

		virtual Ref<DeviceQueue> CreateDeviceQueue(const DeviceQueueCreateInfo& createInfo) const = 0;
		virtual Ref<GraphicsContext> CreateGraphicsContext(const GraphicsContextCreateInfo& createInfo) const = 0;
		virtual Ref<GraphicsDevice> CreateGraphicsDevice(const GraphicsDeviceCreateInfo& createInfo) const = 0;
		virtual Ref<PhysicalGraphicsDevice> CreatePhysicalGraphicsDevice(const PhysicalDeviceCreateInfo& createInfo) const = 0;
		virtual Ref<Swapchain> CreateSwapchain(GLFWwindow* window) const = 0;

		virtual Ref<Image2D> CreateImage2D(const ImageSpecification& specification, const void* data) const = 0;
		virtual Ref<Image2D> CreateImage2D(const ImageSpecification& specification, Ref<Allocator> customAllocator, const void* data) const = 0;
		virtual Ref<Image2D> CreateImage2D(const SwapchainImageSpecification& specification) const = 0;

		virtual Ref<ImageView> CreateImageView(const ImageViewSpecification& specification) const = 0;
		virtual Ref<SamplerState> CreateSamplerState(const SamplerStateCreateInfo& createInfo) const = 0;

		virtual Scope<DefaultAllocator> CreateDefaultAllocator() const = 0;
		virtual Ref<TransientAllocator> CreateTransientAllocator() const = 0;
		virtual Ref<TransientHeap> CreateTransientHeap(const TransientHeapCreateInfo& createInfo) const = 0;

		virtual Ref<RenderPipeline> CreateRenderPipeline(const RenderPipelineCreateInfo& createInfo) const = 0;
		virtual Ref<ComputePipeline> CreateComputePipeline(Ref<Shader> shader, bool useGlobalResources) const = 0;

		virtual Ref<Shader> CreateShader(const ShaderSpecification& specification) const = 0;
		virtual Ref<ShaderCompiler> CreateShaderCompiler(const ShaderCompilerCreateInfo& createInfo) const = 0;

		virtual Ref<Event> CreateEvent(const EventCreateInfo& createInfo) const = 0;
		virtual Ref<Fence> CreateFence(const FenceCreateInfo& createInfo) const = 0;
		virtual Ref<Semaphore> CreateSemaphore(const SemaphoreCreateInfo& createInfo) const = 0;

		virtual Ref<ImGuiImplementation> CreateImGuiImplementation(const ImGuiCreateInfo& createInfo) const = 0;
		
		virtual void SetRHICallbackInfo(const RHICallbackInfo& callbackInfo) = 0;

		virtual void DestroyResource(std::function<void()>&& function) = 0;
		virtual void RequestApplicationClose() = 0;

		static RHIProxy& GetInstance() { return *s_instance; }

	protected:
		inline static RHIProxy* s_instance = nullptr;

		Scope<RHILog> m_logger;

		RHIProxy();
	};
}
