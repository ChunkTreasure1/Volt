#pragma once

#include "VulkanRHIModule/Core.h"

#include <RHIModule/Core/RHICommon.h>
#include <RHIModule/Graphics/Swapchain.h>
#include <RHIModule/Buffers/CommandBuffer.h>

struct VkSwapchainKHR_T;
struct VkRenderPass_T;
struct VkSurfaceKHR_T;

struct VkFence_T;
struct VkSemaphore_T;
struct VkCommandPool_T;
struct VkCommandBuffer_T;

struct VkImage_T;
struct VkImageView_T;
struct VkFramebuffer_T;

struct GLFWwindow;

namespace Volt::RHI
{
	class VulkanSwapchain final : public Swapchain
	{
	public:
		struct SurfaceFormat
		{
			PixelFormat format;
			ColorSpace colorSpace;
		};
		
		inline constexpr static uint32_t MAX_FRAMES_IN_FLIGHT = 3;

		VulkanSwapchain(GLFWwindow* glfwWindow);
		~VulkanSwapchain() override;

		void BeginFrame() override;
		void Present() override;
		void Resize(const uint32_t width, const uint32_t height, bool enableVSync) override;

		const uint32_t GetCurrentFrame() const override;
		const uint32_t GetWidth() const override;
		const uint32_t GetHeight() const override;
		const uint32_t GetFramesInFlight() const override;
		const PixelFormat GetFormat() const override;
		RefPtr<Image2D> GetCurrentImage() const override;
		RefPtr<CommandBuffer> GetCommandBuffer() const override;

		inline VkImage_T* GetImageAtIndex(const uint32_t index) const { return m_perImageData.at(index).image; }

	protected:
		void* GetHandleImpl() const override;
	
	private:
		void Invalidate(const uint32_t width, const uint32_t height, bool enableVSync);
		void Release();

		void QuerySwapchainCapabilities();

		void CreateSwapchain(const uint32_t width, const uint32_t height, bool enableVSync);
		void CreateSyncObjects();

		void GetNextFrameIndex();

		uint32_t m_currentImage = 0;
		uint32_t m_currentFrame = 0;

		uint32_t m_width = 1280;
		uint32_t m_height = 720;

		bool m_vSyncEnabled = false;
		bool m_swapchainNeedsRebuild = false;

		uint32_t m_totalImageCount = 0;

		struct PerFrameInFlightData
		{
			VkSemaphore_T* renderSemaphore = nullptr;
			VkSemaphore_T* presentSemaphore = nullptr;
		};

		struct PerImageData
		{
			VkImage_T* image = nullptr;
			RefPtr<Image2D> imageReference;
		};

		struct SwapchainCapabilities
		{
			uint32_t minImageCount = 0;
			uint32_t maxImageCount = 0;

			Extent2D minImageExtent{};
			Extent2D maxImageExtent{};

			Vector<PresentMode> presentModes{};
			Vector<SurfaceFormat> surfaceFormats{};
		};

		SwapchainCapabilities m_capabilities{};

		Vector<RefPtr<CommandBuffer>> m_commandBuffers;
		Vector<PerFrameInFlightData> m_perFrameInFlightData{};
		Vector<PerImageData> m_perImageData{};

		VkSwapchainKHR_T* m_swapchain = nullptr;
		VkSurfaceKHR_T* m_surface = nullptr;

		PixelFormat m_swapchainFormat = PixelFormat::UNDEFINED;
	};
}
