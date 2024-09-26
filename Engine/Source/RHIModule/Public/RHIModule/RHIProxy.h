#pragma once

#include "RHIModule/Core/RHICommon.h"

#include <CoreUtilities/Pointers/RefPtr.h>
#include <CoreUtilities/Pointers/WeakPtr.h>

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
	class BindlessDescriptorTable;
	class DeviceQueue;
	class GraphicsContext;
	class GraphicsDevice;
	class PhysicalGraphicsDevice;
	class Swapchain;

	class Allocator;
	class DefaultAllocator;
	class TransientAllocator;
	class TransientHeap;

	class Image;
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
	class ResourceStateTracker;

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

	class VTRHI_API RHIProxy : public RefCounted
	{
	public:
		virtual ~RHIProxy();

		virtual RefPtr<BufferView> CreateBufferView(const BufferViewSpecification& specification) const = 0;

		virtual RefPtr<CommandBuffer> CreateCommandBuffer(QueueType queueType) const = 0;

		virtual RefPtr<IndexBuffer> CreateIndexBuffer(std::span<const uint32_t> indices) const = 0;
		virtual RefPtr<VertexBuffer> CreateVertexBuffer(const void* data, const uint32_t size, const uint32_t stride) const = 0;

		virtual RefPtr<StorageBuffer> CreateStorageBuffer(uint32_t count, uint64_t elementSize, std::string_view name, BufferUsage bufferUsage, MemoryUsage memoryUsage, RefPtr<Allocator> allocator) const = 0;
		virtual RefPtr<UniformBuffer> CreateUniformBuffer(const uint32_t size, const void* data, const uint32_t count, std::string_view name) const = 0;

		virtual RefPtr<DescriptorTable> CreateDescriptorTable(const DescriptorTableCreateInfo& createInfo) const = 0;
		virtual RefPtr<BindlessDescriptorTable> CreateBindlessDescriptorTable(const uint64_t framesInFlight) const = 0;

		virtual RefPtr<DeviceQueue> CreateDeviceQueue(const DeviceQueueCreateInfo& createInfo) const = 0;
		virtual RefPtr<GraphicsContext> CreateGraphicsContext(const GraphicsContextCreateInfo& createInfo) const = 0;
		virtual RefPtr<GraphicsDevice> CreateGraphicsDevice(const GraphicsDeviceCreateInfo& createInfo) const = 0;
		virtual RefPtr<PhysicalGraphicsDevice> CreatePhysicalGraphicsDevice(const PhysicalDeviceCreateInfo& createInfo) const = 0;
		virtual RefPtr<Swapchain> CreateSwapchain(GLFWwindow* window) const = 0;

		virtual RefPtr<Image> CreateImage(const ImageSpecification& specification, const void* data, RefPtr<Allocator> allocator) const = 0;
		virtual RefPtr<Image> CreateImage(const SwapchainImageSpecification& specification) const = 0;

		virtual RefPtr<ImageView> CreateImageView(const ImageViewSpecification& specification) const = 0;
		virtual RefPtr<SamplerState> CreateSamplerState(const SamplerStateCreateInfo& createInfo) const = 0;

		virtual RefPtr<DefaultAllocator> CreateDefaultAllocator() const = 0; 
		virtual RefPtr<TransientAllocator> CreateTransientAllocator() const = 0;
		virtual RefPtr<TransientHeap> CreateTransientHeap(const TransientHeapCreateInfo& createInfo) const = 0;

		virtual RefPtr<RenderPipeline> CreateRenderPipeline(const RenderPipelineCreateInfo& createInfo) const = 0;
		virtual RefPtr<ComputePipeline> CreateComputePipeline(RefPtr<Shader> shader, bool useGlobalResources) const = 0;

		virtual RefPtr<Shader> CreateShader(const ShaderSpecification& specification) const = 0;
		virtual RefPtr<ShaderCompiler> CreateShaderCompiler(const ShaderCompilerCreateInfo& createInfo) const = 0;

		virtual RefPtr<Event> CreateEvent(const EventCreateInfo& createInfo) const = 0;
		virtual RefPtr<Fence> CreateFence(const FenceCreateInfo& createInfo) const = 0;
		virtual RefPtr<Semaphore> CreateSemaphore(const SemaphoreCreateInfo& createInfo) const = 0;

		virtual RefPtr<ImGuiImplementation> CreateImGuiImplementation(const ImGuiCreateInfo& createInfo) const = 0;

		virtual void SetRHICallbackInfo(const RHICallbackInfo& callbackInfo) = 0;

		virtual void DestroyResource(std::function<void()>&& function) = 0;
		virtual void RequestApplicationClose() = 0;

		static RHIProxy& GetInstance() { return *s_instance; }

	protected:
		inline static RHIProxy* s_instance = nullptr;

		RHIProxy();
	};
}
